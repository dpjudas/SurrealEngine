/* SPDX-License-Identifier: BSD-3-Clause */
/* SPDX-FileCopyrightText: Olivier Lapicque */
/* SPDX-FileCopyrightText: OpenMPT Project Developers and Contributors */


#pragma once

#include "openmpt/all/BuildSettings.hpp"

#include "openmpt/base/Types.hpp"
#include "openmpt/soundbase/SampleFormat.hpp"

#include <cstddef>


OPENMPT_NAMESPACE_BEGIN


namespace SoundDevice
{


struct StreamPosition
{
	int64 Frames = 0;      // relative to Start()
	double Seconds = 0.0;  // relative to Start()
};


struct TimeInfo
{

	int64 SyncPointStreamFrames = 0;
	uint64 SyncPointSystemTimestamp = 0;
	double Speed = 1.0;

	SoundDevice::StreamPosition RenderStreamPositionBefore;
	SoundDevice::StreamPosition RenderStreamPositionAfter;
	// int64 chunkSize = After - Before

	double Latency = 0.0;  // seconds
};


struct BufferFormat
{
	uint32 Samplerate;
	uint32 Channels;
	uint8 InputChannels;
	SampleFormat sampleFormat;
	bool WantsClippedOutput;
	int32 DitherType;
};


class ICallback
{
public:
	// main thread
	virtual uint64 SoundCallbackGetReferenceClockNowNanoseconds() const = 0;  // timeGetTime()*1000000 on Windows
	virtual void SoundCallbackPreStart() = 0;
	virtual void SoundCallbackPostStop() = 0;
	virtual bool SoundCallbackIsLockedByCurrentThread() const = 0;
	// audio thread
	virtual void SoundCallbackLock() = 0;
	virtual uint64 SoundCallbackLockedGetReferenceClockNowNanoseconds() const = 0;  // timeGetTime()*1000000 on Windows
	virtual void SoundCallbackLockedProcessPrepare(SoundDevice::TimeInfo timeInfo) = 0;
	virtual void SoundCallbackLockedProcess(SoundDevice::BufferFormat bufferFormat, std::size_t numFrames, uint8 *buffer, const uint8 *inputBuffer) = 0;
	virtual void SoundCallbackLockedProcess(SoundDevice::BufferFormat bufferFormat, std::size_t numFrames, int8 *buffer, const int8 *inputBuffer) = 0;
	virtual void SoundCallbackLockedProcess(SoundDevice::BufferFormat bufferFormat, std::size_t numFrames, int16 *buffer, const int16 *inputBuffer) = 0;
	virtual void SoundCallbackLockedProcess(SoundDevice::BufferFormat bufferFormat, std::size_t numFrames, int24 *buffer, const int24 *inputBuffer) = 0;
	virtual void SoundCallbackLockedProcess(SoundDevice::BufferFormat bufferFormat, std::size_t numFrames, int32 *buffer, const int32 *inputBuffer) = 0;
	virtual void SoundCallbackLockedProcess(SoundDevice::BufferFormat bufferFormat, std::size_t numFrames, float *buffer, const float *inputBuffer) = 0;
	virtual void SoundCallbackLockedProcess(SoundDevice::BufferFormat bufferFormat, std::size_t numFrames, double *buffer, const double *inputBuffer) = 0;
	virtual void SoundCallbackLockedProcessDone(SoundDevice::TimeInfo timeInfo) = 0;
	virtual void SoundCallbackUnlock() = 0;
};


}  // namespace SoundDevice


OPENMPT_NAMESPACE_END
