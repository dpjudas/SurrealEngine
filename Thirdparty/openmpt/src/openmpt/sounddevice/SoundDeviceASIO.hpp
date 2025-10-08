/* SPDX-License-Identifier: BSD-3-Clause */
/* SPDX-FileCopyrightText: Olivier Lapicque */
/* SPDX-FileCopyrightText: OpenMPT Project Developers and Contributors */


#pragma once

#include "openmpt/all/BuildSettings.hpp"

#ifdef MPT_WITH_ASIO

#include "SoundDevice.hpp"
#include "SoundDeviceBase.hpp"

#include "mpt/string/types.hpp"
#include "openmpt/base/Types.hpp"
#include "openmpt/logging/Logger.hpp"

#include <atomic>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include <cassert>

#include <ASIOModern/ASIO.hpp>
#include <ASIOModern/ASIOSystemWindows.hpp>

#if defined(MODPLUG_TRACKER)
#if !defined(MPT_BUILD_WINESUPPORT)
#include "../mptrack/ExceptionHandler.h"
#endif  // !MPT_BUILD_WINESUPPORT
#endif  // MODPLUG_TRACKER

#endif  // MPT_WITH_ASIO

OPENMPT_NAMESPACE_BEGIN

namespace SoundDevice
{

#ifdef MPT_WITH_ASIO


class ASIOException
	: public std::runtime_error
{
public:
	ASIOException(const std::string &msg)
		: std::runtime_error(msg)
	{
		return;
	}
};

class CASIODevice
	: public SoundDevice::Base
	, private ASIO::Driver::CallbackHandler
{

	friend class TemporaryASIODriverOpener;

protected:
	std::unique_ptr<ASIO::Windows::IBufferSwitchDispatcher> m_DeferredBufferSwitchDispatcher;
	std::unique_ptr<ASIO::Driver> m_Driver;

#if defined(MODPLUG_TRACKER) && !defined(MPT_BUILD_WINESUPPORT)
	using CrashContext = ExceptionHandler::Context;
	using CrashContextGuard = ExceptionHandler::ContextSetter;
#else   // !(MODPLUG_TRACKER && !MPT_BUILD_WINESUPPORT)
	struct CrashContext
	{
		void SetDescription(mpt::ustring)
		{
			return;
		}
	};
	struct CrashContextGuard
	{
		CrashContextGuard(CrashContext *)
		{
			return;
		}
	};
#endif  // MODPLUG_TRACKER && !MPT_BUILD_WINESUPPORT
	CrashContext m_Ectx;

	class ASIODriverWithContext
	{
	private:
		ASIO::Driver *m_Driver;
		CrashContextGuard m_Guard;

	public:
		ASIODriverWithContext(ASIO::Driver *driver, CrashContext *ectx)
			: m_Driver(driver)
			, m_Guard(ectx)
		{
			assert(driver);
			assert(ectx);
		}
		ASIODriverWithContext(const ASIODriverWithContext &) = delete;
		ASIODriverWithContext &operator=(const ASIODriverWithContext &) = delete;
		ASIO::Driver *operator->()
		{
			return m_Driver;
		}
	};

	ASIODriverWithContext AsioDriver()
	{
		assert(m_Driver);
		return ASIODriverWithContext{m_Driver.get(), &m_Ectx};
	}

	double m_BufferLatency;
	ASIO::Long m_nAsioBufferLen;
	std::vector<ASIO::BufferInfo> m_BufferInfo;
	bool m_BuffersCreated;
	std::vector<ASIO::ChannelInfo> m_ChannelInfo;
	std::vector<double> m_SampleBufferDouble;
	std::vector<float> m_SampleBufferFloat;
	std::vector<int16> m_SampleBufferInt16;
	std::vector<int24> m_SampleBufferInt24;
	std::vector<int32> m_SampleBufferInt32;
	std::vector<double> m_SampleInputBufferDouble;
	std::vector<float> m_SampleInputBufferFloat;
	std::vector<int16> m_SampleInputBufferInt16;
	std::vector<int24> m_SampleInputBufferInt24;
	std::vector<int32> m_SampleInputBufferInt32;
	bool m_CanOutputReady;

	bool m_DeviceRunning;
	uint64 m_TotalFramesWritten;
	bool m_DeferredProcessing;
	ASIO::BufferIndex m_BufferIndex;
	std::atomic<bool> m_RenderSilence;
	std::atomic<bool> m_RenderingSilence;

	int64 m_StreamPositionOffset;

	using AsioRequests = uint8;
	struct AsioRequest
	{
		enum AsioRequestEnum : AsioRequests
		{
			LatenciesChanged = 1 << 0,
		};
	};
	std::atomic<AsioRequests> m_AsioRequest;

	using AsioFeatures = uint16;
	struct AsioFeature
	{
		enum AsioFeatureEnum : AsioFeatures
		{
			ResetRequest = 1 << 0,
			ResyncRequest = 1 << 1,
			BufferSizeChange = 1 << 2,
			Overload = 1 << 3,
			SampleRateChange = 1 << 4,
			DeferredProcess = 1 << 5,
		};
	};
	mutable std::atomic<AsioFeatures> m_UsedFeatures;
	static mpt::ustring AsioFeaturesToString(AsioFeatures features);

	mutable std::atomic<uint32> m_DebugRealtimeThreadID;

	void SetRenderSilence(bool silence, bool wait = false);

public:
	CASIODevice(ILogger &logger, SoundDevice::Info info, SoundDevice::SysInfo sysInfo);
	~CASIODevice();

private:
	void InitMembers();
	bool HandleRequests();  // return true if any work has been done
	void UpdateLatency();

	void InternalStopImpl(bool force);

public:
	bool InternalOpen();
	bool InternalClose();
	void InternalFillAudioBuffer();
	bool InternalStart();
	void InternalStop();
	bool InternalIsOpen() const
	{
		return m_BuffersCreated;
	}

	bool InternalIsPlayingSilence() const;
	void InternalStopAndAvoidPlayingSilence();
	void InternalEndPlayingSilence();

	bool OnIdle()
	{
		return HandleRequests();
	}

	SoundDevice::Caps InternalGetDeviceCaps();
	SoundDevice::DynamicCaps GetDeviceDynamicCaps(const std::vector<uint32> &baseSampleRates);

	bool OpenDriverSettings();

	bool DebugIsFragileDevice() const;
	bool DebugInRealtimeCallback() const;

	SoundDevice::Statistics GetStatistics() const;

public:
	static std::unique_ptr<SoundDevice::BackendInitializer> BackendInitializer()
	{
		return std::make_unique<SoundDevice::BackendInitializer>();
	}
	static std::vector<SoundDevice::Info> EnumerateDevices(ILogger &logger, SoundDevice::SysInfo sysInfo);

protected:
	void OpenDriver();
	void CloseDriver();
	bool IsDriverOpen() const
	{
		return (m_Driver != nullptr);
	}

	bool InternalHasTimeInfo() const;

	SoundDevice::BufferAttributes InternalGetEffectiveBufferAttributes() const;

protected:
	void FillAsioBuffer(bool useSource = true);

private:
	// CallbackHandler

	void MessageResetRequest() noexcept override;
	bool MessageBufferSizeChange(ASIO::Long newSize) noexcept override;
	bool MessageResyncRequest() noexcept override;
	void MessageLatenciesChanged() noexcept override;
	ASIO::Long MessageMMCCommand(ASIO::Long value, const void *message, const ASIO::Double *opt) noexcept override;
	void MessageOverload() noexcept override;

	ASIO::Long MessageUnknown(ASIO::MessageSelector selector, ASIO::Long value, const void *message, const ASIO::Double *opt) noexcept override;

	void RealtimeSampleRateDidChange(ASIO::SampleRate sRate) noexcept override;
	void RealtimeRequestDeferredProcessing(bool value) noexcept override;
	void RealtimeTimeInfo(ASIO::Time time) noexcept override;
	void RealtimeBufferSwitch(ASIO::BufferIndex bufferIndex) noexcept override;

	void RealtimeBufferSwitchImpl(ASIO::BufferIndex bufferIndex) noexcept;

private:
	void ExceptionHandler(const char *func);
};

#endif  // MPT_WITH_ASIO

}  // namespace SoundDevice

OPENMPT_NAMESPACE_END
