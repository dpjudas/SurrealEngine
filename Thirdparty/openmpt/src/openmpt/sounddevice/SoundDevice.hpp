/* SPDX-License-Identifier: BSD-3-Clause */
/* SPDX-FileCopyrightText: Olivier Lapicque */
/* SPDX-FileCopyrightText: OpenMPT Project Developers and Contributors */


#pragma once

#include "openmpt/all/BuildSettings.hpp"

#include "SoundDeviceCallback.hpp"

#include "mpt/base/detect.hpp"
#include "mpt/base/saturate_round.hpp"
#include "mpt/osinfo/class.hpp"
#include "mpt/osinfo/windows_version.hpp"
#include "mpt/osinfo/windows_wine_version.hpp"
#include "mpt/string/types.hpp"
#include "openmpt/base/FlagSet.hpp"
#include "openmpt/base/Types.hpp"
#include "openmpt/logging/Logger.hpp"
#include "openmpt/soundbase/SampleFormat.hpp"

#include <map>
#include <utility>
#include <vector>

#include <cassert>
#include <cstddef>
#include <cstdint>

#if MPT_OS_WINDOWS
#include <windows.h>
#endif  // MPT_OS_WINDOWS

#if defined(MODPLUG_TRACKER)
#include "Logging.h"
#endif  // MODPLUG_TRACKER


OPENMPT_NAMESPACE_BEGIN


namespace SoundDevice
{


#ifndef MPT_SOUNDDEV_TRACE
#if defined(MODPLUG_TRACKER)
#define MPT_SOUNDDEV_TRACE() MPT_TRACE()
#else  // !MODPLUG_TRACKER
#define MPT_SOUNDDEV_TRACE() \
	do \
	{ \
	} while(0)
#endif  // MODPLUG_TRACKER
#endif
#ifndef MPT_SOUNDDEV_TRACE_SCOPE
#if defined(MODPLUG_TRACKER)
#define MPT_SOUNDDEV_TRACE_SCOPE() MPT_TRACE_SCOPE()
#else  // !MODPLUG_TRACKER
#define MPT_SOUNDDEV_TRACE_SCOPE() \
	do \
	{ \
	} while(0)
#endif  // MODPLUG_TRACKER
#endif


class IMessageReceiver
{
public:
	virtual void SoundDeviceMessage(LogLevel level, const mpt::ustring &str) = 0;
};


inline constexpr mpt::uchar TypeWAVEOUT[] = MPT_ULITERAL("WaveOut");
inline constexpr mpt::uchar TypeDSOUND[] = MPT_ULITERAL("DirectSound");
inline constexpr mpt::uchar TypeASIO[] = MPT_ULITERAL("ASIO");
inline constexpr mpt::uchar TypePORTAUDIO_WASAPI[] = MPT_ULITERAL("WASAPI");
inline constexpr mpt::uchar TypePORTAUDIO_WDMKS[] = MPT_ULITERAL("WDM-KS");
inline constexpr mpt::uchar TypePORTAUDIO_WMME[] = MPT_ULITERAL("MME");
inline constexpr mpt::uchar TypePORTAUDIO_DS[] = MPT_ULITERAL("DS");

typedef mpt::ustring Type;


typedef mpt::ustring Identifier;

SoundDevice::Type ParseType(const SoundDevice::Identifier &identifier);


struct Info
{
	SoundDevice::Type type;
	mpt::ustring internalID;
	mpt::ustring name;                  // user visible (and configuration key if useNameAsIdentifier)
	mpt::ustring apiName;               // user visible
	std::vector<mpt::ustring> apiPath;  // i.e. Wine-support, PortAudio
	enum class Default
	{
		None = 0,
		Named = 1,
		Managed = 2,
	};
	Default default_;
	bool useNameAsIdentifier;

	enum class DefaultFor : int8
	{
		System = 3,
		ProAudio = 2,
		LowLevel = 1,
		None = 0,
	};
	struct ManagerFlags
	{
		DefaultFor defaultFor = DefaultFor::None;
	};
	ManagerFlags managerFlags;

