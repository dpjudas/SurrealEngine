/* SPDX-License-Identifier: BSD-3-Clause */
/* SPDX-FileCopyrightText: OpenMPT Project Developers and Contributors */


#pragma once

#include "openmpt/all/BuildSettings.hpp"

#include "SoundDeviceBase.hpp"

#include "openmpt/base/Types.hpp"
#include "openmpt/logging/Logger.hpp"

#include <atomic>
#include <memory>
#include <vector>

#ifdef MPT_WITH_RTAUDIO
#if MPT_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable : 4244)  // conversion from 'int' to 'unsigned char', possible loss of data
#endif
#if MPT_COMPILER_GCC
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-copy"
#endif
#include <RtAudio.h>
#if MPT_COMPILER_GCC
#pragma GCC diagnostic pop
#endif
#if MPT_COMPILER_MSVC
#pragma warning(pop)
#endif
#if defined(RTAUDIO_VERSION_MAJOR)
#if(RTAUDIO_VERSION_MAJOR >= 6)
#define MPT_RTAUDIO_VER 6
#endif
#endif
#ifndef MPT_RTAUDIO_VER
#define MPT_RTAUDIO_VER 5
#endif
#define MPT_RTAUDIO_AT_LEAST(v) (MPT_RTAUDIO_VER >= (v))
#define MPT_RTAUDIO_BEFORE(v)   (MPT_RTAUDIO_VER < (v))
#endif  // MPT_WITH_RTAUDIO

OPENMPT_NAMESPACE_BEGIN

namespace SoundDevice
{


#ifdef MPT_WITH_RTAUDIO


class CRtAudioDevice : public SoundDevice::Base
{

protected:
	std::unique_ptr<RtAudio> m_RtAudio;

	RtAudio::StreamParameters m_InputStreamParameters;
	RtAudio::StreamParameters m_OutputStreamParameters;
	unsigned int m_FramesPerChunk;
	RtAudio::StreamOptions m_StreamOptions;

	void *m_CurrentFrameBufferOutput;
	void *m_CurrentFrameBufferInput;
	unsigned int m_CurrentFrameBufferCount;
	double m_CurrentStreamTime;

	std::atomic<uint32> m_StatisticLatencyFrames;
	std::atomic<uint32> m_StatisticPeriodFrames;

public:
	CRtAudioDevice(ILogger &logger, SoundDevice::Info info, SoundDevice::SysInfo sysInfo);
	~CRtAudioDevice();

public:
	bool InternalOpen();
	bool InternalClose();
	void InternalFillAudioBuffer();
	bool InternalStart();
	void InternalStop();
	bool InternalIsOpen() const { return m_RtAudio && m_RtAudio->isStreamOpen(); }
	bool InternalHasGetStreamPosition() const { return true; }
	int64 InternalGetStreamPositionFrames() const;
	SoundDevice::BufferAttributes InternalGetEffectiveBufferAttributes() const;
	SoundDevice::Statistics GetStatistics() const;
	SoundDevice::Caps InternalGetDeviceCaps();
	SoundDevice::DynamicCaps GetDeviceDynamicCaps(const std::vector<uint32> &baseSampleRates);

private:
#if MPT_RTAUDIO_AT_LEAST(6)
	static bool IsError(const RtAudioErrorType &e);
#endif
#if MPT_RTAUDIO_AT_LEAST(6)
	void SendError(const RtAudioErrorType &e, const std::string &errorText);
#else
	void SendError(const RtAudioError &e);
#endif

	void AudioCallback(void *outputBuffer, void *inputBuffer, unsigned int nFrames, double streamTime, RtAudioStreamStatus status);

	static int RtAudioCallback(void *outputBuffer, void *inputBuffer, unsigned int nFrames, double streamTime, RtAudioStreamStatus status, void *userData);

	static RtAudio::Api GetApi(SoundDevice::Info info);
	static unsigned int GetDevice(SoundDevice::Info info);

public:
	static std::unique_ptr<SoundDevice::BackendInitializer> BackendInitializer()
	{
		return std::make_unique<SoundDevice::BackendInitializer>();
	}
	static std::vector<SoundDevice::Info> EnumerateDevices(ILogger &logger, SoundDevice::SysInfo sysInfo);
};


#endif  // MPT_WITH_RTAUDIO


}  // namespace SoundDevice


OPENMPT_NAMESPACE_END
