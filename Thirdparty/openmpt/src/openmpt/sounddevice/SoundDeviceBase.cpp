/* SPDX-License-Identifier: BSD-3-Clause */
/* SPDX-FileCopyrightText: Olivier Lapicque */
/* SPDX-FileCopyrightText: OpenMPT Project Developers and Contributors */


#include "openmpt/all/BuildSettings.hpp"
#include "openmpt/all/PlatformFixes.hpp"

#include "SoundDeviceBase.hpp"

#include "SoundDeviceCallback.hpp"

#include "mpt/base/saturate_round.hpp"
#include "mpt/format/simple.hpp"
#include "mpt/string/types.hpp"
#include "openmpt/base/Types.hpp"
#include "openmpt/logging/Logger.hpp"
#include "openmpt/soundbase/SampleFormat.hpp"

#include <algorithm>
#include <vector>

#include <cassert>
#include <cstddef>


OPENMPT_NAMESPACE_BEGIN


namespace SoundDevice
{


Base::Base(ILogger &logger, SoundDevice::Info info, SoundDevice::SysInfo sysInfo)
	: m_Logger(logger)
	, m_Callback(nullptr)
	, m_MessageReceiver(nullptr)
	, m_Info(info)
	, m_SysInfo(sysInfo)
	, m_StreamPositionOutputFrames(0)
	, m_RequestFlags(0)
{
	MPT_SOUNDDEV_TRACE_SCOPE();

	m_DeviceUnavailableOnOpen = false;

	m_IsPlaying = false;
	m_StreamPositionRenderFrames = 0;
	m_StreamPositionOutputFrames = 0;

	m_RequestFlags.store(0);
}


Base::~Base()
{
	MPT_SOUNDDEV_TRACE_SCOPE();
	return;
}


SoundDevice::DynamicCaps Base::GetDeviceDynamicCaps(const std::vector<uint32> &baseSampleRates)
{
	MPT_SOUNDDEV_TRACE_SCOPE();
	SoundDevice::DynamicCaps result;
	result.supportedSampleRates = baseSampleRates;
	return result;
}


bool Base::Init(const SoundDevice::AppInfo &appInfo)
{
	MPT_SOUNDDEV_TRACE_SCOPE();
	if(IsInited())
	{
		return true;
	}
	m_AppInfo = appInfo;
	m_Caps = InternalGetDeviceCaps();
	return m_Caps.Available;
}


bool Base::Open(const SoundDevice::Settings &settings)
{
	MPT_SOUNDDEV_TRACE_SCOPE();
	if(IsOpen())
	{
		Close();
	}
	m_Settings = settings;
	if(m_Settings.Latency == 0.0) m_Settings.Latency = m_Caps.DefaultSettings.Latency;
	if(m_Settings.UpdateInterval == 0.0) m_Settings.UpdateInterval = m_Caps.DefaultSettings.UpdateInterval;
	m_Settings.Latency = std::clamp(m_Settings.Latency, m_Caps.LatencyMin, m_Caps.LatencyMax);
	m_Settings.UpdateInterval = std::clamp(m_Settings.UpdateInterval, m_Caps.UpdateIntervalMin, m_Caps.UpdateIntervalMax);
	m_Flags = SoundDevice::Flags();
	m_DeviceUnavailableOnOpen = false;
	m_RequestFlags.store(0);
	return InternalOpen();
}


bool Base::Close()
{
	MPT_SOUNDDEV_TRACE_SCOPE();
	if(!IsOpen())
	{
		return true;
	}
	Stop();
	bool result = InternalClose();
	m_RequestFlags.store(0);
	return result;
}


uint64 Base::CallbackGetReferenceClockNowNanoseconds() const
{
	MPT_SOUNDDEV_TRACE_SCOPE();
	if(!m_Callback)
	{
		return 0;
	}
	uint64 result = m_Callback->SoundCallbackGetReferenceClockNowNanoseconds();
	//MPT_LOG(GetLogger(), LogDebug, "sounddev", MPT_UFORMAT_MESSAGE("clock: {}")(result));
	return result;
}


uint64 Base::CallbackLockedGetReferenceClockNowNanoseconds() const
{
	MPT_SOUNDDEV_TRACE_SCOPE();
	if(!m_Callback)
	{
		return 0;
	}
	uint64 result = m_Callback->SoundCallbackLockedGetReferenceClockNowNanoseconds();
	//MPT_LOG(GetLogger(), LogDebug, "sounddev", MPT_UFORMAT_MESSAGE("clock-rt: {}")(result));
	return result;
}


void Base::CallbackNotifyPreStart()
{
	MPT_SOUNDDEV_TRACE_SCOPE();
	if(m_Callback)
	{
		m_Callback->SoundCallbackPreStart();
	}
}


void Base::CallbackNotifyPostStop()
{
	MPT_SOUNDDEV_TRACE_SCOPE();
	if(m_Callback)
	{
		m_Callback->SoundCallbackPostStop();
	}
}


bool Base::CallbackIsLockedByCurrentThread() const
{
	MPT_SOUNDDEV_TRACE_SCOPE();
	if(!m_Callback)
	{
		return false;
	}
	return m_Callback->SoundCallbackIsLockedByCurrentThread();
}


void Base::CallbackFillAudioBufferLocked()
{
	MPT_SOUNDDEV_TRACE_SCOPE();
	if(m_Callback)
	{
		CallbackLockedGuard lock(*m_Callback);
		InternalFillAudioBuffer();
	}
}


void Base::CallbackLockedAudioReadPrepare(std::size_t numFrames, std::size_t framesLatency)
{
	MPT_SOUNDDEV_TRACE_SCOPE();
	if(!InternalHasTimeInfo())
	{
		SoundDevice::TimeInfo timeInfo;
		if(InternalHasGetStreamPosition())
		{
			timeInfo.SyncPointStreamFrames = InternalHasGetStreamPosition();
			timeInfo.SyncPointSystemTimestamp = CallbackLockedGetReferenceClockNowNanoseconds();
			timeInfo.Speed = 1.0;
		} else
		{
			timeInfo.SyncPointStreamFrames = m_StreamPositionRenderFrames + numFrames;
			timeInfo.SyncPointSystemTimestamp = CallbackLockedGetReferenceClockNowNanoseconds() + mpt::saturate_round<int64>(GetEffectiveBufferAttributes().Latency * 1000000000.0);
			timeInfo.Speed = 1.0;
		}
		timeInfo.RenderStreamPositionBefore = StreamPositionFromFrames(m_StreamPositionRenderFrames);
		timeInfo.RenderStreamPositionAfter = StreamPositionFromFrames(m_StreamPositionRenderFrames + numFrames);
		timeInfo.Latency = GetEffectiveBufferAttributes().Latency;
		SetTimeInfo(timeInfo);
	}
	m_StreamPositionRenderFrames += numFrames;
	if(!InternalHasGetStreamPosition() && !InternalHasTimeInfo())
	{
		m_StreamPositionOutputFrames = m_StreamPositionRenderFrames - framesLatency;
	} else
	{
		// unused, no locking
		m_StreamPositionOutputFrames = 0;
	}
	if(m_Callback)
	{
		m_Callback->SoundCallbackLockedProcessPrepare(m_TimeInfo);
	}
}


template <typename Tsample>
void Base::CallbackLockedAudioProcessImpl(Tsample *buffer, const Tsample *inputBuffer, std::size_t numFrames)
{
	MPT_SOUNDDEV_TRACE_SCOPE();
	if(numFrames <= 0)
	{
		return;
	}
	if(m_Callback)
	{
		m_Callback->SoundCallbackLockedProcess(GetBufferFormat(), numFrames, buffer, inputBuffer);
	}
}

void Base::CallbackLockedAudioProcess(uint8 *buffer, const uint8 *inputBuffer, std::size_t numFrames)
{
	// cppcheck-suppress assertWithSideEffect
	assert(GetBufferFormat().sampleFormat == SampleFormat::Unsigned8);
	CallbackLockedAudioProcessImpl(buffer, inputBuffer, numFrames);
}

void Base::CallbackLockedAudioProcess(int8 *buffer, const int8 *inputBuffer, std::size_t numFrames)
{
	// cppcheck-suppress assertWithSideEffect
	assert(GetBufferFormat().sampleFormat == SampleFormat::Int8);
	CallbackLockedAudioProcessImpl(buffer, inputBuffer, numFrames);
}

void Base::CallbackLockedAudioProcess(int16 *buffer, const int16 *inputBuffer, std::size_t numFrames)
{
	// cppcheck-suppress assertWithSideEffect
	assert(GetBufferFormat().sampleFormat == SampleFormat::Int16);
	CallbackLockedAudioProcessImpl(buffer, inputBuffer, numFrames);
}

void Base::CallbackLockedAudioProcess(int24 *buffer, const int24 *inputBuffer, std::size_t numFrames)
{
	// cppcheck-suppress assertWithSideEffect
	assert(GetBufferFormat().sampleFormat == SampleFormat::Int24);
	CallbackLockedAudioProcessImpl(buffer, inputBuffer, numFrames);
}

void Base::CallbackLockedAudioProcess(int32 *buffer, const int32 *inputBuffer, std::size_t numFrames)
{
	// cppcheck-suppress assertWithSideEffect
	assert(GetBufferFormat().sampleFormat == SampleFormat::Int32);
	CallbackLockedAudioProcessImpl(buffer, inputBuffer, numFrames);
}

void Base::CallbackLockedAudioProcess(float *buffer, const float *inputBuffer, std::size_t numFrames)
{
	// cppcheck-suppress assertWithSideEffect
	assert(GetBufferFormat().sampleFormat == SampleFormat::Float32);
	CallbackLockedAudioProcessImpl(buffer, inputBuffer, numFrames);
}

void Base::CallbackLockedAudioProcess(double *buffer, const double *inputBuffer, std::size_t numFrames)
{
	// cppcheck-suppress assertWithSideEffect
	assert(GetBufferFormat().sampleFormat == SampleFormat::Float64);
	CallbackLockedAudioProcessImpl(buffer, inputBuffer, numFrames);
}

void Base::CallbackLockedAudioProcessVoid(void *buffer, const void *inputBuffer, std::size_t numFrames)
{
	switch(GetBufferFormat().sampleFormat)
	{
		case SampleFormat::Unsigned8:
			CallbackLockedAudioProcess(static_cast<uint8 *>(buffer), static_cast<const uint8 *>(inputBuffer), numFrames);
			break;
		case SampleFormat::Int8:
			CallbackLockedAudioProcess(static_cast<int8 *>(buffer), static_cast<const int8 *>(inputBuffer), numFrames);
			break;
		case SampleFormat::Int16:
			CallbackLockedAudioProcess(static_cast<int16 *>(buffer), static_cast<const int16 *>(inputBuffer), numFrames);
			break;
		case SampleFormat::Int24:
			CallbackLockedAudioProcess(static_cast<int24 *>(buffer), static_cast<const int24 *>(inputBuffer), numFrames);
			break;
		case SampleFormat::Int32:
			CallbackLockedAudioProcess(static_cast<int32 *>(buffer), static_cast<const int32 *>(inputBuffer), numFrames);
			break;
		case SampleFormat::Float32:
			CallbackLockedAudioProcess(static_cast<float *>(buffer), static_cast<const float *>(inputBuffer), numFrames);
			break;
		case SampleFormat::Float64:
			CallbackLockedAudioProcess(static_cast<double *>(buffer), static_cast<const double *>(inputBuffer), numFrames);
			break;
	}
}


void Base::CallbackLockedAudioProcessDone()
{
	MPT_SOUNDDEV_TRACE_SCOPE();
	if(m_Callback)
	{
		m_Callback->SoundCallbackLockedProcessDone(m_TimeInfo);
	}
}


void Base::SendDeviceMessage(LogLevel level, const mpt::ustring &str)
{
	MPT_SOUNDDEV_TRACE_SCOPE();
	MPT_LOG(GetLogger(), level, "sounddev", str);
	if(m_MessageReceiver)
	{
		m_MessageReceiver->SoundDeviceMessage(level, str);
	}
}


bool Base::Start()
{
	MPT_SOUNDDEV_TRACE_SCOPE();
	if(!IsOpen())
	{
		return false;
	}
	if(!IsPlaying())
	{
		m_StreamPositionRenderFrames = 0;
		{
			m_StreamPositionOutputFrames = 0;
		}
		CallbackNotifyPreStart();
		m_RequestFlags.fetch_and((~RequestFlagRestart).as_bits());
		if(!InternalStart())
		{
			CallbackNotifyPostStop();
			return false;
		}
		m_IsPlaying = true;
	}
	return true;
}


void Base::Stop()
{
	MPT_SOUNDDEV_TRACE_SCOPE();
	if(!IsOpen())
	{
		return;
	}
	if(IsPlaying())
	{
		InternalStop();
		m_RequestFlags.fetch_and((~RequestFlagRestart).as_bits());
		CallbackNotifyPostStop();
		m_IsPlaying = false;
		m_StreamPositionOutputFrames = 0;
		m_StreamPositionRenderFrames = 0;
	}
}


void Base::StopAndAvoidPlayingSilence()
{
	MPT_SOUNDDEV_TRACE_SCOPE();
	if(!IsOpen())
	{
		return;
	}
	if(!IsPlaying())
	{
		return;
	}
	InternalStopAndAvoidPlayingSilence();
	m_RequestFlags.fetch_and((~RequestFlagRestart).as_bits());
	CallbackNotifyPostStop();
	m_IsPlaying = false;
	m_StreamPositionOutputFrames = 0;
	m_StreamPositionRenderFrames = 0;
}


void Base::EndPlayingSilence()
{
	MPT_SOUNDDEV_TRACE_SCOPE();
	if(!IsOpen())
	{
		return;
	}
	if(IsPlaying())
	{
		return;
	}
	InternalEndPlayingSilence();
}


SoundDevice::StreamPosition Base::GetStreamPosition() const
{
	MPT_SOUNDDEV_TRACE_SCOPE();
	if(!IsOpen())
	{
		return StreamPosition();
	}
	int64 frames = 0;
	if(InternalHasGetStreamPosition())
	{
		frames = InternalGetStreamPositionFrames();
	} else if(InternalHasTimeInfo())
	{
		const uint64 now = CallbackGetReferenceClockNowNanoseconds();
		const SoundDevice::TimeInfo timeInfo = GetTimeInfo();
		frames = mpt::saturate_round<int64>(
			timeInfo.SyncPointStreamFrames + (static_cast<double>(static_cast<int64>(now - timeInfo.SyncPointSystemTimestamp)) * timeInfo.Speed * m_Settings.Samplerate * (1.0 / (1000.0 * 1000.0))));
	} else
	{
		frames = m_StreamPositionOutputFrames;
	}
	return StreamPositionFromFrames(frames);
}


SoundDevice::Statistics Base::GetStatistics() const
{
	MPT_SOUNDDEV_TRACE_SCOPE();
	SoundDevice::Statistics result;
	result.InstantaneousLatency = m_Settings.Latency;
	result.LastUpdateInterval = m_Settings.UpdateInterval;
	result.text = mpt::ustring();
	return result;
}


}  // namespace SoundDevice


OPENMPT_NAMESPACE_END
