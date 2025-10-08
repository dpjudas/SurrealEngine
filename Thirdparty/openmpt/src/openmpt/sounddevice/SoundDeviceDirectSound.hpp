/* SPDX-License-Identifier: BSD-3-Clause */
/* SPDX-FileCopyrightText: Olivier Lapicque */
/* SPDX-FileCopyrightText: OpenMPT Project Developers and Contributors */


#pragma once

#include "openmpt/all/BuildSettings.hpp"

#include "SoundDevice.hpp"
#include "SoundDeviceUtilities.hpp"

#include "openmpt/base/Types.hpp"
#include "openmpt/logging/Logger.hpp"

#include <atomic>
#include <memory>
#include <vector>

#if defined(MPT_WITH_DIRECTSOUND)
#include <dsound.h>
#endif  // MPT_WITH_DIRECTSOUND

OPENMPT_NAMESPACE_BEGIN

namespace SoundDevice
{

#if defined(MPT_WITH_DIRECTSOUND)


class CDSoundDevice : public CSoundDeviceWithThread
{
protected:
	IDirectSound *m_piDS;
	IDirectSoundBuffer *m_pPrimary;
	IDirectSoundBuffer *m_pMixBuffer;
	uint32 m_nDSoundBufferSize;
	BOOL m_bMixRunning;
	DWORD m_dwWritePos;

	std::atomic<uint32> m_StatisticLatencyFrames;
	std::atomic<uint32> m_StatisticPeriodFrames;

public:
	CDSoundDevice(ILogger &logger, SoundDevice::Info info, SoundDevice::SysInfo sysInfo);
	~CDSoundDevice();

public:
	bool InternalOpen();
	bool InternalClose();
	void InternalFillAudioBuffer();
	void StartFromSoundThread();
	void StopFromSoundThread();
	bool InternalIsOpen() const { return (m_pMixBuffer != NULL); }
	SoundDevice::BufferAttributes InternalGetEffectiveBufferAttributes() const;
	SoundDevice::Statistics GetStatistics() const;
	SoundDevice::Caps InternalGetDeviceCaps();
	SoundDevice::DynamicCaps GetDeviceDynamicCaps(const std::vector<uint32> &baseSampleRates);

public:
	static std::unique_ptr<SoundDevice::BackendInitializer> BackendInitializer() { return std::make_unique<SoundDevice::BackendInitializer>(); }
	static std::vector<SoundDevice::Info> EnumerateDevices(ILogger &logger, SoundDevice::SysInfo sysInfo);
};

#endif  // MPT_WITH_DIRECTSOUND


}  // namespace SoundDevice


OPENMPT_NAMESPACE_END
