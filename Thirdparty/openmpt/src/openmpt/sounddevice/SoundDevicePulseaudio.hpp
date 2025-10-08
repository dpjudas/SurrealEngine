/* SPDX-License-Identifier: BSD-3-Clause */
/* SPDX-FileCopyrightText: OpenMPT Project Developers and Contributors */


#pragma once

#include "openmpt/all/BuildSettings.hpp"

#include "SoundDevice.hpp"
#include "SoundDeviceUtilities.hpp"

#include "mpt/string/types.hpp"
#include "openmpt/base/Types.hpp"
#include "openmpt/logging/Logger.hpp"

#include <memory>
#include <vector>

#if defined(MPT_ENABLE_PULSEAUDIO_FULL)
#if defined(MPT_WITH_PULSEAUDIO)
#include <pulse/pulseaudio.h>
#include <pulse/simple.h>
#endif  // MPT_WITH_PULSEAUDIO
#endif  // MPT_ENABLE_PULSEAUDIO_FULL


OPENMPT_NAMESPACE_BEGIN


namespace SoundDevice
{


#if defined(MPT_ENABLE_PULSEAUDIO_FULL)

#if defined(MPT_WITH_PULSEAUDIO)


class Pulseaudio
	: public ThreadBase
{
private:
	static mpt::ustring PulseErrorString(int error);

public:
	static std::unique_ptr<SoundDevice::BackendInitializer> BackendInitializer() { return std::make_unique<SoundDevice::BackendInitializer>(); }
	static std::vector<SoundDevice::Info> EnumerateDevices(ILogger &logger, SoundDevice::SysInfo sysInfo);

public:
	Pulseaudio(ILogger &logger, SoundDevice::Info info, SoundDevice::SysInfo sysInfo);
	SoundDevice::Caps InternalGetDeviceCaps();
	SoundDevice::DynamicCaps GetDeviceDynamicCaps(const std::vector<uint32> &baseSampleRates);
	bool InternalIsOpen() const;
	bool InternalOpen();
	void InternalStartFromSoundThread();
	void InternalFillAudioBuffer();
	void InternalWaitFromSoundThread();
	SoundDevice::BufferAttributes InternalGetEffectiveBufferAttributes() const;
	SoundDevice::Statistics GetStatistics() const;
	void InternalStopFromSoundThread();
	bool InternalClose();
	~Pulseaudio();

private:
	pa_simple *m_PA_SimpleOutput;
	SoundDevice::BufferAttributes m_EffectiveBufferAttributes;
	std::vector<somefloat32> m_OutputBuffer;
	std::atomic<uint32> m_StatisticLastLatencyFrames;
};


#endif  // MPT_WITH_PULSEAUDIO

#endif  // MPT_ENABLE_PULSEAUDIO_FULL


}  // namespace SoundDevice


OPENMPT_NAMESPACE_END