	enum class Usability : int8
	{
		Usable = 3,
		Experimental = 2,
		Legacy = 1,
		Unknown = 0,
		Deprecated = -4,
		Broken = -5,
		NotAvailable = -6,
	};
	enum class Level : int8
	{
		Primary = 1,
		Unknown = 0,
		Secondary = -1,
	};
	enum class Compatible : int8
	{
		Yes = 1,
		Unknown = 0,
		No = -1,
	};
	enum class Api : int8
	{
		Native = 1,
		Unknown = 0,
		Emulated = -1,
	};
	enum class Io : int8
	{
		FullDuplex = 1,
		Unknown = 0,
		OutputOnly = -1,
	};
	enum class Mixing : int8
	{
		Server = 2,
		Software = 1,
		Unknown = 0,
		Hardware = -1,
	};
	enum class Implementor : int8
	{
		OpenMPT = 1,
		Unknown = 0,
		External = -1,
	};
	struct Flags
	{
		Usability usability = Usability::Unknown;
		Level level = Level::Unknown;
		Compatible compatible = Compatible::Unknown;
		Api api = Api::Unknown;
		Io io = Io::Unknown;
		Mixing mixing = Mixing::Unknown;
		Implementor implementor = Implementor::Unknown;
	};
	Flags flags;

	std::map<mpt::ustring, mpt::ustring> extraData;  // user visible (hidden by default)

	Info()
		: default_(Default::None)
		, useNameAsIdentifier(false)
	{
	}

	bool IsValid() const
	{
		return !type.empty() && !internalID.empty();
	}

	bool IsDeprecated() const
	{
		return (static_cast<int8>(flags.usability) <= 0) || (static_cast<int8>(flags.level) <= 0);
	}

	SoundDevice::Identifier GetIdentifier() const;

	mpt::ustring GetDisplayName() const;
};


struct ChannelMapping
{

private:
	std::vector<int32> ChannelToDeviceChannel;

public:
	static constexpr int32 MaxDeviceChannel = 32000;

public:
	// Construct default identity mapping
	ChannelMapping(uint32 numHostChannels = 2);

	// Construct mapping from given vector.
	// Silently fall back to identity mapping if mapping is invalid.
	ChannelMapping(const std::vector<int32> &mapping);

	// Construct mapping for #channels with a baseChannel offset.
	static ChannelMapping BaseChannel(uint32 channels, int32 baseChannel);

private:
	// check that the channel mapping is actually a 1:1 mapping
	static bool IsValid(const std::vector<int32> &mapping);

public:
	operator int() const
	{
		return GetNumHostChannels();
	}

	ChannelMapping &operator=(int channels)
	{
		return (*this = ChannelMapping(channels));
	}

	friend bool operator==(const SoundDevice::ChannelMapping &a, const SoundDevice::ChannelMapping &b)
	{
		return (a.ChannelToDeviceChannel == b.ChannelToDeviceChannel);
	}

	friend bool operator!=(const SoundDevice::ChannelMapping &a, const SoundDevice::ChannelMapping &b)
	{
		return (a.ChannelToDeviceChannel != b.ChannelToDeviceChannel);
	}

	friend bool operator==(int a, const SoundDevice::ChannelMapping &b)
	{
		return (a == static_cast<int>(b));
	}

	friend bool operator==(const SoundDevice::ChannelMapping &a, int b)
	{
		return (static_cast<int>(a) == b);
	}

	friend bool operator!=(int a, const SoundDevice::ChannelMapping &b)
	{
		return (a != static_cast<int>(b));
	}

	friend bool operator!=(const SoundDevice::ChannelMapping &a, int b)
	{
		return (static_cast<int>(a) != b);
	}

	uint32 GetNumHostChannels() const
	{
		return static_cast<uint32>(ChannelToDeviceChannel.size());
	}

