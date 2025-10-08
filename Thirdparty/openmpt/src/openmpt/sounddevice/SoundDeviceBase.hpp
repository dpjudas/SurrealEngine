/* SPDX-License-Identifier: BSD-3-Clause */
/* SPDX-FileCopyrightText: Olivier Lapicque */
/* SPDX-FileCopyrightText: OpenMPT Project Developers and Contributors */


#pragma once

#include "openmpt/all/BuildSettings.hpp"

#include "SoundDevice.hpp"
#include "SoundDeviceCallback.hpp"

#include "mpt/mutex/mutex.hpp"
#include "mpt/string/types.hpp"
#include "openmpt/base/Types.hpp"
#include "openmpt/logging/Logger.hpp"
#include "openmpt/soundbase/SampleFormat.hpp"

#include <atomic>
#include <vector>

#include <cstddef>


OPENMPT_NAMESPACE_BEGIN


namespace SoundDevice
{


class Base
	: public IBase
{

private:
	class CallbackLockedGuard
	{
	private:
		ICallback &m_Callback;

	public:
		CallbackLockedGuard(ICallback &callback)
			: m_Callback(callback)
		{
			m_Callback.SoundCallbackLock();
		}
		~CallbackLockedGuard()
		{
			m_Callback.SoundCallbackUnlock();
		}
	};

protected:
	ILogger &m_Logger;

private:
	SoundDevice::ICallback *m_Callback;
	SoundDevice::IMessageReceiver *m_MessageReceiver;

	const SoundDevice::Info m_Info;

private:
	SoundDevice::Caps m_Caps;

protected:
	SoundDevice::SysInfo m_SysInfo;
	SoundDevice::AppInfo m_AppInfo;
	SoundDevice::Settings m_Settings;
	SoundDevice::Flags m_Flags;
	bool m_DeviceUnavailableOnOpen;

private:
	bool m_IsPlaying;

	SoundDevice::TimeInfo m_TimeInfo;

	int64 m_StreamPositionRenderFrames;  // only updated or read in audio CALLBACK or when device is stopped. requires no further locking

	std::atomic<int64> m_StreamPositionOutputFrames;

	std::atomic<uint32> m_RequestFlags;

public:
	ILogger &GetLogger() const { return m_Logger; }
	SoundDevice::SysInfo GetSysInfo() const { return m_SysInfo; }
	SoundDevice::AppInfo GetAppInfo() const { return m_AppInfo; }

protected:
	SoundDevice::Type GetDeviceType() const { return m_Info.type; }
	mpt::ustring GetDeviceInternalID() const { return m_Info.internalID; }
	SoundDevice::Identifier GetDeviceIdentifier() const { return m_Info.GetIdentifier(); }

	virtual void InternalFillAudioBuffer() = 0;

	uint64 CallbackGetReferenceClockNowNanoseconds() const;
	void CallbackNotifyPreStart();
	void CallbackNotifyPostStop();
	bool CallbackIsLockedByCurrentThread() const;
	void CallbackFillAudioBufferLocked();
	uint64 CallbackLockedGetReferenceClockNowNanoseconds() const;
	void CallbackLockedAudioReadPrepare(std::size_t numFrames, std::size_t framesLatency);
	template <typename Tsample>
	void CallbackLockedAudioProcessImpl(Tsample *buffer, const Tsample *inputBuffer, std::size_t numFrames);
	void CallbackLockedAudioProcess(uint8 *buffer, const uint8 *inputBuffer, std::size_t numFrames);
	void CallbackLockedAudioProcess(int8 *buffer, const int8 *inputBuffer, std::size_t numFrames);
	void CallbackLockedAudioProcess(int16 *buffer, const int16 *inputBuffer, std::size_t numFrames);
	void CallbackLockedAudioProcess(int24 *buffer, const int24 *inputBuffer, std::size_t numFrames);
	void CallbackLockedAudioProcess(int32 *buffer, const int32 *inputBuffer, std::size_t numFrames);
	void CallbackLockedAudioProcess(float *buffer, const float *inputBuffer, std::size_t numFrames);
	void CallbackLockedAudioProcess(double *buffer, const double *inputBuffer, std::size_t numFrames);
	void CallbackLockedAudioProcessVoid(void *buffer, const void *inputBuffer, std::size_t numFrames);
	void CallbackLockedAudioProcessDone();

	void RequestClose() { m_RequestFlags.fetch_or(RequestFlagClose); }
	void RequestReset() { m_RequestFlags.fetch_or(RequestFlagReset); }
	void RequestRestart() { m_RequestFlags.fetch_or(RequestFlagRestart); }

	void SendDeviceMessage(LogLevel level, const mpt::ustring &str);

protected:
	void SetTimeInfo(SoundDevice::TimeInfo timeInfo) { m_TimeInfo = timeInfo; }

	SoundDevice::StreamPosition StreamPositionFromFrames(int64 frames) const { return SoundDevice::StreamPosition{frames, static_cast<double>(frames) / static_cast<double>(m_Settings.Samplerate)}; }

	virtual bool InternalHasTimeInfo() const { return false; }

	virtual bool InternalHasGetStreamPosition() const { return false; }
	virtual int64 InternalGetStreamPositionFrames() const { return 0; }

	virtual bool InternalIsOpen() const = 0;

	virtual bool InternalOpen() = 0;
	virtual bool InternalStart() = 0;
	virtual void InternalStop() = 0;
	virtual bool InternalClose() = 0;

	virtual bool InternalIsPlayingSilence() const { return false; }
	virtual void InternalStopAndAvoidPlayingSilence() { InternalStop(); }
	virtual void InternalEndPlayingSilence() { return; }

	virtual SoundDevice::Caps InternalGetDeviceCaps() = 0;

	virtual SoundDevice::BufferAttributes InternalGetEffectiveBufferAttributes() const = 0;

protected:
	Base(ILogger &logger, SoundDevice::Info info, SoundDevice::SysInfo sysInfo);

public:
	virtual ~Base();

	void SetCallback(SoundDevice::ICallback *callback) { m_Callback = callback; }
	void SetMessageReceiver(SoundDevice::IMessageReceiver *receiver) { m_MessageReceiver = receiver; }

	SoundDevice::Info GetDeviceInfo() const { return m_Info; }

	SoundDevice::Caps GetDeviceCaps() const { return m_Caps; }
	virtual SoundDevice::DynamicCaps GetDeviceDynamicCaps(const std::vector<uint32> &baseSampleRates);

	bool Init(const SoundDevice::AppInfo &appInfo);
	bool Open(const SoundDevice::Settings &settings);
	bool Close();
	bool Start();
	void Stop();

	FlagSet<RequestFlags> GetRequestFlags() const { return FlagSet<RequestFlags>(m_RequestFlags.load()); }

	bool IsInited() const { return m_Caps.Available; }
	bool IsOpen() const { return IsInited() && InternalIsOpen(); }
	bool IsAvailable() const { return m_Caps.Available && !m_DeviceUnavailableOnOpen; }
	bool IsPlaying() const { return m_IsPlaying; }

	virtual bool IsPlayingSilence() const { return IsOpen() && !IsPlaying() && InternalIsPlayingSilence(); }
	virtual void StopAndAvoidPlayingSilence();
	virtual void EndPlayingSilence();

	virtual bool OnIdle() { return false; }

	SoundDevice::Settings GetSettings() const { return m_Settings; }
	SampleFormat GetActualSampleFormat() const { return IsOpen() ? m_Settings.sampleFormat : SampleFormat(SampleFormat::Default); }
	SoundDevice::BufferFormat GetBufferFormat() const
	{
		BufferFormat bufferFormat;
		bufferFormat.Samplerate = m_Settings.Samplerate;
		bufferFormat.Channels = m_Settings.Channels;
		bufferFormat.InputChannels = m_Settings.InputChannels;
		bufferFormat.sampleFormat = m_Settings.sampleFormat;
		bufferFormat.WantsClippedOutput = m_Flags.WantsClippedOutput;
		bufferFormat.DitherType = m_Settings.DitherType;
		return bufferFormat;
	}
	SoundDevice::BufferAttributes GetEffectiveBufferAttributes() const { return (IsOpen() && IsPlaying()) ? InternalGetEffectiveBufferAttributes() : SoundDevice::BufferAttributes(); }

	SoundDevice::TimeInfo GetTimeInfo() const { return m_TimeInfo; }
	SoundDevice::StreamPosition GetStreamPosition() const;

	virtual bool DebugIsFragileDevice() const { return false; }
	virtual bool DebugInRealtimeCallback() const { return false; }

	virtual SoundDevice::Statistics GetStatistics() const;

	virtual bool OpenDriverSettings() { return false; };
};


}  // namespace SoundDevice


OPENMPT_NAMESPACE_END
