/* SPDX-License-Identifier: BSD-3-Clause */
/* SPDX-FileCopyrightText: Olivier Lapicque */
/* SPDX-FileCopyrightText: OpenMPT Project Developers and Contributors */


#include "openmpt/all/BuildSettings.hpp"
#include "openmpt/all/PlatformFixes.hpp"

#ifdef MPT_WITH_ASIO

#include "SoundDeviceASIO.hpp"

#include "SoundDevice.hpp"
#include "SoundDeviceBase.hpp"
#include "SoundDeviceCallback.hpp"

#include "mpt/base/bit.hpp"
#include "mpt/base/detect.hpp"
#include "mpt/base/macros.hpp"
#include "mpt/base/saturate_round.hpp"
#include "mpt/base/utility.hpp"
#include "mpt/exception/exception_text.hpp"
#include "mpt/format/message_macros.hpp"
#include "mpt/format/simple.hpp"
#include "mpt/out_of_memory/out_of_memory.hpp"
#include "mpt/string/types.hpp"
#include "mpt/string/utility.hpp"
#include "mpt/string_transcode/transcode.hpp"
#include "mpt/uuid/guid.hpp"
#include "mpt/uuid/uuid.hpp"
#include "openmpt/base/Types.hpp"
#include "openmpt/logging/Logger.hpp"
#include "openmpt/soundbase/SampleFormat.hpp"

#if defined(MODPLUG_TRACKER)
#include "mptAssert.h"
#if !defined(MPT_BUILD_WINESUPPORT)
#include "../mptrack/ExceptionHandler.h"
#endif  // !MPT_BUILD_WINESUPPORT
#endif  // MODPLUG_TRACKER

#include <algorithm>
#include <atomic>
#include <chrono>
#include <exception>
#include <memory>
#include <new>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include <cassert>
#include <cstddef>
#include <cstdint>

#include <ASIOModern/ASIO.hpp>
#include <ASIOModern/ASIOSystemWindows.hpp>
#if defined(MODPLUG_TRACKER)
#include <ASIOModern/ASIOSystemWindowsSEH.hpp>
#endif  // MODPLUG_TRACKER
#include <ASIOModern/ASIOSampleConvert.hpp>
//#include <ASIOModern/ASIOVerifyABI.hpp>

#endif  // MPT_WITH_ASIO


OPENMPT_NAMESPACE_BEGIN


namespace SoundDevice
{


#ifdef MPT_WITH_ASIO


static constexpr uint64 AppID1 = 0x4f70656e4d50542dull;  // "OpenMPT-"
static constexpr uint64 AppID2 = 0x4153494f00000000ull;  // "ASIO"

static constexpr double AsioSampleRateTolerance = 0.05;


static constexpr inline auto value_cast(ASIO::Bool b) noexcept -> bool
{
	return static_cast<bool>(b);
}


// Helper class to temporarily open a driver for a query.
class TemporaryASIODriverOpener
{
protected:
	CASIODevice &device;
	const bool wasOpen;

public:
	TemporaryASIODriverOpener(CASIODevice &d)
		: device(d)
		, wasOpen(d.IsDriverOpen())
	{
		if(!wasOpen)
		{
			device.OpenDriver();
		}
	}