	// Get the number of required device channels for this mapping. Derived from the maximum mapped-to channel number.
	int32 GetRequiredDeviceChannels() const
	{
		if(ChannelToDeviceChannel.empty())
		{
			return 0;
		}
		int32 maxChannel = 0;
		for(uint32 channel = 0; channel < ChannelToDeviceChannel.size(); ++channel)
		{
			if(ChannelToDeviceChannel[channel] > maxChannel)
			{
				maxChannel = ChannelToDeviceChannel[channel];
			}
		}
		return maxChannel + 1;
	}

	// Convert OpenMPT channel number to the mapped device channel number.
	int32 ToDevice(uint32 channel) const
	{
		if(channel >= ChannelToDeviceChannel.size())
		{
			return channel;
		}
		return ChannelToDeviceChannel[channel];
	}

	mpt::ustring ToUString() const;

	static SoundDevice::ChannelMapping FromString(const mpt::ustring &str);
};


struct SysInfo
{
public:
	mpt::osinfo::osclass SystemClass = mpt::osinfo::osclass::Unknown;
	mpt::osinfo::windows::Version WindowsVersion = mpt::osinfo::windows::Version::NoWindows();
	bool IsWine = false;
	mpt::osinfo::osclass WineHostClass = mpt::osinfo::osclass::Unknown;
	mpt::osinfo::windows::wine::version WineVersion;

public:
	bool IsOriginal() const { return !IsWine; }
	bool IsWindowsOriginal() const { return !IsWine; }
	bool IsWindowsWine() const { return IsWine; }

public:
	SysInfo() = delete;
	SysInfo(mpt::osinfo::osclass systemClass)
		: SystemClass(systemClass)
	{
		assert(SystemClass != mpt::osinfo::osclass::Windows);
		return;
	}
	SysInfo(mpt::osinfo::osclass systemClass, mpt::osinfo::windows::Version windowsVersion)
		: SystemClass(systemClass)
		, WindowsVersion(windowsVersion)
	{
		return;
	}
	SysInfo(mpt::osinfo::osclass systemClass, mpt::osinfo::windows::Version windowsVersion, bool isWine, mpt::osinfo::osclass wineHostClass, mpt::osinfo::windows::wine::version wineVersion)
		: SystemClass(systemClass)
		, WindowsVersion(windowsVersion)
		, IsWine(isWine)
		, WineHostClass(wineHostClass)
		, WineVersion(wineVersion)
	{
		return;
	}
};


struct AppInfo
{
	mpt::ustring Name;
	std::uintptr_t UIHandle;  // HWND on Windows
	int BoostedThreadPriorityXP;
	mpt::ustring BoostedThreadMMCSSClassVista;
	bool BoostedThreadRealtimePosix;
	int BoostedThreadNicenessPosix;
	int BoostedThreadRtprioPosix;
#if defined(MODPLUG_TRACKER)
	bool MaskDriverCrashes;
#endif  // MODPLUG_TRACKER
	bool AllowDeferredProcessing;
	AppInfo()
		: UIHandle(0)
		, BoostedThreadPriorityXP(2)  // THREAD_PRIORITY_HIGHEST
		, BoostedThreadMMCSSClassVista(MPT_USTRING("Pro Audio"))
		, BoostedThreadRealtimePosix(false)
		, BoostedThreadNicenessPosix(-5)
		, BoostedThreadRtprioPosix(10)
#if defined(MODPLUG_TRACKER)
		, MaskDriverCrashes(false)
#endif  // MODPLUG_TRACKER
		, AllowDeferredProcessing(true)
	{
		return;
	}
	AppInfo &SetName(const mpt::ustring &name)
	{
		Name = name;
		return *this;
	}
	mpt::ustring GetName() const
	{
		return Name;
	}
#if MPT_OS_WINDOWS
	AppInfo &SetHWND(HWND hwnd)
	{
		UIHandle = reinterpret_cast<uintptr_t>(hwnd);
		return *this;
	}
	HWND GetHWND() const
	{
		return reinterpret_cast<HWND>(UIHandle);
	}
#endif  // MPT_OS_WINDOWS
};


struct Settings
{
	double Latency;         // seconds
	double UpdateInterval;  // seconds
	uint32 Samplerate;
	SoundDevice::ChannelMapping Channels;
	uint8 InputChannels;
	SampleFormat sampleFormat;
	bool ExclusiveMode;        // Use hardware buffers directly
	bool BoostThreadPriority;  // Boost thread priority for glitch-free audio rendering
	bool KeepDeviceRunning;
	bool UseHardwareTiming;
	int32 DitherType;
	uint32 InputSourceID;
	Settings()
		: Latency(0.1)
		, UpdateInterval(0.005)
		, Samplerate(48000)
		, Channels(2)
		, InputChannels(0)
		, sampleFormat(SampleFormat::Float32)
		, ExclusiveMode(false)
		, BoostThreadPriority(true)
		, KeepDeviceRunning(true)
		, UseHardwareTiming(false)
		, DitherType(1)
		, InputSourceID(0)
	{
		return;
	}
	bool operator==(const SoundDevice::Settings &cmp) const
	{
		return true
			&& mpt::saturate_round<int64>(Latency * 1000000000.0) == mpt::saturate_round<int64>(cmp.Latency * 1000000000.0)                // compare in nanoseconds
			&& mpt::saturate_round<int64>(UpdateInterval * 1000000000.0) == mpt::saturate_round<int64>(cmp.UpdateInterval * 1000000000.0)  // compare in nanoseconds
			&& Samplerate == cmp.Samplerate
			&& Channels == cmp.Channels
			&& InputChannels == cmp.InputChannels
			&& sampleFormat == cmp.sampleFormat
			&& ExclusiveMode == cmp.ExclusiveMode
			&& BoostThreadPriority == cmp.BoostThreadPriority
			&& KeepDeviceRunning == cmp.KeepDeviceRunning
			&& UseHardwareTiming == cmp.UseHardwareTiming
			&& DitherType == cmp.DitherType
			&& InputSourceID == cmp.InputSourceID;
	}
	bool operator!=(const SoundDevice::Settings &cmp) const
	{
		return !(*this == cmp);
	}
	std::size_t GetBytesPerFrame() const
	{
		return sampleFormat.GetSampleSize() * Channels;
	}
	std::size_t GetBytesPerSecond() const
	{
		return Samplerate * GetBytesPerFrame();
	}
	uint32 GetTotalChannels() const
	{
		return InputChannels + Channels;
	}
};


struct Flags
{
	// Windows since Vista has a limiter/compressor in the audio path that kicks
	// in as soon as there are samples > 0dBFs (i.e. the absolute float value >
	// 1.0). This happens for all APIs that get processed through the system-
	// wide audio engine. It does not happen for exclusive mode WASAPI streams
	// or direct WaveRT (labeled WDM-KS in PortAudio) streams. As there is no
	// known way to disable this annoying behavior, avoid unclipped samples on
	// affected windows versions and clip them ourselves before handing them to
	// the APIs.
	bool WantsClippedOutput;
	Flags()
		: WantsClippedOutput(false)
	{
		return;
	}
};


struct Caps
{
	bool Available;
	bool CanUpdateInterval;
	bool CanSampleFormat;
	bool CanExclusiveMode;
	bool CanBoostThreadPriority;
	bool CanKeepDeviceRunning;
	bool CanUseHardwareTiming;
	bool CanChannelMapping;
	bool CanInput;
	bool HasNamedInputSources;
	bool CanDriverPanel;
	bool HasInternalDither;
	mpt::ustring ExclusiveModeDescription;
	double LatencyMin;
	double LatencyMax;
	double UpdateIntervalMin;
	double UpdateIntervalMax;
	SoundDevice::Settings DefaultSettings;
	Caps()
		: Available(false)
		, CanUpdateInterval(true)
		, CanSampleFormat(true)
		, CanExclusiveMode(false)
		, CanBoostThreadPriority(true)
		, CanKeepDeviceRunning(false)
		, CanUseHardwareTiming(false)
		, CanChannelMapping(false)
		, CanInput(false)
		, HasNamedInputSources(false)
		, CanDriverPanel(false)
		, HasInternalDither(false)
		, ExclusiveModeDescription(MPT_USTRING("Use device exclusively"))
		, LatencyMin(0.002)         // 2ms
		, LatencyMax(0.5)           // 500ms
		, UpdateIntervalMin(0.001)  // 1ms
		, UpdateIntervalMax(0.2)    // 200ms
	{
		return;
	}
};


struct DynamicCaps
{
	uint32 currentSampleRate = 0;
	std::vector<uint32> supportedSampleRates;
	std::vector<uint32> supportedExclusiveSampleRates;
	std::vector<SampleFormat> supportedSampleFormats = DefaultSampleFormats<std::vector<SampleFormat>>();
	std::vector<SampleFormat> supportedExclusiveModeSampleFormats = DefaultSampleFormats<std::vector<SampleFormat>>();
	std::vector<mpt::ustring> channelNames;
	std::vector<std::pair<uint32, mpt::ustring>> inputSourceNames;
};


struct BufferAttributes
{
	double Latency;         // seconds
	double UpdateInterval;  // seconds
	int NumBuffers;
	BufferAttributes()
		: Latency(0.0)
		, UpdateInterval(0.0)
		, NumBuffers(0)
	{
		return;
	}
};


enum RequestFlags : uint32
{
	RequestFlagClose = 1 << 0,
	RequestFlagReset = 1 << 1,
	RequestFlagRestart = 1 << 2,
};
MPT_DECLARE_ENUM(RequestFlags)


struct Statistics
{
	double InstantaneousLatency;
	double LastUpdateInterval;
	mpt::ustring text;
	Statistics()
		: InstantaneousLatency(0.0)
		, LastUpdateInterval(0.0)
	{
		return;
	}
};


class BackendInitializer
{
public:
	BackendInitializer() = default;
	virtual void Reload()
	{
		return;
	}
	virtual ~BackendInitializer() = default;
};


class IBase
{

protected:
	IBase() = default;

public:
	virtual ~IBase() = default;

public:
	virtual void SetMessageReceiver(SoundDevice::IMessageReceiver *receiver) = 0;
	virtual void SetCallback(SoundDevice::ICallback *callback) = 0;

