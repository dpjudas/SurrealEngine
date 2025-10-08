/* SPDX-License-Identifier: BSD-3-Clause */
/* SPDX-FileCopyrightText: OpenMPT Project Developers and Contributors */


#include "openmpt/all/BuildSettings.hpp"
#include "openmpt/all/PlatformFixes.hpp"

#include "SoundDeviceRtAudio.hpp"

#include "SoundDevice.hpp"
#include "SoundDeviceBase.hpp"

#include "mpt/base/alloc.hpp"
#include "mpt/base/pointer.hpp"
#include "mpt/base/saturate_cast.hpp"
#include "mpt/base/saturate_round.hpp"
#include "mpt/format/message_macros.hpp"
#include "mpt/format/simple.hpp"
#include "mpt/parse/parse.hpp"
#include "mpt/string/types.hpp"
#include "mpt/string/utility.hpp"
#include "mpt/string_transcode/transcode.hpp"
#include "openmpt/base/Types.hpp"
#include "openmpt/logging/Logger.hpp"
#include "openmpt/soundbase/SampleFormat.hpp"

#include <algorithm>
#include <array>
#include <memory>
#include <string>
#include <utility>
#include <vector>


OPENMPT_NAMESPACE_BEGIN


namespace SoundDevice
{


#ifdef MPT_WITH_RTAUDIO



static constexpr uint8 ParseDigit(char c)
{
	return c - '0';
}

using RtAudioVersion = std::array<unsigned int, 3>;

static constexpr RtAudioVersion ParseVersion(const char *str)
{
	RtAudioVersion version = {0, 0, 0};
	std::size_t version_pos = 0;
	while(*str)
	{
		const char c = *str;
		if(c == '.')
		{
			version_pos += 1;
		} else
		{
			version[version_pos] = (version[version_pos] * 10) + ParseDigit(c);
		}
		str++;
	}
	return version;
}

static constexpr bool RtAudioCheckVersion(const char *wanted_)
{
	RtAudioVersion actual = ParseVersion(RTAUDIO_VERSION);
	RtAudioVersion wanted = ParseVersion(wanted_);
	if(actual[0] > wanted[0])
	{
		return true;
	} else if(actual[0] == wanted[0])
	{
		if(actual[1] > wanted[1])
		{
			return true;
		} else if(actual[1] == wanted[1])
		{
			return (actual[2] >= wanted[2]);
		} else
		{
			return false;
		}
	} else
	{
		return false;
	}
}

template <typename RtAudio = ::RtAudio, bool is_v5_1_0 = RtAudioCheckVersion("5.1.0")>
struct RtAudio_v5_1_0_Shim
{
};

template <typename RtAudio>
struct RtAudio_v5_1_0_Shim<RtAudio, true>
{

	static inline std::string getApiName(typename RtAudio::Api api)
	{
		return RtAudio::getApiName(api);
	}

	static inline std::string getApiDisplayName(typename RtAudio::Api api)
	{
		return RtAudio::getApiDisplayName(api);
	}

	static inline typename RtAudio::Api getCompiledApiByName(const std::string &name)
	{
		return RtAudio::getCompiledApiByName(name);
	}
};

template <typename RtAudio>
struct RtAudio_v5_1_0_Shim<RtAudio, false>
{

	static constexpr const char *rtaudio_api_names[][2] = {
		{"unspecified", "Unknown"        },
		{"alsa",        "ALSA"           },
		{"pulse",       "Pulse"          },
		{"oss",         "OpenSoundSystem"},
		{"jack",        "Jack"           },
		{"core",        "CoreAudio"      },
		{"wasapi",      "WASAPI"         },
		{"asio",        "ASIO"           },
		{"ds",          "DirectSound"    },
		{"dummy",       "Dummy"          },
	};

	static constexpr typename RtAudio::Api rtaudio_all_apis[] = {
		RtAudio::UNIX_JACK,
		RtAudio::LINUX_PULSE,
		RtAudio::LINUX_ALSA,
		RtAudio::LINUX_OSS,
		RtAudio::WINDOWS_ASIO,
		RtAudio::WINDOWS_WASAPI,
		RtAudio::WINDOWS_DS,
		RtAudio::MACOSX_CORE,
		RtAudio::RTAUDIO_DUMMY,
		RtAudio::UNSPECIFIED,
	};

