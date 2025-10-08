/* SPDX-License-Identifier: BSD-3-Clause */
/* SPDX-FileCopyrightText: OpenMPT Project Developers and Contributors */


#include "openmpt/all/BuildSettings.hpp"
#include "openmpt/all/PlatformFixes.hpp"

#include "SoundDevicePulseaudio.hpp"

#include "SoundDevice.hpp"
#include "SoundDeviceUtilities.hpp"

#include "mpt/base/numeric.hpp"
#include "mpt/base/pointer.hpp"
#include "mpt/base/saturate_round.hpp"
#include "mpt/format/message_macros.hpp"
#include "mpt/format/simple.hpp"
#include "mpt/parse/split.hpp"
#include "mpt/string/types.hpp"
#include "mpt/string_transcode/transcode.hpp"
#include "openmpt/base/Types.hpp"
#include "openmpt/logging/Logger.hpp"
#include "openmpt/soundbase/SampleFormat.hpp"

#include <vector>

#include <cstddef>
#include <cstring>


OPENMPT_NAMESPACE_BEGIN


namespace SoundDevice
{


#if defined(MPT_ENABLE_PULSEAUDIO_FULL)

#if defined(MPT_WITH_PULSEAUDIO)


mpt::ustring Pulseaudio::PulseErrorString(int error)
{
	if(error == 0)
	{
		return mpt::ustring();
	}
	const char *str = pa_strerror(error);
	if(!str)
	{
		return MPT_UFORMAT_MESSAGE("error={}")(error);
	}
	if(std::strlen(str) == 0)
	{
		return MPT_UFORMAT_MESSAGE("error={}")(error);
	}
	return MPT_UFORMAT_MESSAGE("{} (error={})")(mpt::transcode<mpt::ustring>(mpt::common_encoding::utf8, str), error);
}


static void PulseAudioSinkInfoListCallback(pa_context * /* c */, const pa_sink_info *i, int /* eol */, void *userdata)
{
	MPT_LOG(GetLogger(), LogDebug, "sounddev", MPT_USTRING("PulseAudioSinkInfoListCallback"));
	std::vector<SoundDevice::Info> *devices_ = mpt::void_ptr<std::vector<SoundDevice::Info>>(userdata);
	if(!devices_)
	{
		return;
	}
	std::vector<SoundDevice::Info> &devices = *devices_;
	if(!i)
	{
		return;
	}
	if(!i->name)
	{
		return;
	}
	if(!i->description)
	{
		return;
	}
	if(i->n_ports <= 0)
	{
		return;
	}
	for(uint32 port = 0; port < i->n_ports; ++port)
	{
		// we skip all sinks without ports or with all ports known to be currently unavailable
		if(!i->ports)
		{
			break;
		}
		if(!i->ports[port])
		{
			continue;
		}
		if(i->ports[port]->available == PA_PORT_AVAILABLE_NO)
		{
			continue;
		}
		SoundDevice::Info info;
		info.type = MPT_USTRING("PulseAudio");
		info.internalID = mpt::transcode<mpt::ustring>(mpt::common_encoding::utf8, i->name);
		info.name = mpt::transcode<mpt::ustring>(mpt::common_encoding::utf8, i->description);
		info.apiName = MPT_USTRING("PulseAudio");
		info.default_ = Info::Default::None;
		info.useNameAsIdentifier = false;
		// clang-format off
		info.flags = {
			sysInfo.SystemClass == mpt::osinfo::osclass::Linux ? Info::Usability::Usable : Info::Usability::Experimental,
			Info::Level::Primary,
			Info::Compatible::No,
			sysInfo.SystemClass == mpt::osinfo::osclass::Linux ? Info::Api::Native : Info::Api::Emulated,
			Info::Io::FullDuplex,
			Info::Mixing::Server,
			Info::Implementor::External
		};
		// clang-format on
		devices.push_back(info);
		break;
	}
}


std::vector<SoundDevice::Info> Pulseaudio::EnumerateDevices(ILogger &logger, SoundDevice::SysInfo sysInfo)
{
	auto GetLogger = [&]() -> ILogger &
	{
		return logger;
	};
	std::vector<SoundDevice::Info> devices;
	SoundDevice::Info info;
	info.type = MPT_USTRING("PulseAudio");
	info.internalID = MPT_USTRING("0");
	info.name = MPT_USTRING("Default Device");
	info.apiName = MPT_USTRING("PulseAudio");
	info.default_ = Info::Default::Managed;
	info.useNameAsIdentifier = false;
	// clang-format off
	info.flags = {
		sysInfo.SystemClass == mpt::osinfo::osclass::Linux ? Info::Usability::Usable : Info::Usability::Experimental,
		Info::Level::Primary,
		Info::Compatible::No,
		sysInfo.SystemClass == mpt::osinfo::osclass::Linux ? Info::Api::Native : Info::Api::Emulated,
		Info::Io::FullDuplex,
		Info::Mixing::Server,
		Info::Implementor::External
	};
	// clang-format on
	devices.push_back(info);

	int result = 0;
	pa_mainloop *m = nullptr;
	pa_context *c = nullptr;
	bool doneConnect = false;
	pa_context_state_t cs = PA_CONTEXT_UNCONNECTED;
	pa_operation *o = nullptr;
	pa_operation_state_t s = PA_OPERATION_RUNNING;

	m = pa_mainloop_new();
	if(!m)
	{
		MPT_LOG(GetLogger(), LogError, "sounddev", MPT_USTRING("pa_mainloop_new"));
		goto cleanup;
	}
	c = pa_context_new(pa_mainloop_get_api(m), mpt::transcode<std::string>(mpt::common_encoding::utf8, mpt::ustring()).c_str());  // TODO: get AppInfo
	if(!c)
	{
		MPT_LOG(GetLogger(), LogError, "sounddev", MPT_USTRING("pa_context_new"));
		goto cleanup;
	}
	if(pa_context_connect(c, NULL, PA_CONTEXT_NOFLAGS, NULL) < 0)
	{
		MPT_LOG(GetLogger(), LogError, "sounddev", MPT_USTRING("pa_context_connect"));
		goto cleanup;
	}
	doneConnect = false;
	while(!doneConnect)
	{
		if(pa_mainloop_iterate(m, 1, &result) < 0)
		{
			MPT_LOG(GetLogger(), LogError, "sounddev", MPT_USTRING("pa_mainloop_iterate"));
			goto cleanup;
		}
		cs = pa_context_get_state(c);
		switch(cs)
		{
			case PA_CONTEXT_UNCONNECTED:
			case PA_CONTEXT_CONNECTING:
			case PA_CONTEXT_AUTHORIZING:
			case PA_CONTEXT_SETTING_NAME:
				break;
			case PA_CONTEXT_READY:
				doneConnect = true;
				break;
			case PA_CONTEXT_FAILED:
			case PA_CONTEXT_TERMINATED:
			default:
				{
					MPT_LOG(GetLogger(), LogError, "sounddev", MPT_USTRING("pa_context_connect"));
					goto cleanup;
				}
				break;
		}
	}
	o = pa_context_get_sink_info_list(c, &PulseAudioSinkInfoListCallback, &devices);
	if(!o)
	{
		MPT_LOG(GetLogger(), LogError, "sounddev", MPT_USTRING("pa_context_get_sink_info_list: ") + PulseErrorString(pa_context_errno(c)));
		goto cleanup;
	}
	s = PA_OPERATION_RUNNING;
	while((s = pa_operation_get_state(o)) == PA_OPERATION_RUNNING)
	{
		if(pa_mainloop_iterate(m, 1, &result) < 0)
		{
			MPT_LOG(GetLogger(), LogError, "sounddev", MPT_USTRING("pa_mainloop_iterate"));
			goto cleanup;
		}
	}
	if(s == PA_OPERATION_CANCELLED)
	{
		MPT_LOG(GetLogger(), LogError, "sounddev", MPT_USTRING("pa_operation_get_state"));
		goto cleanup;
	}
	goto cleanup;

cleanup:

	if(o)
	{
		pa_operation_unref(o);
		o = nullptr;
	}
	if(c)
	{
		pa_context_disconnect(c);
		pa_context_unref(c);
		c = nullptr;
	}
	if(m)
	{
		pa_mainloop_quit(m, 0);
		pa_mainloop_run(m, &result);
		pa_mainloop_free(m);
		m = nullptr;
	}

	return devices;
}


Pulseaudio::Pulseaudio(ILogger &logger, SoundDevice::Info info, SoundDevice::SysInfo sysInfo)
	: ThreadBase(logger, info, sysInfo)
	, m_PA_SimpleOutput(nullptr)
	, m_StatisticLastLatencyFrames(0)
{
	return;
}


SoundDevice::Caps Pulseaudio::InternalGetDeviceCaps()
{
	SoundDevice::Caps caps;
	caps.Available = true;  // TODO: poll PulseAudio
	caps.CanUpdateInterval = true;
	caps.CanSampleFormat = false;
	caps.CanExclusiveMode = true;
	caps.CanBoostThreadPriority = true;
	caps.CanKeepDeviceRunning = false;
	caps.CanUseHardwareTiming = true;
	caps.CanChannelMapping = false;
	caps.CanInput = false;
	caps.HasNamedInputSources = false;
	caps.CanDriverPanel = false;
	caps.HasInternalDither = false;
	caps.ExclusiveModeDescription = MPT_USTRING("Use early requests");
	caps.DefaultSettings.Latency = 0.030;
	caps.DefaultSettings.UpdateInterval = 0.005;
	caps.DefaultSettings.sampleFormat = SampleFormat::Float32;
	caps.DefaultSettings.ExclusiveMode = true;
	return caps;
}


SoundDevice::DynamicCaps Pulseaudio::GetDeviceDynamicCaps(const std::vector<uint32> &baseSampleRates)
{
	SoundDevice::DynamicCaps caps;
	caps.supportedSampleRates = baseSampleRates;
	caps.supportedExclusiveSampleRates = baseSampleRates;
	caps.supportedSampleFormats = {SampleFormat::Float32};
	caps.supportedExclusiveModeSampleFormats = {SampleFormat::Float32};
	return caps;
}


bool Pulseaudio::InternalIsOpen() const
{
	return m_PA_SimpleOutput;
}


bool Pulseaudio::InternalOpen()
{
	if(m_Settings.sampleFormat != SampleFormat::Float32)
	{
		InternalClose();
		return false;
	}
	int error = 0;
	pa_sample_spec ss = {};
	ss.format = PA_SAMPLE_FLOAT32;
	ss.rate = m_Settings.Samplerate;
	ss.channels = m_Settings.Channels;
	pa_buffer_attr ba = {};
	ba.minreq = mpt::align_up<uint32>(mpt::saturate_round<uint32>(m_Settings.GetBytesPerSecond() * m_Settings.UpdateInterval), m_Settings.GetBytesPerFrame());
	ba.maxlength = mpt::align_up<uint32>(mpt::saturate_round<uint32>(m_Settings.GetBytesPerSecond() * m_Settings.Latency), m_Settings.GetBytesPerFrame());
	ba.tlength = ba.maxlength - ba.minreq;
	ba.prebuf = ba.tlength;
	ba.fragsize = 0;
	m_EffectiveBufferAttributes = SoundDevice::BufferAttributes();
	m_EffectiveBufferAttributes.Latency = static_cast<double>(ba.maxlength) / static_cast<double>(m_Settings.GetBytesPerSecond());
	m_EffectiveBufferAttributes.UpdateInterval = static_cast<double>(ba.minreq) / static_cast<double>(m_Settings.GetBytesPerSecond());
	m_EffectiveBufferAttributes.NumBuffers = 1;
	m_OutputBuffer.resize(ba.minreq / m_Settings.sampleFormat.GetSampleSize());
	m_PA_SimpleOutput = pa_simple_new(
		NULL,
		mpt::transcode<std::string>(mpt::common_encoding::utf8, m_AppInfo.GetName()).c_str(),
		PA_STREAM_PLAYBACK,
		((GetDeviceInternalID() == MPT_USTRING("0")) ? NULL : mpt::transcode<std::string>(mpt::common_encoding::utf8, GetDeviceInternalID()).c_str()),
		mpt::transcode<std::string>(mpt::common_encoding::utf8, m_AppInfo.GetName()).c_str(),
		&ss,
		NULL,
		(m_Settings.ExclusiveMode ? &ba : NULL),
		&error);
	if(!m_PA_SimpleOutput)
	{
		SendDeviceMessage(LogError, MPT_UFORMAT_MESSAGE("pa_simple_new failed: {}")(PulseErrorString(error)));
		InternalClose();
		return false;
	}
	return true;
}


void Pulseaudio::InternalStartFromSoundThread()
{
	return;
}


void Pulseaudio::InternalFillAudioBuffer()
{
	bool needsClose = false;
	int error = 0;
	error = 0;
	pa_usec_t latency_usec = pa_simple_get_latency(m_PA_SimpleOutput, &error);
	if(error != 0)
	{
		SendDeviceMessage(LogError, MPT_UFORMAT_MESSAGE("pa_simple_get_latency failed: {}")(PulseErrorString(error)));
		RequestClose();
		return;
	}
	error = 0;
	// We add the update period to the latency because:
	//  1. PulseAudio latency calculation is done before we are actually
	//     refilling.
	//  2. We have 1 additional period latency becasue the writing is blocking and
	//     audio has will be calculated almost one period in advance in the worst
	//     case.
	// I think, in total we only need to add the period once.
	std::size_t latencyFrames = 0;
	latencyFrames += (latency_usec * m_Settings.Samplerate) / 1000000;
	latencyFrames += 1 * (m_OutputBuffer.size() / m_Settings.Channels);
	CallbackLockedAudioReadPrepare(m_OutputBuffer.size() / m_Settings.Channels, latencyFrames);
	CallbackLockedAudioProcess(m_OutputBuffer.data(), nullptr, m_OutputBuffer.size() / m_Settings.Channels);
	error = 0;
	static_assert(sizeof(somefloat32) == 4);
	if(pa_simple_write(m_PA_SimpleOutput, m_OutputBuffer.data(), m_OutputBuffer.size() * sizeof(somefloat32), &error) < 0)
	{
		SendDeviceMessage(LogError, MPT_UFORMAT_MESSAGE("pa_simple_write failed: {}")(PulseErrorString(error)));
		needsClose = true;
	}
	m_StatisticLastLatencyFrames.store(latencyFrames);
	CallbackLockedAudioProcessDone();
	if(needsClose)
	{
		RequestClose();
		return;
	}
}


void Pulseaudio::InternalWaitFromSoundThread()
{
	// We block in InternalFillAudioBuffer and thus have no need to wait further
	return;
}


SoundDevice::BufferAttributes Pulseaudio::InternalGetEffectiveBufferAttributes() const
{
	return m_EffectiveBufferAttributes;
}


SoundDevice::Statistics Pulseaudio::GetStatistics() const
{
	SoundDevice::Statistics stats;
	stats.InstantaneousLatency = static_cast<double>(m_StatisticLastLatencyFrames.load()) / static_cast<double>(m_Settings.Samplerate);
	stats.LastUpdateInterval = m_EffectiveBufferAttributes.UpdateInterval;
	stats.text = mpt::ustring();
	return stats;
}


void Pulseaudio::InternalStopFromSoundThread()
{
	int error = 0;
	bool oldVersion = false;
	std::vector<uint64> version = mpt::split_parse<uint64>(mpt::transcode<mpt::ustring>(mpt::common_encoding::utf8, pa_get_library_version() ? pa_get_library_version() : ""));
	if(!version.empty())
	{
		if(version[0] < 4)
		{
			oldVersion = true;
		}
	}
	if(oldVersion)
	{
		// draining is awfully slow with pulseaudio version < 4.0.0,
		// just flush there
		error = 0;
		if(pa_simple_flush(m_PA_SimpleOutput, &error) < 0)
		{
			SendDeviceMessage(LogError, MPT_UFORMAT_MESSAGE("pa_simple_flush failed: {}")(PulseErrorString(error)));
		}
	} else
	{
		error = 0;
		if(pa_simple_drain(m_PA_SimpleOutput, &error) < 0)
		{
			SendDeviceMessage(LogError, MPT_UFORMAT_MESSAGE("pa_simple_drain failed: {}")(PulseErrorString(error)));
		}
	}
	return;
}


bool Pulseaudio::InternalClose()
{
	if(m_PA_SimpleOutput)
	{
		pa_simple_free(m_PA_SimpleOutput);
		m_PA_SimpleOutput = nullptr;
	}
	m_OutputBuffer.resize(0);
	m_EffectiveBufferAttributes = SoundDevice::BufferAttributes();
	return true;
}


Pulseaudio::~Pulseaudio()
{
	return;
}


#endif  // MPT_WITH_PULSEAUDIO

#endif  // MPT_ENABLE_PULSEAUDIO_FULL


}  // namespace SoundDevice


OPENMPT_NAMESPACE_END