	virtual SoundDevice::Info GetDeviceInfo() const = 0;

	virtual SoundDevice::Caps GetDeviceCaps() const = 0;
	virtual SoundDevice::DynamicCaps GetDeviceDynamicCaps(const std::vector<uint32> &baseSampleRates) = 0;

	virtual bool Init(const SoundDevice::AppInfo &appInfo) = 0;
	virtual bool Open(const SoundDevice::Settings &settings) = 0;
	virtual bool Close() = 0;
	virtual bool Start() = 0;
	virtual void Stop() = 0;

	virtual FlagSet<RequestFlags> GetRequestFlags() const = 0;

	virtual bool IsInited() const = 0;
	virtual bool IsOpen() const = 0;
	virtual bool IsAvailable() const = 0;
	virtual bool IsPlaying() const = 0;

	virtual bool IsPlayingSilence() const = 0;
	virtual void StopAndAvoidPlayingSilence() = 0;
	virtual void EndPlayingSilence() = 0;

	virtual bool OnIdle() = 0;  // return true if any work has been done

	virtual SoundDevice::Settings GetSettings() const = 0;
	virtual SampleFormat GetActualSampleFormat() const = 0;
	virtual SoundDevice::BufferAttributes GetEffectiveBufferAttributes() const = 0;

	virtual SoundDevice::TimeInfo GetTimeInfo() const = 0;
	virtual SoundDevice::StreamPosition GetStreamPosition() const = 0;

	// Debugging aids in case of a crash
	virtual bool DebugIsFragileDevice() const = 0;
	virtual bool DebugInRealtimeCallback() const = 0;

	// Informational only, do not use for timing.
	// Use GetStreamPositionFrames() for timing
	virtual SoundDevice::Statistics GetStatistics() const = 0;

	virtual bool OpenDriverSettings() = 0;
};


}  // namespace SoundDevice


OPENMPT_NAMESPACE_END