	~TemporaryASIODriverOpener()
	{
		if(!wasOpen)
		{
			device.CloseDriver();
		}
	}
};


static mpt::winstring AsWinstring(const std::basic_string<TCHAR> &str)
{
	return mpt::winstring(str.data(), str.length());
}


std::vector<SoundDevice::Info> CASIODevice::EnumerateDevices(ILogger &logger, SoundDevice::SysInfo sysInfo)
{
	MPT_SOUNDDEV_TRACE_SCOPE();
	auto GetLogger = [&]() -> ILogger &
	{
		return logger;
	};
	std::vector<SoundDevice::Info> devices;
	std::vector<ASIO::Windows::DriverInfo> drivers = ASIO::Windows::EnumerateDrivers();
	for(const auto &driver : drivers)
	{
		SoundDevice::Info info;
		info.type = TypeASIO;
		info.internalID = mpt::transcode<mpt::ustring>(mpt::CLSIDToString(driver.Clsid));
		info.apiName = MPT_USTRING("ASIO");
		info.name = mpt::transcode<mpt::ustring>(AsWinstring(driver.DisplayName()));
		info.useNameAsIdentifier = false;
		info.default_ = Info::Default::None;
		// clang-format off
		info.flags = {
			sysInfo.SystemClass == mpt::osinfo::osclass::Windows ? sysInfo.IsWindowsOriginal() ? Info::Usability::Usable : Info::Usability::Experimental : Info::Usability::NotAvailable,
			Info::Level::Primary,
			Info::Compatible::No,
			sysInfo.SystemClass == mpt::osinfo::osclass::Windows && sysInfo.IsWindowsOriginal() ? Info::Api::Native : Info::Api::Emulated,
			Info::Io::FullDuplex,
			Info::Mixing::Hardware,
			Info::Implementor::OpenMPT
		};
		// clang-format on
		info.extraData[MPT_USTRING("Key")] = mpt::transcode<mpt::ustring>(AsWinstring(driver.Key));
		;
		info.extraData[MPT_USTRING("Id")] = mpt::transcode<mpt::ustring>(AsWinstring(driver.Id));
		info.extraData[MPT_USTRING("CLSID")] = mpt::transcode<mpt::ustring>(mpt::CLSIDToString(driver.Clsid));
		info.extraData[MPT_USTRING("Name")] = mpt::transcode<mpt::ustring>(AsWinstring(driver.Name));
		;
		info.extraData[MPT_USTRING("Description")] = mpt::transcode<mpt::ustring>(AsWinstring(driver.Description));
		;
		info.extraData[MPT_USTRING("DisplayName")] = mpt::transcode<mpt::ustring>(AsWinstring(driver.DisplayName()));
		;
		MPT_LOG(GetLogger(), LogDebug, "sounddev", MPT_UFORMAT_MESSAGE("ASIO: Found driver:")());
		MPT_LOG(GetLogger(), LogDebug, "sounddev", MPT_UFORMAT_MESSAGE("ASIO:  Key         = '{}'")(info.extraData[MPT_USTRING("Key")]));
		MPT_LOG(GetLogger(), LogDebug, "sounddev", MPT_UFORMAT_MESSAGE("ASIO:  Id          = '{}'")(info.extraData[MPT_USTRING("Id")]));
		MPT_LOG(GetLogger(), LogDebug, "sounddev", MPT_UFORMAT_MESSAGE("ASIO:  CLSID       = '{}'")(info.extraData[MPT_USTRING("CLSID")]));
		MPT_LOG(GetLogger(), LogDebug, "sounddev", MPT_UFORMAT_MESSAGE("ASIO:  Name        = '{}'")(info.extraData[MPT_USTRING("Name")]));
		MPT_LOG(GetLogger(), LogDebug, "sounddev", MPT_UFORMAT_MESSAGE("ASIO:  Description = '{}'")(info.extraData[MPT_USTRING("Description")]));
		MPT_LOG(GetLogger(), LogDebug, "sounddev", MPT_UFORMAT_MESSAGE("ASIO:  DisplayName = '{}'")(info.extraData[MPT_USTRING("DisplayName")]));
		devices.push_back(info);
	}
	return devices;
}


CASIODevice::CASIODevice(ILogger &logger, SoundDevice::Info info, SoundDevice::SysInfo sysInfo)
	: SoundDevice::Base(logger, info, sysInfo)
	, m_RenderSilence(false)
	, m_RenderingSilence(false)
	, m_AsioRequest(0)
	, m_UsedFeatures(0)
	, m_DebugRealtimeThreadID(0)
{
	MPT_SOUNDDEV_TRACE_SCOPE();
	m_Ectx.SetDescription(MPT_UFORMAT_MESSAGE("ASIO Driver: {}")(GetDeviceInternalID()));
	InitMembers();
}


void CASIODevice::InitMembers()
{
	MPT_SOUNDDEV_TRACE_SCOPE();
	m_DeferredBufferSwitchDispatcher = nullptr;
	m_Driver = nullptr;

	m_BufferLatency = 0.0;
	m_nAsioBufferLen = 0;
	m_BufferInfo.clear();
	m_BuffersCreated = false;
	m_ChannelInfo.clear();
	m_SampleBufferDouble.clear();
	m_SampleBufferFloat.clear();
	m_SampleBufferInt16.clear();
	m_SampleBufferInt24.clear();
	m_SampleBufferInt32.clear();
	m_SampleInputBufferDouble.clear();
	m_SampleInputBufferFloat.clear();
	m_SampleInputBufferInt16.clear();
	m_SampleInputBufferInt24.clear();
	m_SampleInputBufferInt32.clear();
	m_CanOutputReady = false;

	m_DeviceRunning = false;
	m_TotalFramesWritten = 0;
	m_DeferredProcessing = false;
	m_BufferIndex = 0;
	m_RenderSilence = false;
	m_RenderingSilence = false;

	m_AsioRequest.store(0);

	m_DebugRealtimeThreadID.store(0);
}


bool CASIODevice::HandleRequests()
{
	MPT_SOUNDDEV_TRACE_SCOPE();
	bool result = false;
	uint32 flags = m_AsioRequest.exchange(0);
	if(flags & AsioRequest::LatenciesChanged)
	{
		UpdateLatency();
		result = true;
	}
	return result;
}


CASIODevice::~CASIODevice()
{
	MPT_SOUNDDEV_TRACE_SCOPE();
	Close();
}


bool CASIODevice::InternalOpen()
{
	MPT_SOUNDDEV_TRACE_SCOPE();

	assert(!IsDriverOpen());

	InitMembers();

	MPT_LOG(GetLogger(), LogDebug, "sounddev", MPT_UFORMAT_MESSAGE("ASIO: Open('{}'): {}-bit, ({},{}) channels, {}Hz, hw-timing={}")(GetDeviceInternalID(), m_Settings.sampleFormat.GetBitsPerSample(), m_Settings.InputChannels, static_cast<int>(m_Settings.Channels), m_Settings.Samplerate, m_Settings.UseHardwareTiming));

	SoundDevice::ChannelMapping inputChannelMapping = SoundDevice::ChannelMapping::BaseChannel(m_Settings.InputChannels, m_Settings.InputSourceID);

	try
	{

		OpenDriver();

		if(!IsDriverOpen())
		{
			throw ASIOException("Initializing driver failed.");
		}

		ASIO::Channels channels = AsioDriver()->getChannels();
		MPT_LOG(GetLogger(), LogDebug, "sounddev", MPT_UFORMAT_MESSAGE("ASIO: getChannels() => inputChannels={} outputChannel={}")(channels.Input, channels.Output));
		if(channels.Input <= 0 && channels.Output <= 0)
		{
			m_DeviceUnavailableOnOpen = true;
			throw ASIOException("Device unavailble.");
		}
		if(m_Settings.Channels > channels.Output)
		{
			throw ASIOException("Not enough output channels.");
		}
		if(m_Settings.Channels.GetRequiredDeviceChannels() > channels.Output)
		{
			throw ASIOException("Channel mapping requires more channels than available.");
		}
		if(m_Settings.InputChannels > channels.Input)
		{
			throw ASIOException("Not enough input channels.");
		}
		if(inputChannelMapping.GetRequiredDeviceChannels() > channels.Input)
		{
			throw ASIOException("Channel mapping requires more channels than available.");
		}

		MPT_LOG(GetLogger(), LogDebug, "sounddev", MPT_UFORMAT_MESSAGE("ASIO: setSampleRate(sampleRate={})")(m_Settings.Samplerate));
		AsioDriver()->setSampleRate(m_Settings.Samplerate);

		ASIO::BufferSizes bufferSizes = AsioDriver()->getBufferSizes();
		MPT_LOG(GetLogger(), LogDebug, "sounddev", MPT_UFORMAT_MESSAGE("ASIO: getBufferSize() => minSize={} maxSize={} preferredSize={} granularity={}")(bufferSizes.Min, bufferSizes.Max, bufferSizes.Preferred, bufferSizes.Granularity));
		m_nAsioBufferLen = mpt::saturate_round<int32>(m_Settings.Latency * m_Settings.Samplerate / 2.0);
		if(bufferSizes.Min <= 0 || bufferSizes.Max <= 0 || bufferSizes.Min > bufferSizes.Max)
		{  // limits make no sense
			if(bufferSizes.Preferred > 0)
			{
				m_nAsioBufferLen = bufferSizes.Preferred;
			} else
			{
				// just leave the user value, perhaps it works
			}
		} else if(bufferSizes.Granularity < -1)
		{  // bufferSizes.Granularity value not allowed, just clamp value
			m_nAsioBufferLen = std::clamp(m_nAsioBufferLen, bufferSizes.Min, bufferSizes.Max);
		} else if(bufferSizes.Granularity == -1 && (mpt::popcount(static_cast<ASIO::ULong>(bufferSizes.Min)) != 1 || mpt::popcount(static_cast<ASIO::ULong>(bufferSizes.Max)) != 1))
		{  // bufferSizes.Granularity tells us we need power-of-2 sizes, but min or max sizes are no power-of-2
			m_nAsioBufferLen = std::clamp(m_nAsioBufferLen, bufferSizes.Min, bufferSizes.Max);
			// just start at 1 and find a matching power-of-2 in range
			const ASIO::Long bufTarget = m_nAsioBufferLen;
			for(ASIO::Long bufSize = 1; bufSize <= bufferSizes.Max && bufSize <= bufTarget; bufSize *= 2)
			{
				if(bufSize >= bufferSizes.Min)
				{
					m_nAsioBufferLen = bufSize;
				}
			}
			// if no power-of-2 in range is found, just leave the clamped value alone, perhaps it works
		} else if(bufferSizes.Granularity == -1)
		{  // sane values, power-of-2 size required between min and max
			m_nAsioBufferLen = std::clamp(m_nAsioBufferLen, bufferSizes.Min, bufferSizes.Max);
			// get the largest allowed buffer size that is smaller or equal to the target size
			const ASIO::Long bufTarget = m_nAsioBufferLen;
			for(ASIO::Long bufSize = bufferSizes.Min; bufSize <= bufferSizes.Max && bufSize <= bufTarget; bufSize *= 2)
			{
				m_nAsioBufferLen = bufSize;
			}
		} else if(bufferSizes.Granularity > 0)
		{  // buffer size in bufferSizes.Granularity steps from min to max allowed
			m_nAsioBufferLen = std::clamp(m_nAsioBufferLen, bufferSizes.Min, bufferSizes.Max);
			// get the largest allowed buffer size that is smaller or equal to the target size
			const ASIO::Long bufTarget = m_nAsioBufferLen;
			for(ASIO::Long bufSize = bufferSizes.Min; bufSize <= bufferSizes.Max && bufSize <= bufTarget; bufSize += bufferSizes.Granularity)
			{
				m_nAsioBufferLen = bufSize;
			}
		} else if(bufferSizes.Granularity == 0)
		{  // no bufferSizes.Granularity given, we should use bufferSizes.Preferred if possible
			if(bufferSizes.Preferred > 0)
			{
				m_nAsioBufferLen = bufferSizes.Preferred;
			} else if(m_nAsioBufferLen >= bufferSizes.Max)
			{  // a large latency was requested, use bufferSizes.Max
				m_nAsioBufferLen = bufferSizes.Max;
			} else
			{  // use bufferSizes.Min otherwise
				m_nAsioBufferLen = bufferSizes.Min;
			}
		} else
		{  // should not happen
#if defined(MODPLUG_TRACKER)
			MPT_ASSERT_NOTREACHED();
#else   // !MODPLUG_TRACKER
			assert(false);
#endif  // MODPLUG_TRACKER
		}

		m_BufferInfo.resize(m_Settings.GetTotalChannels());
		for(uint32 channel = 0; channel < m_Settings.GetTotalChannels(); ++channel)
		{
			m_BufferInfo[channel] = ASIO::BufferInfo();
			if(channel < m_Settings.InputChannels)
			{
				m_BufferInfo[channel].isInput = true;
				m_BufferInfo[channel].channelNum = inputChannelMapping.ToDevice(channel);
			} else
			{
				m_BufferInfo[channel].isInput = false;
				m_BufferInfo[channel].channelNum = m_Settings.Channels.ToDevice(channel - m_Settings.InputChannels);
			}
		}
		MPT_LOG(GetLogger(), LogDebug, "sounddev", MPT_UFORMAT_MESSAGE("ASIO: createBuffers(numChannels={}, bufferSize={})")(m_Settings.Channels.GetNumHostChannels(), m_nAsioBufferLen));
		AsioDriver()->template createBuffers<AppID1, AppID2>(m_BufferInfo, m_nAsioBufferLen, *this);
		m_BuffersCreated = true;
		for(std::size_t i = 0; i < m_BufferInfo.size(); ++i)
		{
			if(!m_BufferInfo[i].buffers[0] || !m_BufferInfo[i].buffers[1])
			{
				throw ASIOException("createBuffes returned nullptr.");
			}
		}

		m_ChannelInfo.resize(m_Settings.GetTotalChannels());
		for(uint32 channel = 0; channel < m_Settings.GetTotalChannels(); ++channel)
		{
			if(channel < m_Settings.InputChannels)
			{
				m_ChannelInfo[channel] = AsioDriver()->getChannelInfo(inputChannelMapping.ToDevice(channel), true);
			} else
			{
				m_ChannelInfo[channel] = AsioDriver()->getChannelInfo(m_Settings.Channels.ToDevice(channel - m_Settings.InputChannels), false);
			}
			assert(m_ChannelInfo[channel].isActive);
			MPT_LOG(GetLogger(), LogDebug, "sounddev", MPT_UFORMAT_MESSAGE("ASIO: getChannelInfo(isInput={} channel={}) => isActive={} channelGroup={} type={} name='{}'")((channel < m_Settings.InputChannels), m_Settings.Channels.ToDevice(channel), value_cast(m_ChannelInfo[channel].isActive), m_ChannelInfo[channel].channelGroup, mpt::to_underlying(m_ChannelInfo[channel].type), mpt::transcode<mpt::ustring>(mpt::logical_encoding::locale, static_cast<std::string>(m_ChannelInfo[channel].name))));
		}

		bool allChannelsAreInt = true;
		bool allChannelsAreInt16ValidBits = true;
		bool allChannelsAreNativeInt24 = true;
		bool allChannelsAreFloat32 = true;
		for(std::size_t channel = 0; channel < m_Settings.GetTotalChannels(); ++channel)
		{
			ASIO::Sample::Traits sampleTraits = ASIO::Sample::Traits(m_ChannelInfo[channel].type);
			bool isFloat = sampleTraits.is_float;
			bool isFloat32 = sampleTraits.is_float && sampleTraits.valid_bits == 32;
			bool isInt16ValidBits = !sampleTraits.is_float && sampleTraits.valid_bits == 16;
			bool isInt24 = !sampleTraits.is_float && sampleTraits.size_bytes == 3 && sampleTraits.valid_bits == 24;
			bool isNative = (mpt::endian_is_little() && !sampleTraits.is_be) || (mpt::endian_is_big() && sampleTraits.is_be);
			if(isFloat)
			{
				allChannelsAreInt = false;
			}
			if(!isInt16ValidBits)
			{
				allChannelsAreInt16ValidBits = false;
			}
			if(!(isInt24 && isNative))
			{
				allChannelsAreNativeInt24 = false;
			}
			if(!isFloat32)
			{
				allChannelsAreFloat32 = false;
			}
		}
		if(allChannelsAreInt16ValidBits)
		{
			m_Settings.sampleFormat = SampleFormat::Int16;
			m_SampleBufferInt16.resize(m_nAsioBufferLen * m_Settings.Channels);
			m_SampleInputBufferInt16.resize(m_nAsioBufferLen * m_Settings.InputChannels);
		} else if(allChannelsAreNativeInt24)
		{
			m_Settings.sampleFormat = SampleFormat::Int24;
			m_SampleBufferInt24.resize(m_nAsioBufferLen * m_Settings.Channels);
			m_SampleInputBufferInt24.resize(m_nAsioBufferLen * m_Settings.InputChannels);
		} else if(allChannelsAreInt)
		{
			m_Settings.sampleFormat = SampleFormat::Int32;
			m_SampleBufferInt32.resize(m_nAsioBufferLen * m_Settings.Channels);
			m_SampleInputBufferInt32.resize(m_nAsioBufferLen * m_Settings.InputChannels);
		} else if(allChannelsAreFloat32)
		{
			m_Settings.sampleFormat = SampleFormat::Float32;
			m_SampleBufferFloat.resize(m_nAsioBufferLen * m_Settings.Channels);
			m_SampleInputBufferFloat.resize(m_nAsioBufferLen * m_Settings.InputChannels);
		} else
		{
			m_Settings.sampleFormat = SampleFormat::Float64;
			m_SampleBufferDouble.resize(m_nAsioBufferLen * m_Settings.Channels);
			m_SampleInputBufferDouble.resize(m_nAsioBufferLen * m_Settings.InputChannels);
		}

		for(std::size_t channel = 0; channel < m_Settings.GetTotalChannels(); ++channel)
		{
			ASIO::Sample::ClearBufferASIO(m_BufferInfo[channel].buffers[0], m_ChannelInfo[channel].type, m_nAsioBufferLen);
			ASIO::Sample::ClearBufferASIO(m_BufferInfo[channel].buffers[1], m_ChannelInfo[channel].type, m_nAsioBufferLen);
		}

		m_CanOutputReady = AsioDriver()->canOutputReady();

		m_StreamPositionOffset = m_nAsioBufferLen;

		UpdateLatency();

		return true;

	} catch(...)
	{
		ExceptionHandler(__func__);
	}
	InternalClose();
	return false;
}


void CASIODevice::UpdateLatency()
{
	MPT_SOUNDDEV_TRACE_SCOPE();
	ASIO::Latencies latencies;
	try
	{
		latencies = AsioDriver()->getLatencies();
	} catch(const ASIO::Error &)
	{
		// continue, failure is not fatal here
	} catch(...)
	{
		ExceptionHandler(__func__);
	}
	if(latencies.Output >= m_nAsioBufferLen)
	{
		m_BufferLatency = static_cast<double>(latencies.Output + m_nAsioBufferLen) / static_cast<double>(m_Settings.Samplerate);  // ASIO and OpenMPT semantics of 'latency' differ by one chunk/buffer
	} else
	{
		// pointless value returned from asio driver, use a sane estimate
		m_BufferLatency = 2.0 * static_cast<double>(m_nAsioBufferLen) / static_cast<double>(m_Settings.Samplerate);
	}
}


void CASIODevice::SetRenderSilence(bool silence, bool wait)
{
	MPT_SOUNDDEV_TRACE_SCOPE();
	m_RenderSilence = silence;
	if(!wait)
	{
		return;
	}
	std::chrono::steady_clock::time_point pollingstart = std::chrono::steady_clock::now();
	while(m_RenderingSilence != silence)
	{
		if((std::chrono::steady_clock::now() - pollingstart) > std::chrono::microseconds(250))
		{
#if defined(MODPLUG_TRACKER)
			if(silence)
			{
				if(CallbackIsLockedByCurrentThread())
				{
					MPT_ASSERT_MSG(false, "AudioCriticalSection locked while stopping ASIO");
				} else
				{
					MPT_ASSERT_MSG(false, "waiting for asio failed in Stop()");
				}
			} else
			{
				if(CallbackIsLockedByCurrentThread())
				{
					MPT_ASSERT_MSG(false, "AudioCriticalSection locked while starting ASIO");
				} else
				{
					MPT_ASSERT_MSG(false, "waiting for asio failed in Start()");
				}
			}
#else   // !MODPLUG_TRACKER
			assert(false);
#endif  // MODPLUG_TRACKER
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}


bool CASIODevice::InternalStart()
{
	MPT_SOUNDDEV_TRACE_SCOPE();
#if defined(MODPLUG_TRACKER)
	MPT_ASSERT_ALWAYS_MSG(!CallbackIsLockedByCurrentThread(), "AudioCriticalSection locked while starting ASIO");
#else   // !MODPLUG_TRACKER
	assert(!CallbackIsLockedByCurrentThread());
#endif  // MODPLUG_TRACKER

	if(m_Settings.KeepDeviceRunning)
	{
		if(m_DeviceRunning)
		{
			SetRenderSilence(false, true);
			return true;
		}
	}

	SetRenderSilence(false);
	try
	{
		m_TotalFramesWritten = 0;
		AsioDriver()->start();
		m_DeviceRunning = true;
	} catch(...)
	{
		ExceptionHandler(__func__);
		return false;
	}

	return true;
}


bool CASIODevice::InternalIsPlayingSilence() const
{
	MPT_SOUNDDEV_TRACE_SCOPE();
	return m_Settings.KeepDeviceRunning && m_DeviceRunning && m_RenderSilence.load();
}


void CASIODevice::InternalEndPlayingSilence()
{
	MPT_SOUNDDEV_TRACE_SCOPE();
	if(!InternalIsPlayingSilence())
	{
		return;
	}
	m_DeviceRunning = false;
	try
	{
		AsioDriver()->stop();
	} catch(...)
	{
		ExceptionHandler(__func__);
		// continue
	}
	m_TotalFramesWritten = 0;
	SetRenderSilence(false);
}


void CASIODevice::InternalStopAndAvoidPlayingSilence()
{
	MPT_SOUNDDEV_TRACE_SCOPE();
	InternalStopImpl(true);
}

void CASIODevice::InternalStop()
{
	MPT_SOUNDDEV_TRACE_SCOPE();
	InternalStopImpl(false);
}

void CASIODevice::InternalStopImpl(bool force)
{
	MPT_SOUNDDEV_TRACE_SCOPE();
#if defined(MODPLUG_TRACKER)
	MPT_ASSERT_ALWAYS_MSG(!CallbackIsLockedByCurrentThread(), "AudioCriticalSection locked while stopping ASIO");
#else   // !MODPLUG_TRACKER
	assert(!CallbackIsLockedByCurrentThread());
#endif  // MODPLUG_TRACKER

	if(m_Settings.KeepDeviceRunning && !force)
	{
		SetRenderSilence(true, true);
		return;
	}

	m_DeviceRunning = false;
	try
	{
		AsioDriver()->stop();
	} catch(...)
	{
		ExceptionHandler(__func__);
		// continue
	}
	m_TotalFramesWritten = 0;
	SetRenderSilence(false);
}


bool CASIODevice::InternalClose()
{
	MPT_SOUNDDEV_TRACE_SCOPE();
	if(m_DeviceRunning)
	{
		m_DeviceRunning = false;
		try
		{
			AsioDriver()->stop();
		} catch(...)
		{
			ExceptionHandler(__func__);
			// continue
		}
		m_TotalFramesWritten = 0;
	}
	SetRenderSilence(false);

	m_CanOutputReady = false;
	m_SampleBufferFloat.clear();
	m_SampleBufferInt16.clear();
	m_SampleBufferInt24.clear();
	m_SampleBufferInt32.clear();
	m_SampleInputBufferFloat.clear();
	m_SampleInputBufferInt16.clear();
	m_SampleInputBufferInt24.clear();
	m_SampleInputBufferInt32.clear();
	m_ChannelInfo.clear();
	if(m_BuffersCreated)
	{
		try
		{
			AsioDriver()->disposeBuffers();
		} catch(...)
		{
			ExceptionHandler(__func__);
			// continue
		}
		m_BuffersCreated = false;
	}
	m_BufferInfo.clear();
	m_nAsioBufferLen = 0;
	m_BufferLatency = 0.0;

	CloseDriver();

	return true;
}


void CASIODevice::OpenDriver()
{
	MPT_SOUNDDEV_TRACE_SCOPE();
	if(IsDriverOpen())
	{
		return;
	}
	CLSID clsid = mpt::StringToCLSID(mpt::transcode<mpt::winstring>(GetDeviceInternalID()));
	try
	{
		if(GetAppInfo().AllowDeferredProcessing)
		{
			m_DeferredBufferSwitchDispatcher = ASIO::Windows::CreateBufferSwitchDispatcher([=](ASIO::BufferIndex bufferIndex)
																						   { this->RealtimeBufferSwitchImpl(bufferIndex); });
		}
		{
			CrashContextGuard guard{&m_Ectx};
#if defined(MODPLUG_TRACKER)
			if(GetAppInfo().MaskDriverCrashes)
			{
				m_Driver = std::make_unique<ASIO::Driver>(std::make_unique<ASIO::Windows::SEH::Driver>(clsid, GetAppInfo().GetHWND()));
			} else
#endif  // MODPLUG_TRACKER
			{
				m_Driver = std::make_unique<ASIO::Driver>(std::make_unique<ASIO::Windows::Driver>(clsid, GetAppInfo().GetHWND()));
			}
		}
	} catch(...)
	{
		ExceptionHandler(__func__);
		return;
	}
	std::string driverName;
	ASIO::Long driverVersion = 0;
	std::string driverErrorMessage;
	try
	{
		driverName = AsioDriver()->getDriverName();
		driverVersion = AsioDriver()->getDriverVersion();
		driverErrorMessage = AsioDriver()->getErrorMessage();
	} catch(...)
	{
		CloseDriver();
		ExceptionHandler(__func__);
		return;
	}
	MPT_LOG(GetLogger(), LogInformation, "sounddev", MPT_UFORMAT_MESSAGE("ASIO: Opened driver {} Version 0x{}: {}")(mpt::transcode<mpt::ustring>(mpt::logical_encoding::locale, driverName), mpt::format<mpt::ustring>::HEX0<8>(driverVersion), mpt::transcode<mpt::ustring>(mpt::logical_encoding::locale, driverErrorMessage)));
}


void CASIODevice::CloseDriver()
{
	MPT_SOUNDDEV_TRACE_SCOPE();
	if(!IsDriverOpen())
	{
		return;
	}
	try
	{
		{
			CrashContextGuard guard{&m_Ectx};
			m_Driver = nullptr;
		}
		m_DeferredBufferSwitchDispatcher = nullptr;
	} catch(...)
	{
		ExceptionHandler(__func__);
	}
}


void CASIODevice::InternalFillAudioBuffer()
{
	MPT_SOUNDDEV_TRACE_SCOPE();
	FillAsioBuffer();
}


void CASIODevice::FillAsioBuffer(bool useSource)
{
	MPT_SOUNDDEV_TRACE_SCOPE();
	const bool rendersilence = !useSource;
	const std::size_t countChunk = m_nAsioBufferLen;
	const std::size_t inputChannels = m_Settings.InputChannels;
	const std::size_t outputChannels = m_Settings.Channels;
	for(std::size_t inputChannel = 0; inputChannel < inputChannels; ++inputChannel)
	{
		std::size_t channel = inputChannel;
		const void *src = m_BufferInfo[channel].buffers[m_BufferIndex];
		ASIO::SampleType sampleType = m_ChannelInfo[channel].type;
		if(m_Settings.sampleFormat == SampleFormat::Float64)
		{
			double *const dstDouble = m_SampleInputBufferDouble.data();
			if((mpt::endian_is_little() && sampleType == ASIO::SampleType::Float64LSB) || (mpt::endian_is_big() && sampleType == ASIO::SampleType::Float64MSB))
			{
				ASIO::Sample::CopyRawFromASIO(dstDouble + inputChannel, inputChannels, src, countChunk);
			} else
			{
				ASIO::Sample::ConvertFromASIO(dstDouble + inputChannel, inputChannels, sampleType, src, countChunk);
			}
		} else if(m_Settings.sampleFormat == SampleFormat::Float32)
		{
			float *const dstFloat = m_SampleInputBufferFloat.data();
			if((mpt::endian_is_little() && sampleType == ASIO::SampleType::Float32LSB) || (mpt::endian_is_big() && sampleType == ASIO::SampleType::Float32MSB))
			{
				ASIO::Sample::CopyRawFromASIO(dstFloat + inputChannel, inputChannels, src, countChunk);
			} else
			{
				ASIO::Sample::ConvertFromASIO(dstFloat + inputChannel, inputChannels, sampleType, src, countChunk);
			}
		} else if(m_Settings.sampleFormat == SampleFormat::Int16)
		{
			int16 *const dstInt16 = m_SampleInputBufferInt16.data();
			if((mpt::endian_is_little() && sampleType == ASIO::SampleType::Int16LSB) || (mpt::endian_is_big() && sampleType == ASIO::SampleType::Int16MSB))
			{
				ASIO::Sample::CopyRawFromASIO(dstInt16 + inputChannel, inputChannels, src, countChunk);
			} else
			{
				ASIO::Sample::ConvertFromASIO(dstInt16 + inputChannel, inputChannels, sampleType, src, countChunk);
			}
		} else if(m_Settings.sampleFormat == SampleFormat::Int24)
		{
			int24 *const dstInt24 = m_SampleInputBufferInt24.data();
			assert((mpt::endian_is_little() && sampleType == ASIO::SampleType::Int24LSB) || (mpt::endian_is_big() && sampleType == ASIO::SampleType::Int24MSB));
			ASIO::Sample::CopyRawFromASIO(dstInt24 + inputChannel, inputChannels, src, countChunk);
		} else if(m_Settings.sampleFormat == SampleFormat::Int32)
		{
			int32 *const dstInt32 = m_SampleInputBufferInt32.data();
			if((mpt::endian_is_little() && sampleType == ASIO::SampleType::Int32LSB) || (mpt::endian_is_big() && sampleType == ASIO::SampleType::Int32MSB))
			{
				ASIO::Sample::CopyRawFromASIO(dstInt32 + inputChannel, inputChannels, src, countChunk);
			} else
			{
				ASIO::Sample::ConvertFromASIO(dstInt32 + inputChannel, inputChannels, sampleType, src, countChunk);
			}
		} else
		{
#if defined(MODPLUG_TRACKER)
			MPT_ASSERT_NOTREACHED();
#else   // !MODPLUG_TRACKER
			assert(false);
#endif  // MODPLUG_TRACKER
		}
	}
	if(rendersilence)
	{
		if(m_Settings.sampleFormat == SampleFormat::Float64)
		{
			std::fill(m_SampleBufferDouble.data(), m_SampleBufferDouble.data() + countChunk * outputChannels, double(0.0));
		} else if(m_Settings.sampleFormat == SampleFormat::Float32)
		{
			std::fill(m_SampleBufferFloat.data(), m_SampleBufferFloat.data() + countChunk * outputChannels, float(0.0f));
		} else if(m_Settings.sampleFormat == SampleFormat::Int16)
		{
			std::fill(m_SampleBufferInt16.data(), m_SampleBufferInt16.data() + countChunk * outputChannels, int16(0));
		} else if(m_Settings.sampleFormat == SampleFormat::Int24)
		{
			std::fill(m_SampleBufferInt24.data(), m_SampleBufferInt24.data() + countChunk * outputChannels, int24(0));
		} else if(m_Settings.sampleFormat == SampleFormat::Int32)
		{
			std::fill(m_SampleBufferInt32.data(), m_SampleBufferInt32.data() + countChunk * outputChannels, int32(0));
		} else
		{
#if defined(MODPLUG_TRACKER)
			MPT_ASSERT_NOTREACHED();
#else   // !MODPLUG_TRACKER
			assert(false);
#endif  // MODPLUG_TRACKER
		}
	} else
	{
		CallbackLockedAudioReadPrepare(countChunk, m_nAsioBufferLen * 2);
		if(m_Settings.sampleFormat == SampleFormat::Float64)
		{
			CallbackLockedAudioProcess(m_SampleBufferDouble.data(), (m_SampleInputBufferDouble.size() > 0) ? m_SampleInputBufferDouble.data() : nullptr, countChunk);
		} else if(m_Settings.sampleFormat == SampleFormat::Float32)
		{
			CallbackLockedAudioProcess(m_SampleBufferFloat.data(), (m_SampleInputBufferFloat.size() > 0) ? m_SampleInputBufferFloat.data() : nullptr, countChunk);
		} else if(m_Settings.sampleFormat == SampleFormat::Int16)
		{
			CallbackLockedAudioProcess(m_SampleBufferInt16.data(), (m_SampleInputBufferInt16.size() > 0) ? m_SampleInputBufferInt16.data() : nullptr, countChunk);
		} else if(m_Settings.sampleFormat == SampleFormat::Int24)
		{
			CallbackLockedAudioProcess(m_SampleBufferInt24.data(), (m_SampleInputBufferInt24.size() > 0) ? m_SampleInputBufferInt24.data() : nullptr, countChunk);
		} else if(m_Settings.sampleFormat == SampleFormat::Int32)
		{
			CallbackLockedAudioProcess(m_SampleBufferInt32.data(), (m_SampleInputBufferInt32.size() > 0) ? m_SampleInputBufferInt32.data() : nullptr, countChunk);
		} else
		{
#if defined(MODPLUG_TRACKER)
			MPT_ASSERT_NOTREACHED();
#else   // !MODPLUG_TRACKER
			assert(false);
#endif  // MODPLUG_TRACKER
		}
	}
	for(std::size_t outputChannel = 0; outputChannel < outputChannels; ++outputChannel)
	{
		std::size_t channel = outputChannel + m_Settings.InputChannels;
		void *dst = m_BufferInfo[channel].buffers[m_BufferIndex];
		ASIO::SampleType sampleType = m_ChannelInfo[channel].type;
		if(m_Settings.sampleFormat == SampleFormat::Float64)
		{
			const double *const srcDouble = m_SampleBufferDouble.data();
			if((mpt::endian_is_little() && sampleType == ASIO::SampleType::Float64LSB) || (mpt::endian_is_big() && sampleType == ASIO::SampleType::Float64MSB))
			{
				ASIO::Sample::CopyRawToASIO(dst, srcDouble + outputChannel, outputChannels, countChunk);
			} else
			{
				ASIO::Sample::ConvertToASIO(dst, sampleType, srcDouble + outputChannel, outputChannels, countChunk);
			}
		} else if(m_Settings.sampleFormat == SampleFormat::Float32)
		{
			const float *const srcFloat = m_SampleBufferFloat.data();
			if((mpt::endian_is_little() && sampleType == ASIO::SampleType::Float32LSB) || (mpt::endian_is_big() && sampleType == ASIO::SampleType::Float32MSB))
			{
				ASIO::Sample::CopyRawToASIO(dst, srcFloat + outputChannel, outputChannels, countChunk);
			} else
			{
				ASIO::Sample::ConvertToASIO(dst, sampleType, srcFloat + outputChannel, outputChannels, countChunk);
			}
		} else if(m_Settings.sampleFormat == SampleFormat::Int16)
		{
			const int16 *const srcInt16 = m_SampleBufferInt16.data();
			if((mpt::endian_is_little() && sampleType == ASIO::SampleType::Int16LSB) || (mpt::endian_is_big() && sampleType == ASIO::SampleType::Int16MSB))
			{
				ASIO::Sample::CopyRawToASIO(dst, srcInt16 + outputChannel, outputChannels, countChunk);
			} else
			{
				ASIO::Sample::ConvertToASIO(dst, sampleType, srcInt16 + outputChannel, outputChannels, countChunk);
			}
		} else if(m_Settings.sampleFormat == SampleFormat::Int24)
		{
			const int24 *const srcInt24 = m_SampleBufferInt24.data();
			assert((mpt::endian_is_little() && sampleType == ASIO::SampleType::Int24LSB) || (mpt::endian_is_big() && sampleType == ASIO::SampleType::Int24MSB));
			ASIO::Sample::CopyRawToASIO(dst, srcInt24 + outputChannel, outputChannels, countChunk);
		} else if(m_Settings.sampleFormat == SampleFormat::Int32)
		{
			const int32 *const srcInt32 = m_SampleBufferInt32.data();
			if((mpt::endian_is_little() && sampleType == ASIO::SampleType::Int32LSB) || (mpt::endian_is_big() && sampleType == ASIO::SampleType::Int32MSB))
			{
				ASIO::Sample::CopyRawToASIO(dst, srcInt32 + outputChannel, outputChannels, countChunk);
			} else
			{
				ASIO::Sample::ConvertToASIO(dst, sampleType, srcInt32 + outputChannel, outputChannels, countChunk);
			}
		} else
		{
#if defined(MODPLUG_TRACKER)
			MPT_ASSERT_NOTREACHED();
#else   // !MODPLUG_TRACKER
			assert(false);
#endif  // MODPLUG_TRACKER
		}
	}
	if(m_CanOutputReady)
	{
		try
		{
			AsioDriver()->outputReady();  // do not handle errors, there is nothing we could do about them
		} catch(...)
		{
			ExceptionHandler(__func__);
		}
	}
	if(!rendersilence)
	{
		CallbackLockedAudioProcessDone();
	}
}


bool CASIODevice::InternalHasTimeInfo() const
{
	MPT_SOUNDDEV_TRACE_SCOPE();
	return m_Settings.UseHardwareTiming;
}


SoundDevice::BufferAttributes CASIODevice::InternalGetEffectiveBufferAttributes() const
{
	SoundDevice::BufferAttributes bufferAttributes;
	bufferAttributes.Latency = m_BufferLatency;
	bufferAttributes.UpdateInterval = static_cast<double>(m_nAsioBufferLen) / static_cast<double>(m_Settings.Samplerate);
	bufferAttributes.NumBuffers = 2;
	return bufferAttributes;
}


namespace
{
struct DebugRealtimeThreadIdGuard
{
	std::atomic<uint32> &ThreadID;
	DebugRealtimeThreadIdGuard(std::atomic<uint32> &ThreadID)
		: ThreadID(ThreadID)
	{
		ThreadID.store(GetCurrentThreadId());
	}
	~DebugRealtimeThreadIdGuard()
	{
		ThreadID.store(0);
	}
};
}  // namespace


void CASIODevice::RealtimeSampleRateDidChange(ASIO::SampleRate sRate) noexcept
{
	MPT_SOUNDDEV_TRACE_SCOPE();
	if(mpt::saturate_round<uint32>(sRate) == m_Settings.Samplerate)
	{
		// not different, ignore it
		return;
	}
	m_UsedFeatures.fetch_or(AsioFeature::SampleRateChange);
	if(static_cast<double>(m_Settings.Samplerate) * (1.0 - AsioSampleRateTolerance) <= sRate && sRate <= static_cast<double>(m_Settings.Samplerate) * (1.0 + AsioSampleRateTolerance))
	{
		// ignore slight differences which might be due to a unstable external ASIO clock source
		return;
	}
	// play safe and close the device
	RequestClose();
}


void CASIODevice::RealtimeRequestDeferredProcessing(bool deferred) noexcept
{
	MPT_SOUNDDEV_TRACE_SCOPE();
	DebugRealtimeThreadIdGuard debugThreadIdGuard(m_DebugRealtimeThreadID);
	if(deferred)
	{
		m_UsedFeatures.fetch_or(AsioFeature::DeferredProcess);
	}
	m_DeferredProcessing = deferred;
}


void CASIODevice::RealtimeTimeInfo(ASIO::Time asioTime) noexcept
{
	MPT_SOUNDDEV_TRACE_SCOPE();
	DebugRealtimeThreadIdGuard debugThreadIdGuard(m_DebugRealtimeThreadID);
	if(m_Settings.UseHardwareTiming)
	{
		SoundDevice::TimeInfo timeInfo;
		if((asioTime.timeInfo.flags & ASIO::TimeInfoFlagSamplePositionValid) && (asioTime.timeInfo.flags & ASIO::TimeInfoFlagSystemTimeValid))
		{
			double speed = 1.0;
			if((asioTime.timeInfo.flags & ASIO::TimeInfoFlagSpeedValid) && (asioTime.timeInfo.speed > 0.0))
			{
				speed = asioTime.timeInfo.speed;
			} else if((asioTime.timeInfo.flags & ASIO::TimeInfoFlagSampleRateValid) && (asioTime.timeInfo.sampleRate > 0.0))
			{
				speed *= asioTime.timeInfo.sampleRate / m_Settings.Samplerate;
			}
			timeInfo.SyncPointStreamFrames = asioTime.timeInfo.samplePosition - m_StreamPositionOffset;
			timeInfo.SyncPointSystemTimestamp = asioTime.timeInfo.systemTime;
			timeInfo.Speed = speed;
		} else
		{  // spec violation or nothing provided at all, better to estimate this stuff ourselves
			const uint64 asioNow = CallbackLockedGetReferenceClockNowNanoseconds();
			timeInfo.SyncPointStreamFrames = m_TotalFramesWritten + m_nAsioBufferLen - m_StreamPositionOffset;
			timeInfo.SyncPointSystemTimestamp = asioNow + mpt::saturate_round<int64>(m_BufferLatency * 1000.0 * 1000.0 * 1000.0);
			timeInfo.Speed = 1.0;
		}
		timeInfo.RenderStreamPositionBefore = StreamPositionFromFrames(m_TotalFramesWritten - m_StreamPositionOffset);
		timeInfo.RenderStreamPositionAfter = StreamPositionFromFrames(m_TotalFramesWritten - m_StreamPositionOffset + m_nAsioBufferLen);
		timeInfo.Latency = GetEffectiveBufferAttributes().Latency;
		SetTimeInfo(timeInfo);
	}
}


void CASIODevice::RealtimeBufferSwitch(ASIO::BufferIndex bufferIndex) noexcept
{
	MPT_SOUNDDEV_TRACE_SCOPE();
	if(m_DeferredBufferSwitchDispatcher && m_DeferredProcessing)
	{
		m_DeferredBufferSwitchDispatcher->Dispatch(bufferIndex);
	} else
	{
		RealtimeBufferSwitchImpl(bufferIndex);
	}
}


void CASIODevice::RealtimeBufferSwitchImpl(ASIO::BufferIndex bufferIndex) noexcept
{
	MPT_SOUNDDEV_TRACE_SCOPE();
	DebugRealtimeThreadIdGuard debugThreadIdGuard(m_DebugRealtimeThreadID);
	m_BufferIndex = bufferIndex;
	bool rendersilence = m_RenderSilence;
	m_RenderingSilence = rendersilence;
	if(rendersilence)
	{
		m_StreamPositionOffset += m_nAsioBufferLen;
		FillAsioBuffer(false);
	} else
	{
		CallbackFillAudioBufferLocked();
	}
	m_TotalFramesWritten += m_nAsioBufferLen;
}


mpt::ustring CASIODevice::AsioFeaturesToString(AsioFeatures features)
{
	std::vector<mpt::ustring> results;
	if(features & AsioFeature::ResetRequest)
	{
		results.push_back(MPT_USTRING("reset"));
	}
	if(features & AsioFeature::ResyncRequest)
	{
		results.push_back(MPT_USTRING("resync"));
	}
	if(features & AsioFeature::BufferSizeChange)
	{
		results.push_back(MPT_USTRING("buffer"));
	}
	if(features & AsioFeature::Overload)
	{
		results.push_back(MPT_USTRING("load"));
	}
	if(features & AsioFeature::SampleRateChange)
	{
		results.push_back(MPT_USTRING("srate"));
	}
	if(features & AsioFeature::DeferredProcess)
	{
		results.push_back(MPT_USTRING("deferred"));
	}
	return mpt::join(results, MPT_USTRING(","));
}


bool CASIODevice::DebugIsFragileDevice() const
{
	return true;
}


bool CASIODevice::DebugInRealtimeCallback() const
{
	return GetCurrentThreadId() == m_DebugRealtimeThreadID.load();
}


SoundDevice::Statistics CASIODevice::GetStatistics() const
{
	MPT_SOUNDDEV_TRACE_SCOPE();
	SoundDevice::Statistics result;
	result.InstantaneousLatency = m_BufferLatency;
	result.LastUpdateInterval = static_cast<double>(m_nAsioBufferLen) / static_cast<double>(m_Settings.Samplerate);
	result.text = mpt::ustring();
	const AsioFeatures unsupported = AsioFeature::Overload | AsioFeature::BufferSizeChange | AsioFeature::SampleRateChange;
	AsioFeatures usedFeatures = m_UsedFeatures.fetch_or(0);
	AsioFeatures unsupportedFeatues = usedFeatures & unsupported;
	if(unsupportedFeatues)
	{
		result.text = MPT_UFORMAT_MESSAGE("WARNING: unsupported features: {}")(AsioFeaturesToString(unsupportedFeatues));
	} else if(usedFeatures)
	{
		result.text = MPT_UFORMAT_MESSAGE("OK, features used: {}")(AsioFeaturesToString(m_UsedFeatures));
	} else
	{
		result.text = MPT_USTRING("OK.");
	}
	return result;
}


void CASIODevice::MessageResetRequest() noexcept
{
	MPT_SOUNDDEV_TRACE_SCOPE();
	m_UsedFeatures.fetch_or(AsioFeature::ResetRequest);
	RequestReset();
}

bool CASIODevice::MessageBufferSizeChange(ASIO::Long newSize) noexcept
{
	MPT_SOUNDDEV_TRACE_SCOPE();
	MPT_UNUSED(newSize);
	m_UsedFeatures.fetch_or(AsioFeature::BufferSizeChange);
	// We do not support ASIO::MessageSelector::BufferSizeChange.
	// This should cause a driver to send a ASIO::MessageSelector::ResetRequest.
	return false;
}

bool CASIODevice::MessageResyncRequest() noexcept
{
	MPT_SOUNDDEV_TRACE_SCOPE();
	m_UsedFeatures.fetch_or(AsioFeature::ResyncRequest);
	RequestRestart();
	return true;
}

void CASIODevice::MessageLatenciesChanged() noexcept
{
	MPT_SOUNDDEV_TRACE_SCOPE();
	m_AsioRequest.fetch_or(AsioRequest::LatenciesChanged);
}

ASIO::Long CASIODevice::MessageMMCCommand(ASIO::Long value, const void *message, const ASIO::Double *opt) noexcept
{
	MPT_SOUNDDEV_TRACE_SCOPE();
	ASIO::Long result = 0;
	MPT_LOG(GetLogger(), LogDebug, "sounddev", MPT_UFORMAT_MESSAGE("ASIO: MMCCommand(value={}, message={}, opt={}) => result={}")(value, reinterpret_cast<std::uintptr_t>(message), opt ? mpt::format<mpt::ustring>::val(*opt) : MPT_USTRING("NULL"), result));
	return result;
}

void CASIODevice::MessageOverload() noexcept
{
	MPT_SOUNDDEV_TRACE_SCOPE();
	m_UsedFeatures.fetch_or(AsioFeature::Overload);
}

ASIO::Long CASIODevice::MessageUnknown(ASIO::MessageSelector selector, ASIO::Long value, const void *message, const ASIO::Double *opt) noexcept
{
	MPT_SOUNDDEV_TRACE_SCOPE();
	ASIO::Long result = 0;
	MPT_LOG(GetLogger(), LogDebug, "sounddev", MPT_UFORMAT_MESSAGE("ASIO: AsioMessage(selector={}, value={}, message={}, opt={}) => result={}")(mpt::to_underlying(selector), value, reinterpret_cast<std::uintptr_t>(message), opt ? mpt::format<mpt::ustring>::val(*opt) : MPT_USTRING("NULL"), result));
	return result;
}


void CASIODevice::ExceptionHandler(const char *func)
{
	MPT_SOUNDDEV_TRACE_SCOPE();
	try
	{
		throw;  // rethrow
#if defined(MODPLUG_TRACKER)
	} catch(const ASIO::Windows::SEH::DriverCrash &e)
	{
#if !defined(MPT_BUILD_WINESUPPORT)
		ExceptionHandler::TaintProcess(ExceptionHandler::TaintReason::Driver);
#endif  // !MPT_BUILD_WINESUPPORT
		MPT_LOG(GetLogger(), LogError, "sounddev", MPT_UFORMAT_MESSAGE("ASIO: {}: Driver Crash: {}!")(mpt::transcode<mpt::ustring>(mpt::source_encoding, func), mpt::transcode<mpt::ustring>(mpt::source_encoding, std::string(e.func()))));
		SendDeviceMessage(LogError, MPT_UFORMAT_MESSAGE("ASIO Driver Crash: {}")(mpt::transcode<mpt::ustring>(mpt::source_encoding, std::string(e.func()))));
#endif  // MODPLUG_TRACKER
	} catch(const std::bad_alloc &)
	{
		mpt::throw_out_of_memory();
	} catch(const ASIO::Windows::DriverLoadFailed &e)
	{
		MPT_LOG(GetLogger(), LogDebug, "sounddev", MPT_UFORMAT_MESSAGE("ASIO: {}: Driver Load: {}")(mpt::transcode<mpt::ustring>(mpt::source_encoding, func), mpt::get_exception_text<mpt::ustring>(e)));
	} catch(const ASIO::Windows::DriverInitFailed &e)
	{
		MPT_LOG(GetLogger(), LogDebug, "sounddev", MPT_UFORMAT_MESSAGE("ASIO: {}: Driver Init: {}")(mpt::transcode<mpt::ustring>(mpt::source_encoding, func), mpt::get_exception_text<mpt::ustring>(e)));
	} catch(const ASIO::Error &e)
	{
		MPT_LOG(GetLogger(), LogDebug, "sounddev", MPT_UFORMAT_MESSAGE("ASIO: {}: Error: {}")(mpt::transcode<mpt::ustring>(mpt::source_encoding, func), mpt::get_exception_text<mpt::ustring>(e)));
	} catch(const std::exception &e)
	{
		MPT_LOG(GetLogger(), LogDebug, "sounddev", MPT_UFORMAT_MESSAGE("ASIO: {}: Exception: {}")(mpt::transcode<mpt::ustring>(mpt::source_encoding, func), mpt::get_exception_text<mpt::ustring>(e)));
	} catch(...)
	{
		MPT_LOG(GetLogger(), LogDebug, "sounddev", MPT_UFORMAT_MESSAGE("ASIO: {}: Unknown Exception")(mpt::transcode<mpt::ustring>(mpt::source_encoding, func)));
	}
}


SoundDevice::Caps CASIODevice::InternalGetDeviceCaps()
{
	MPT_SOUNDDEV_TRACE_SCOPE();
	SoundDevice::Caps caps;

	caps.Available = true;
	caps.CanUpdateInterval = false;
	caps.CanSampleFormat = false;
	caps.CanExclusiveMode = false;
	caps.CanBoostThreadPriority = false;
	caps.CanKeepDeviceRunning = true;
	caps.CanUseHardwareTiming = true;
	caps.CanChannelMapping = true;
	caps.CanInput = true;
	caps.HasNamedInputSources = true;
	caps.CanDriverPanel = true;

	caps.LatencyMin = 0.000001;    // 1 us
	caps.LatencyMax = 0.5;         // 500 ms
	caps.UpdateIntervalMin = 0.0;  // disabled
	caps.UpdateIntervalMax = 0.0;  // disabled

	caps.DefaultSettings.sampleFormat = SampleFormat::Float32;

	return caps;
}


SoundDevice::DynamicCaps CASIODevice::GetDeviceDynamicCaps(const std::vector<uint32> &baseSampleRates)
{
	MPT_SOUNDDEV_TRACE_SCOPE();

	SoundDevice::DynamicCaps caps;

	TemporaryASIODriverOpener opener(*this);
	if(!IsDriverOpen())
	{
		m_DeviceUnavailableOnOpen = true;
		return caps;
	}

	try
	{
		ASIO::SampleRate samplerate = AsioDriver()->getSampleRate();
		if(samplerate > 0.0)
		{
			caps.currentSampleRate = mpt::saturate_round<uint32>(samplerate);
		}
	} catch(...)
	{
		ExceptionHandler(__func__);
		// continue
	}

	for(size_t i = 0; i < baseSampleRates.size(); i++)
	{
		try
		{
			if(AsioDriver()->canSampleRate(static_cast<ASIO::SampleRate>(baseSampleRates[i])))
			{
				caps.supportedSampleRates.push_back(baseSampleRates[i]);
				caps.supportedExclusiveSampleRates.push_back(baseSampleRates[i]);
			}
		} catch(...)
		{
			ExceptionHandler(__func__);
			// continue
		}
	}

	try
	{
		ASIO::Channels channels = AsioDriver()->getChannels();
		if(!((channels.Input > 0) || (channels.Output > 0)))
		{
			m_DeviceUnavailableOnOpen = true;
		}
		for(ASIO::Long i = 0; i < channels.Output; ++i)
		{
			mpt::ustring name = mpt::format<mpt::ustring>::dec(i);
			try
			{
				ASIO::ChannelInfo channelInfo = AsioDriver()->getChannelInfo(i, false);
				name = mpt::transcode<mpt::ustring>(mpt::logical_encoding::locale, static_cast<std::string>(channelInfo.name));
			} catch(...)
			{
				ExceptionHandler(__func__);
				// continue
			}
			caps.channelNames.push_back(name);
		}
		for(ASIO::Long i = 0; i < channels.Input; ++i)
		{
			mpt::ustring name = mpt::format<mpt::ustring>::dec(i);
			try
			{
				ASIO::ChannelInfo channelInfo = AsioDriver()->getChannelInfo(i, true);
				name = mpt::transcode<mpt::ustring>(mpt::logical_encoding::locale, static_cast<std::string>(channelInfo.name));
			} catch(...)
			{
				ExceptionHandler(__func__);
				// continue
			}
			caps.inputSourceNames.push_back(std::make_pair(static_cast<uint32>(i), name));
		}
	} catch(...)
	{
		ExceptionHandler(__func__);
		// continue
	}
	return caps;
}


bool CASIODevice::OpenDriverSettings()
{
	MPT_SOUNDDEV_TRACE_SCOPE();
	bool result = false;
	TemporaryASIODriverOpener opener(*this);
	if(!IsDriverOpen())
	{
		return false;
	}
	try
	{
		result = AsioDriver()->controlPanel();
	} catch(...)
	{
		ExceptionHandler(__func__);
		return false;
	}
	return result;
}


#endif  // MPT_WITH_ASIO


}  // namespace SoundDevice


OPENMPT_NAMESPACE_END
