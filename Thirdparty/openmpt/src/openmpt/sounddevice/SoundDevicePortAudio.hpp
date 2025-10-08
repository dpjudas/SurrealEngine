/* SPDX-License-Identifier: BSD-3-Clause */
/* SPDX-FileCopyrightText: OpenMPT Project Developers and Contributors */


#pragma once

#include "openmpt/all/BuildSettings.hpp"

#include "SoundDevice.hpp"
#include "SoundDeviceBase.hpp"

#include "mpt/base/detect.hpp"
#include "mpt/string/types.hpp"
#include "openmpt/base/Types.hpp"
#include "openmpt/logging/Logger.hpp"

#include <atomic>
#include <memory>
#include <utility>
#include <vector>

#ifdef MPT_WITH_PORTAUDIO
#include <portaudio.h>
#if MPT_OS_WINDOWS
#include <pa_win_wasapi.h>
#endif  // MPT_OS_WINDOWS
#endif

OPENMPT_NAMESPACE_BEGIN

namespace SoundDevice
{

#ifdef MPT_WITH_PORTAUDIO

class PortAudioInitializer
	: public BackendInitializer
{
private:
	bool m_initialized = false;

public:
	PortAudioInitializer();
	PortAudioInitializer(const PortAudioInitializer &) = delete;
	PortAudioInitializer &operator=(const PortAudioInitializer &) = delete;
	void Reload() override;
	~PortAudioInitializer() override;
};

class CPortaudioDevice : public SoundDevice::Base
{

private:
	PortAudioInitializer m_PortAudio;

protected:
	PaDeviceIndex m_DeviceIsDefault;
	PaDeviceIndex m_DeviceIndex;
	PaHostApiTypeId m_HostApiType;
	PaStreamParameters m_StreamParameters;
	PaStreamParameters m_InputStreamParameters;
#if MPT_OS_WINDOWS
	PaWasapiStreamInfo m_WasapiStreamInfo;
#endif  // MPT_OS_WINDOWS
	PaStream *m_Stream;
	const PaStreamInfo *m_StreamInfo;
	void *m_CurrentFrameBuffer;
	const void *m_CurrentFrameBufferInput;
	unsigned long m_CurrentFrameCount;

	double m_CurrentRealLatency;  // seconds
	std::atomic<uint32> m_StatisticPeriodFrames;

public:
	CPortaudioDevice(ILogger &logger, SoundDevice::Info info, SoundDevice::SysInfo sysInfo);
	~CPortaudioDevice();

public:
	bool InternalOpen();
	bool InternalClose();
	void InternalFillAudioBuffer();
	bool InternalStart();
	void InternalStop();
	bool InternalIsOpen() const
	{
		return m_Stream ? true : false;
	}
	bool InternalHasGetStreamPosition() const
	{
		return false;
	}
	int64 InternalGetStreamPositionFrames() const;
	SoundDevice::BufferAttributes InternalGetEffectiveBufferAttributes() const;
	SoundDevice::Statistics GetStatistics() const;
	SoundDevice::Caps InternalGetDeviceCaps();
	SoundDevice::DynamicCaps GetDeviceDynamicCaps(const std::vector<uint32> &baseSampleRates);
	bool OpenDriverSettings();
	bool OnIdle();

	int StreamCallback(
		const void *input, void *output, unsigned long frameCount, const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags);

public:
	static int StreamCallbackWrapper(
		const void *input, void *output, unsigned long frameCount, const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags, void *userData);

	static std::unique_ptr<SoundDevice::BackendInitializer> BackendInitializer()
	{
		return std::make_unique<PortAudioInitializer>();
	}

	static std::vector<SoundDevice::Info> EnumerateDevices(ILogger &logger, SoundDevice::SysInfo sysInfo);

private:
	bool HasInputChannelsOnSameDevice() const;

	static std::vector<std::pair<PaDeviceIndex, mpt::ustring>> EnumerateInputOnlyDevices(PaHostApiTypeId hostApiType);
};


#endif  // MPT_WITH_PORTAUDIO


}  // namespace SoundDevice


OPENMPT_NAMESPACE_END