	static inline std::string getApiName(typename RtAudio::Api api)
	{
		if(api < 0)
		{
			return std::string();
		}
		if(api >= mpt::saturate_cast<int>(std::size(rtaudio_api_names)))
		{
			return std::string();
		}
		return rtaudio_api_names[api][0];
	}

	static inline std::string getApiDisplayName(typename RtAudio::Api api)
	{
		if(api < 0)
		{
			return std::string();
		}
		if(api >= mpt::saturate_cast<int>(std::size(rtaudio_api_names)))
		{
			return std::string();
		}
		return rtaudio_api_names[api][1];
	}

	static inline typename RtAudio::Api getCompiledApiByName(const std::string &name)
	{
		for(std::size_t i = 0; i < std::size(rtaudio_api_names); ++i)
		{
			if(name == rtaudio_api_names[rtaudio_all_apis[i]][0])
			{
				return rtaudio_all_apis[i];
			}
		}
		return RtAudio::UNSPECIFIED;
	}
};

struct RtAudioShim
	: RtAudio_v5_1_0_Shim<>
{
};


static RtAudioFormat SampleFormatToRtAudioFormat(SampleFormat sampleFormat)
{
	RtAudioFormat result = RtAudioFormat();
	if(sampleFormat.IsFloat())
	{
		switch(sampleFormat.GetBitsPerSample())
		{
			case 32: result = RTAUDIO_FLOAT32; break;
			case 64: result = RTAUDIO_FLOAT64; break;
		}
	} else if(sampleFormat.IsInt())
	{
		switch(sampleFormat.GetBitsPerSample())
		{
			case 8: result = RTAUDIO_SINT8; break;
			case 16: result = RTAUDIO_SINT16; break;
			case 24: result = RTAUDIO_SINT24; break;
			case 32: result = RTAUDIO_SINT32; break;
		}
	}
	return result;
}


CRtAudioDevice::CRtAudioDevice(ILogger &logger, SoundDevice::Info info, SoundDevice::SysInfo sysInfo)
	: SoundDevice::Base(logger, info, sysInfo)
	, m_RtAudio(std::unique_ptr<RtAudio>())
	, m_FramesPerChunk(0)
{
	m_CurrentFrameBufferOutput = nullptr;
	m_CurrentFrameBufferInput = nullptr;
	m_CurrentFrameBufferCount = 0;
	m_CurrentStreamTime = 0.0;
	m_StatisticLatencyFrames.store(0);
	m_StatisticPeriodFrames.store(0);
#if MPT_RTAUDIO_BEFORE(6)
	try
	{
#endif
#if MPT_RTAUDIO_AT_LEAST(6)
		m_RtAudio = std::make_unique<RtAudio>(GetApi(info), [this](RtAudioErrorType type, const std::string &errorText)
											  { this->SendError(type, errorText); });
#else
	m_RtAudio = std::make_unique<RtAudio>(GetApi(info));
#endif
#if MPT_RTAUDIO_BEFORE(6)
	} catch(const RtAudioError &)
	{
		// nothing
	}
#endif
}


CRtAudioDevice::~CRtAudioDevice()
{
	Close();
}


bool CRtAudioDevice::InternalOpen()
{
#if MPT_RTAUDIO_BEFORE(6)
	try
	{
#endif
		if(SampleFormatToRtAudioFormat(m_Settings.sampleFormat) == RtAudioFormat())
		{
			return false;
		}
		if(ChannelMapping::BaseChannel(m_Settings.Channels, m_Settings.Channels.ToDevice(0)) != m_Settings.Channels)
		{  // only simple base channel mappings are supported
			return false;
		}
		m_OutputStreamParameters.deviceId = GetDevice(GetDeviceInfo());
		m_OutputStreamParameters.nChannels = m_Settings.Channels;
		m_OutputStreamParameters.firstChannel = m_Settings.Channels.ToDevice(0);
		m_InputStreamParameters.deviceId = GetDevice(GetDeviceInfo());
		m_InputStreamParameters.nChannels = m_Settings.InputChannels;
		m_InputStreamParameters.firstChannel = m_Settings.InputSourceID;
		m_FramesPerChunk = mpt::saturate_round<int>(m_Settings.UpdateInterval * m_Settings.Samplerate);
		m_StreamOptions.flags = RtAudioStreamFlags();
		m_StreamOptions.numberOfBuffers = mpt::saturate_round<int>(m_Settings.Latency * m_Settings.Samplerate / m_FramesPerChunk);
		m_StreamOptions.priority = 0;
		m_StreamOptions.streamName = mpt::transcode<std::string>(mpt::common_encoding::utf8, m_AppInfo.GetName());
		if(m_Settings.BoostThreadPriority)
		{
			m_StreamOptions.flags |= RTAUDIO_SCHEDULE_REALTIME;
			m_StreamOptions.priority = m_AppInfo.BoostedThreadPriorityXP;
		}
		if(m_Settings.ExclusiveMode)
		{
			//m_FramesPerChunk = 0; // auto
			m_StreamOptions.flags |= RTAUDIO_MINIMIZE_LATENCY | RTAUDIO_HOG_DEVICE;
			m_StreamOptions.numberOfBuffers = 2;
		}
		if(m_RtAudio->getCurrentApi() == RtAudio::Api::WINDOWS_WASAPI)
		{
			m_Flags.WantsClippedOutput = true;
		} else if(m_RtAudio->getCurrentApi() == RtAudio::Api::WINDOWS_DS)
		{
			m_Flags.WantsClippedOutput = (GetSysInfo().IsOriginal() && GetSysInfo().WindowsVersion.IsAtLeast(mpt::osinfo::windows::Version::WinVista));
		}
#if MPT_RTAUDIO_AT_LEAST(6)
		if(IsError(m_RtAudio->openStream((m_OutputStreamParameters.nChannels > 0) ? &m_OutputStreamParameters : nullptr, (m_InputStreamParameters.nChannels > 0) ? &m_InputStreamParameters : nullptr, SampleFormatToRtAudioFormat(m_Settings.sampleFormat), m_Settings.Samplerate, &m_FramesPerChunk, &RtAudioCallback, this, &m_StreamOptions)))
		{
			return false;
		}
#else
	m_RtAudio->openStream((m_OutputStreamParameters.nChannels > 0) ? &m_OutputStreamParameters : nullptr, (m_InputStreamParameters.nChannels > 0) ? &m_InputStreamParameters : nullptr, SampleFormatToRtAudioFormat(m_Settings.sampleFormat), m_Settings.Samplerate, &m_FramesPerChunk, &RtAudioCallback, this, &m_StreamOptions, nullptr);
#endif
#if MPT_RTAUDIO_BEFORE(6)
	} catch(const RtAudioError &e)
	{
		SendError(e);
		return false;
	}
#endif
	return true;
}


bool CRtAudioDevice::InternalClose()
{
#if MPT_RTAUDIO_BEFORE(6)
	try
	{
#endif
		m_RtAudio->closeStream();
#if MPT_RTAUDIO_BEFORE(6)
	} catch(const RtAudioError &e)
	{
		SendError(e);
		return false;
	}
#endif
	return true;
}


bool CRtAudioDevice::InternalStart()
{
#if MPT_RTAUDIO_BEFORE(6)
	try
	{
#endif
#if MPT_RTAUDIO_AT_LEAST(6)
		if(IsError(m_RtAudio->startStream()))
		{
			return false;
		}
#else
	m_RtAudio->startStream();
#endif
#if MPT_RTAUDIO_BEFORE(6)
	} catch(const RtAudioError &e)
	{
		SendError(e);
		return false;
	}
#endif
	return true;
}


void CRtAudioDevice::InternalStop()
{
#if MPT_RTAUDIO_BEFORE(6)
	try
	{
#endif
#if MPT_RTAUDIO_AT_LEAST(6)
		if(IsError(m_RtAudio->stopStream()))
		{
			return;
		}
#else
	m_RtAudio->stopStream();
#endif
#if MPT_RTAUDIO_BEFORE(6)
	} catch(const RtAudioError &e)
	{
		SendError(e);
		return;
	}
#endif
	return;
}


void CRtAudioDevice::InternalFillAudioBuffer()
{
	if(m_CurrentFrameBufferCount == 0)
	{
		return;
	}
	CallbackLockedAudioReadPrepare(m_CurrentFrameBufferCount, m_FramesPerChunk * m_StreamOptions.numberOfBuffers);
	CallbackLockedAudioProcessVoid(m_CurrentFrameBufferOutput, m_CurrentFrameBufferInput, m_CurrentFrameBufferCount);
	m_StatisticLatencyFrames.store(m_CurrentFrameBufferCount * m_StreamOptions.numberOfBuffers);
	m_StatisticPeriodFrames.store(m_CurrentFrameBufferCount);
	CallbackLockedAudioProcessDone();
}


int64 CRtAudioDevice::InternalGetStreamPositionFrames() const
{
	return mpt::saturate_round<int64>(m_RtAudio->getStreamTime() * m_RtAudio->getStreamSampleRate());
}


SoundDevice::BufferAttributes CRtAudioDevice::InternalGetEffectiveBufferAttributes() const
{
	SoundDevice::BufferAttributes bufferAttributes;
	bufferAttributes.Latency = m_FramesPerChunk * m_StreamOptions.numberOfBuffers / static_cast<double>(m_Settings.Samplerate);
	bufferAttributes.UpdateInterval = m_FramesPerChunk / static_cast<double>(m_Settings.Samplerate);
	bufferAttributes.NumBuffers = m_StreamOptions.numberOfBuffers;
	return bufferAttributes;
}


int CRtAudioDevice::RtAudioCallback(void *outputBuffer, void *inputBuffer, unsigned int nFrames, double streamTime, RtAudioStreamStatus status, void *userData)
{
	mpt::void_ptr<CRtAudioDevice>(userData)->AudioCallback(outputBuffer, inputBuffer, nFrames, streamTime, status);
	return 0;  // continue
}


void CRtAudioDevice::AudioCallback(void *outputBuffer, void *inputBuffer, unsigned int nFrames, double streamTime, RtAudioStreamStatus status)
{
	m_CurrentFrameBufferOutput = outputBuffer;
	m_CurrentFrameBufferInput = inputBuffer;
	m_CurrentFrameBufferCount = nFrames;
	m_CurrentStreamTime = streamTime;
	CallbackFillAudioBufferLocked();
	m_CurrentFrameBufferCount = 0;
	m_CurrentFrameBufferOutput = 0;
	m_CurrentFrameBufferInput = 0;
	if(status != RtAudioStreamStatus())
	{
		// maybe
		// RequestRestart();
	}
}


SoundDevice::Statistics CRtAudioDevice::GetStatistics() const
{
	MPT_SOUNDDEV_TRACE_SCOPE();
	SoundDevice::Statistics result;
	long latency = 0;
#if MPT_RTAUDIO_BEFORE(6)
	try
	{
#endif
		if(m_RtAudio->isStreamOpen())
		{
			latency = m_RtAudio->getStreamLatency();
			if(m_Settings.InputChannels > 0 && m_Settings.Channels > 0)
			{
				latency /= 2;
			}
		}
#if MPT_RTAUDIO_BEFORE(6)
	} catch(const RtAudioError &)
	{
		latency = 0;
	}
#endif
	if(latency > 0)
	{
		result.InstantaneousLatency = latency / static_cast<double>(m_Settings.Samplerate);
		result.LastUpdateInterval = m_StatisticPeriodFrames.load() / static_cast<double>(m_Settings.Samplerate);
	} else
	{
		result.InstantaneousLatency = m_StatisticLatencyFrames.load() / static_cast<double>(m_Settings.Samplerate);
		result.LastUpdateInterval = m_StatisticPeriodFrames.load() / static_cast<double>(m_Settings.Samplerate);
	}
	return result;
}


SoundDevice::Caps CRtAudioDevice::InternalGetDeviceCaps()
{
	MPT_SOUNDDEV_TRACE_SCOPE();
	SoundDevice::Caps caps;
	if(!m_RtAudio)
	{
		return caps;
	}
	RtAudio::DeviceInfo rtinfo;
#if MPT_RTAUDIO_BEFORE(6)
	try
	{
#endif
		rtinfo = m_RtAudio->getDeviceInfo(GetDevice(GetDeviceInfo()));
#if MPT_RTAUDIO_BEFORE(6)
	} catch(const RtAudioError &)
	{
		return caps;
	}
#endif
#if MPT_RTAUDIO_AT_LEAST(6)
	caps.Available = true;
#else
	caps.Available = rtinfo.probed;
#endif
	caps.CanUpdateInterval = true;
	caps.CanSampleFormat = true;
	caps.CanExclusiveMode = true;
	caps.CanBoostThreadPriority = true;
	caps.CanKeepDeviceRunning = false;
	caps.CanUseHardwareTiming = false;
	caps.CanChannelMapping = false;  // only base channel is supported, and that does not make too much sense for non-ASIO backends
	caps.CanInput = (rtinfo.inputChannels > 0);
	caps.HasNamedInputSources = true;
	caps.CanDriverPanel = false;
	caps.HasInternalDither = false;
	caps.ExclusiveModeDescription = MPT_USTRING("Exclusive Mode");
#if MPT_RTAUDIO_AT_LEAST(6)
	if(rtinfo.currentSampleRate > 0)
	{
		caps.DefaultSettings.Samplerate = rtinfo.currentSampleRate;
	}
#endif
	return caps;
}


SoundDevice::DynamicCaps CRtAudioDevice::GetDeviceDynamicCaps(const std::vector<uint32> & /* baseSampleRates */)
{
	MPT_SOUNDDEV_TRACE_SCOPE();
	SoundDevice::DynamicCaps caps;
	RtAudio::DeviceInfo rtinfo;
#if MPT_RTAUDIO_BEFORE(6)
	try
	{
#endif
		rtinfo = m_RtAudio->getDeviceInfo(GetDevice(GetDeviceInfo()));
#if MPT_RTAUDIO_BEFORE(6)
	} catch(const RtAudioError &)
	{
		return caps;
	}
#endif
#if MPT_RTAUDIO_BEFORE(6)
	if(!rtinfo.probed)
	{
		return caps;
	}
#endif
	caps.inputSourceNames.clear();
	for(unsigned int channel = 0; channel < rtinfo.inputChannels; ++channel)
	{
		caps.inputSourceNames.push_back(std::make_pair(channel, MPT_USTRING("Channel ") + mpt::format<mpt::ustring>::dec(channel + 1)));
	}
	mpt::append(caps.supportedSampleRates, rtinfo.sampleRates);
	std::reverse(caps.supportedSampleRates.begin(), caps.supportedSampleRates.end());
	mpt::append(caps.supportedExclusiveSampleRates, rtinfo.sampleRates);
	std::reverse(caps.supportedExclusiveSampleRates.begin(), caps.supportedExclusiveSampleRates.end());
	caps.supportedSampleFormats = {SampleFormat::Float32};
	caps.supportedExclusiveModeSampleFormats.clear();
	if(rtinfo.nativeFormats & RTAUDIO_SINT8)
	{
		caps.supportedExclusiveModeSampleFormats.push_back(SampleFormat::Int8);
	}
	if(rtinfo.nativeFormats & RTAUDIO_SINT16)
	{
		caps.supportedExclusiveModeSampleFormats.push_back(SampleFormat::Int16);
	}
	if(rtinfo.nativeFormats & RTAUDIO_SINT24)
	{
		caps.supportedExclusiveModeSampleFormats.push_back(SampleFormat::Int24);
	}
	if(rtinfo.nativeFormats & RTAUDIO_SINT32)
	{
		caps.supportedExclusiveModeSampleFormats.push_back(SampleFormat::Int32);
	}
	if(rtinfo.nativeFormats & RTAUDIO_FLOAT32)
	{
		caps.supportedExclusiveModeSampleFormats.push_back(SampleFormat::Float32);
	}
	if(rtinfo.nativeFormats & RTAUDIO_FLOAT64)
	{
		caps.supportedExclusiveModeSampleFormats.push_back(SampleFormat::Float64);
	}
	for(unsigned int channel = 0; channel < rtinfo.outputChannels; ++channel)
	{
		caps.channelNames.push_back(MPT_UFORMAT_MESSAGE("Output Channel {}")(channel));
	}
	for(unsigned int channel = 0; channel < rtinfo.inputChannels; ++channel)
	{
		caps.inputSourceNames.push_back(std::make_pair(static_cast<uint32>(channel), MPT_UFORMAT_MESSAGE("Input Channel {}")(channel)));
	}
#if MPT_RTAUDIO_AT_LEAST(6)
	if(rtinfo.currentSampleRate > 0)
	{
		caps.currentSampleRate = rtinfo.currentSampleRate;
	}
#endif
	return caps;
}


#if MPT_RTAUDIO_AT_LEAST(6)

bool CRtAudioDevice::IsError(const RtAudioErrorType &e)
{
	return (e != RTAUDIO_NO_ERROR) && (e != RTAUDIO_WARNING);
}

#endif


#if MPT_RTAUDIO_AT_LEAST(6)

void CRtAudioDevice::SendError(const RtAudioErrorType &e, const std::string &errorText)
{
	LogLevel level = LogError;
	switch(e)
	{
		case RTAUDIO_WARNING:
			level = LogWarning;
			break;
		case RTAUDIO_UNKNOWN_ERROR:
			level = LogError;
			break;
		case RTAUDIO_NO_DEVICES_FOUND:
			level = LogError;
			break;
		case RTAUDIO_INVALID_DEVICE:
			level = LogError;
			break;
		case RTAUDIO_DEVICE_DISCONNECT:
			level = LogError;
			break;
		case RTAUDIO_MEMORY_ERROR:
			level = LogError;
			break;
		case RTAUDIO_INVALID_PARAMETER:
			level = LogError;
			break;
		case RTAUDIO_INVALID_USE:
			level = LogError;
			break;
		case RTAUDIO_DRIVER_ERROR:
			level = LogError;
			break;
		case RTAUDIO_SYSTEM_ERROR:
			level = LogError;
			break;
		case RTAUDIO_THREAD_ERROR:
			level = LogError;
			break;
		default:
			level = LogError;
			break;
	}
	SendDeviceMessage(level, mpt::transcode<mpt::ustring>(mpt::common_encoding::utf8, errorText));
}

#else

void CRtAudioDevice::SendError(const RtAudioError &e)
{
	LogLevel level = LogError;
	switch(e.getType())
	{
		case RtAudioError::WARNING:
			level = LogWarning;
			break;
		case RtAudioError::DEBUG_WARNING:
			level = LogDebug;
			break;
		case RtAudioError::UNSPECIFIED:
			level = LogError;
			break;
		case RtAudioError::NO_DEVICES_FOUND:
			level = LogError;
			break;
		case RtAudioError::INVALID_DEVICE:
			level = LogError;
			break;
		case RtAudioError::MEMORY_ERROR:
			level = LogError;
			break;
		case RtAudioError::INVALID_PARAMETER:
			level = LogError;
			break;
		case RtAudioError::INVALID_USE:
			level = LogError;
			break;
		case RtAudioError::DRIVER_ERROR:
			level = LogError;
			break;
		case RtAudioError::SYSTEM_ERROR:
			level = LogError;
			break;
		case RtAudioError::THREAD_ERROR:
			level = LogError;
			break;
		default:
			level = LogError;
			break;
	}
	SendDeviceMessage(level, mpt::transcode<mpt::ustring>(mpt::common_encoding::utf8, e.getMessage()));
}

#endif


RtAudio::Api CRtAudioDevice::GetApi(SoundDevice::Info info)
{
	std::vector<mpt::ustring> apidev = mpt::split(info.internalID, MPT_USTRING(","));
	if(apidev.size() != 2)
	{
		return RtAudio::UNSPECIFIED;
	}
	return RtAudioShim::getCompiledApiByName(mpt::transcode<std::string>(mpt::common_encoding::utf8, apidev[0]));
}


unsigned int CRtAudioDevice::GetDevice(SoundDevice::Info info)
{
	std::vector<mpt::ustring> apidev = mpt::split(info.internalID, MPT_USTRING(","));
	if(apidev.size() != 2)
	{
		return 0;
	}
	return mpt::parse<unsigned int>(apidev[1]);
}


std::vector<SoundDevice::Info> CRtAudioDevice::EnumerateDevices(ILogger &logger, SoundDevice::SysInfo sysInfo)
{
#if 0
	auto GetLogger = [&]() -> ILogger &
	{
		return logger;
	};
#else
	MPT_UNUSED(logger);
#endif
	std::vector<SoundDevice::Info> devices;
	std::vector<RtAudio::Api> apis;
	RtAudio::getCompiledApi(apis);
	for(const auto &api : apis)
	{
		if(api == RtAudio::RTAUDIO_DUMMY)
		{
			continue;
		}
#if MPT_RTAUDIO_BEFORE(6)
		try
		{
#endif
			RtAudio rtaudio(api);
#if MPT_RTAUDIO_AT_LEAST(6)
			MPT_DISCARD(rtaudio.getDeviceCount());
			std::vector<unsigned int> deviceIDs = rtaudio.getDeviceIds();
			for(const auto &device : deviceIDs)
#else
		for(unsigned int device = 0; device < rtaudio.getDeviceCount(); ++device)
#endif
			{
				RtAudio::DeviceInfo rtinfo;
#if MPT_RTAUDIO_BEFORE(6)
				try
				{
#endif
					rtinfo = rtaudio.getDeviceInfo(device);
#if MPT_RTAUDIO_BEFORE(6)
				} catch(const RtAudioError &)
				{
					continue;
				}
#endif
#if MPT_RTAUDIO_AT_LEAST(6)
				if(rtinfo.ID == 0)
				{
					continue;
				}
#else
			if(!rtinfo.probed)
			{
				continue;
			}
#endif
				SoundDevice::Info info = SoundDevice::Info();
				info.type = MPT_USTRING("RtAudio") + MPT_USTRING("-") + mpt::transcode<mpt::ustring>(mpt::common_encoding::utf8, RtAudioShim::getApiName(rtaudio.getCurrentApi()));
				std::vector<mpt::ustring> apidev;
				apidev.push_back(mpt::transcode<mpt::ustring>(mpt::common_encoding::utf8, RtAudioShim::getApiName(rtaudio.getCurrentApi())));
				apidev.push_back(mpt::format<mpt::ustring>::val(device));
				info.internalID = mpt::join(apidev, MPT_USTRING(","));
				info.name = mpt::transcode<mpt::ustring>(mpt::common_encoding::utf8, rtinfo.name);
				info.apiName = mpt::transcode<mpt::ustring>(mpt::common_encoding::utf8, RtAudioShim::getApiDisplayName(rtaudio.getCurrentApi()));
				info.extraData[MPT_USTRING("RtAudio-ApiDisplayName")] = mpt::transcode<mpt::ustring>(mpt::common_encoding::utf8, RtAudioShim::getApiDisplayName(rtaudio.getCurrentApi()));
				info.apiPath.push_back(MPT_USTRING("RtAudio"));
				info.useNameAsIdentifier = true;
				// clang-format off
				switch(rtaudio.getCurrentApi())
				{
				case RtAudio::LINUX_ALSA:
					info.apiName = MPT_USTRING("ALSA");
					info.default_ = (rtinfo.isDefaultOutput ? Info::Default::Named : Info::Default::None);
					info.flags = {
						sysInfo.SystemClass == mpt::osinfo::osclass::Linux ? Info::Usability::Usable : Info::Usability::Experimental,
						Info::Level::Secondary,
						Info::Compatible::No,
						sysInfo.SystemClass == mpt::osinfo::osclass::Linux ? Info::Api::Native : Info::Api::Emulated,
						Info::Io::FullDuplex,
						sysInfo.SystemClass == mpt::osinfo::osclass::Linux ? Info::Mixing::Hardware : Info::Mixing::Software,
						Info::Implementor::External
					};
					break;
				case RtAudio::LINUX_PULSE:
					info.apiName = MPT_USTRING("PulseAudio");
					info.default_ = (rtinfo.isDefaultOutput ? Info::Default::Managed : Info::Default::None);
					info.flags = {
						sysInfo.SystemClass == mpt::osinfo::osclass::Linux ? Info::Usability::Usable : Info::Usability::Experimental,
						Info::Level::Secondary,
						Info::Compatible::No,
						sysInfo.SystemClass == mpt::osinfo::osclass::Linux ? Info::Api::Native : Info::Api::Emulated,
						Info::Io::FullDuplex,
						Info::Mixing::Server,
						Info::Implementor::External
					};
					break;
				case RtAudio::LINUX_OSS:
					info.apiName = MPT_USTRING("OSS");
					info.default_ = (rtinfo.isDefaultOutput ? Info::Default::Named : Info::Default::None);
					info.flags = {
						sysInfo.SystemClass == mpt::osinfo::osclass::BSD_ ? Info::Usability::Usable : sysInfo.SystemClass == mpt::osinfo::osclass::Linux ? Info::Usability::Deprecated : Info::Usability::NotAvailable,
						Info::Level::Secondary,
						Info::Compatible::No,
						sysInfo.SystemClass == mpt::osinfo::osclass::BSD_ ? Info::Api::Native : sysInfo.SystemClass == mpt::osinfo::osclass::Linux ? Info::Api::Emulated : Info::Api::Emulated,
						Info::Io::FullDuplex,
						sysInfo.SystemClass == mpt::osinfo::osclass::BSD_ ? Info::Mixing::Hardware : sysInfo.SystemClass == mpt::osinfo::osclass::Linux ? Info::Mixing::Software : Info::Mixing::Software,
						Info::Implementor::External
					};
					break;
				case RtAudio::UNIX_JACK:
					info.apiName = MPT_USTRING("JACK");
					info.default_ = (rtinfo.isDefaultOutput ? Info::Default::Managed : Info::Default::None);
					info.flags = {
						sysInfo.SystemClass == mpt::osinfo::osclass::Linux ? Info::Usability::Usable : sysInfo.SystemClass == mpt::osinfo::osclass::Darwin ? Info::Usability::Usable : Info::Usability::Experimental,
						Info::Level::Primary,
						Info::Compatible::Yes,
						sysInfo.SystemClass == mpt::osinfo::osclass::Linux ? Info::Api::Native : Info::Api::Emulated,
						Info::Io::FullDuplex,
						Info::Mixing::Server,
						Info::Implementor::External
					};
					break;
				case RtAudio::MACOSX_CORE:
					info.apiName = MPT_USTRING("CoreAudio");
					info.default_ = (rtinfo.isDefaultOutput ? Info::Default::Named : Info::Default::None);
					info.flags = {
						sysInfo.SystemClass == mpt::osinfo::osclass::Darwin ? Info::Usability::Usable : Info::Usability::NotAvailable,
						Info::Level::Primary,
						Info::Compatible::Yes,
						sysInfo.SystemClass == mpt::osinfo::osclass::Darwin ? Info::Api::Native : Info::Api::Emulated,
						Info::Io::FullDuplex,
						Info::Mixing::Server,
						Info::Implementor::External
					};
					break;
				case RtAudio::WINDOWS_WASAPI:
					info.apiName = MPT_USTRING("WASAPI");
					info.default_ = (rtinfo.isDefaultOutput ? Info::Default::Named : Info::Default::None);
					info.flags = {
						sysInfo.SystemClass == mpt::osinfo::osclass::Windows ?
							sysInfo.IsWindowsOriginal() ?
								sysInfo.WindowsVersion.IsAtLeast(mpt::osinfo::windows::Version::Win7) ?
									Info::Usability::Usable
								:
									sysInfo.WindowsVersion.IsAtLeast(mpt::osinfo::windows::Version::WinVista) ?
										Info::Usability::Experimental
									:
										Info::Usability::NotAvailable
							:
								Info::Usability::Usable
						:
							Info::Usability::NotAvailable,
						Info::Level::Secondary,
						Info::Compatible::No,
						sysInfo.SystemClass == mpt::osinfo::osclass::Windows ? Info::Api::Native : Info::Api::Emulated,
						Info::Io::FullDuplex,
						Info::Mixing::Server,
						Info::Implementor::External
					};
					break;
				case RtAudio::WINDOWS_ASIO:
					info.apiName = MPT_USTRING("ASIO");
					info.default_ = (rtinfo.isDefaultOutput ? Info::Default::Named : Info::Default::None);
					info.flags = {
						sysInfo.SystemClass == mpt::osinfo::osclass::Windows ? sysInfo.IsWindowsOriginal() ? Info::Usability::Usable : Info::Usability::Experimental : Info::Usability::NotAvailable,
						Info::Level::Secondary,
						Info::Compatible::No,
						sysInfo.SystemClass == mpt::osinfo::osclass::Windows && sysInfo.IsWindowsOriginal() ? Info::Api::Native : Info::Api::Emulated,
						Info::Io::FullDuplex,
						Info::Mixing::Hardware,
						Info::Implementor::External
					};
					break;
				case RtAudio::WINDOWS_DS:
					info.apiName = MPT_USTRING("DirectSound");
					info.default_ = (rtinfo.isDefaultOutput ? Info::Default::Managed : Info::Default::None);
					info.flags = {
						Info::Usability::Broken, // sysInfo.SystemClass == mpt::osinfo::osclass::Windows ? sysInfo.IsWindowsOriginal() && sysInfo.WindowsVersion.IsBefore(mpt::Windows::Version::Win7) ? Info::Usability:Usable : Info::Usability::Deprecated : Info::Usability::NotAvailable,
						Info::Level::Secondary,
						Info::Compatible::No,
						sysInfo.SystemClass == mpt::osinfo::osclass::Windows ? sysInfo.IsWindowsWine() ? Info::Api::Emulated : sysInfo.WindowsVersion.IsAtLeast(mpt::osinfo::windows::Version::WinVista) ? Info::Api::Emulated : Info::Api::Native : Info::Api::Emulated,
						Info::Io::FullDuplex,
						Info::Mixing::Software,
						Info::Implementor::External
					};
					break;
				default:
					// nothing
					break;
				}
				// clang-format on

				devices.push_back(info);
			}
#if MPT_RTAUDIO_BEFORE(6)
		} catch(const RtAudioError &)
		{
			// nothing
		}
#endif
	}
	return devices;
}


#endif  // MPT_WITH_RTAUDIO


}  // namespace SoundDevice


OPENMPT_NAMESPACE_END
