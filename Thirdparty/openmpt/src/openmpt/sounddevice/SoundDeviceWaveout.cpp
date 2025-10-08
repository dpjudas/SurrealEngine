/* SPDX-License-Identifier: BSD-3-Clause */
/* SPDX-FileCopyrightText: Olivier Lapicque */
/* SPDX-FileCopyrightText: OpenMPT Project Developers and Contributors */


#include "openmpt/all/BuildSettings.hpp"
#include "openmpt/all/PlatformFixes.hpp"

#include "SoundDeviceWaveout.hpp"

#include "SoundDevice.hpp"
#include "SoundDeviceUtilities.hpp"

#include "mpt/base/detect.hpp"
#include "mpt/base/macros.hpp"
#include "mpt/base/numeric.hpp"
#include "mpt/base/saturate_round.hpp"
#include "mpt/format/message_macros.hpp"
#include "mpt/format/simple.hpp"
#include "mpt/parse/parse.hpp"
#include "mpt/string/buffer.hpp"
#include "mpt/string/types.hpp"
#include "mpt/string_transcode/transcode.hpp"
#include "openmpt/base/Types.hpp"
#include "openmpt/logging/Logger.hpp"
#include "openmpt/soundbase/SampleFormat.hpp"

#include <algorithm>
#include <array>
#include <set>
#include <vector>

#include <cstddef>

#if MPT_OS_WINDOWS
#include <windows.h>
#endif  // MPT_OS_WINDOWS


OPENMPT_NAMESPACE_BEGIN


namespace SoundDevice
{


#if MPT_OS_WINDOWS



static constexpr std::size_t WAVEOUT_MINBUFFERS = 3;
static constexpr std::size_t WAVEOUT_MAXBUFFERS = 4096;
static constexpr std::size_t WAVEOUT_MINBUFFERFRAMECOUNT = 8;
static constexpr std::size_t WAVEOUT_MAXBUFFERSIZE = 16384;  // fits in int16


static inline LONG *interlocked_access(DWORD *p)
{
	static_assert(sizeof(LONG) == sizeof(DWORD));
	return reinterpret_cast<LONG *>(p);
}


CWaveDevice::CWaveDevice(ILogger &logger, SoundDevice::Info info, SoundDevice::SysInfo sysInfo)
	: CSoundDeviceWithThread(logger, info, sysInfo)
	, m_DriverBugs(0)
{
	MPT_SOUNDDEV_TRACE_SCOPE();
	m_ThreadWakeupEvent = NULL;
	m_Failed = false;
	m_hWaveOut = NULL;
	m_nWaveBufferSize = 0;
	m_JustStarted = false;
	m_nPreparedHeaders = 0;
	m_nWriteBuffer = 0;
	m_nDoneBuffer = 0;
	m_nBuffersPending = 0;
	m_PositionLast = {};
	m_PositionWrappedCount = 0;
}


CWaveDevice::~CWaveDevice()
{
	MPT_SOUNDDEV_TRACE_SCOPE();
	Close();
}


int CWaveDevice::GetDeviceIndex() const
{
	return mpt::parse<int>(GetDeviceInternalID());
}


SoundDevice::Caps CWaveDevice::InternalGetDeviceCaps()
{
	MPT_SOUNDDEV_TRACE_SCOPE();
	SoundDevice::Caps caps;
	caps.Available = true;
	caps.CanUpdateInterval = true;
	caps.CanSampleFormat = true;
	caps.CanExclusiveMode = (GetDeviceIndex() > 0);  // no direct mode for WAVE_MAPPER, makes no sense there
	caps.CanBoostThreadPriority = true;
	caps.CanKeepDeviceRunning = false;
	caps.CanUseHardwareTiming = false;
	caps.CanChannelMapping = false;
	caps.CanInput = false;
	caps.HasNamedInputSources = false;
	caps.CanDriverPanel = false;
	caps.HasInternalDither = false;
	caps.ExclusiveModeDescription = MPT_USTRING("Use direct mode");
	if(GetSysInfo().IsWine)
	{
		caps.DefaultSettings.sampleFormat = SampleFormat::Int16;
	} else if(GetSysInfo().WindowsVersion.IsAtLeast(mpt::osinfo::windows::Version::WinVista))
	{
		caps.DefaultSettings.sampleFormat = SampleFormat::Float32;
	} else
	{
		caps.DefaultSettings.sampleFormat = SampleFormat::Int16;
	}
	return caps;
}


SoundDevice::DynamicCaps CWaveDevice::GetDeviceDynamicCaps(const std::vector<uint32> &baseSampleRates)
{
	MPT_SOUNDDEV_TRACE_SCOPE();
	SoundDevice::DynamicCaps caps;
	if(GetSysInfo().IsOriginal() && GetSysInfo().WindowsVersion.IsAtLeast(mpt::osinfo::windows::Version::WinVista))
	{  // emulated on WASAPI
		caps.supportedSampleFormats = {SampleFormat::Float32};
		caps.supportedExclusiveModeSampleFormats = {SampleFormat::Float32};
	} else
	{  // native WDM/VDX, or Wine
		caps.supportedSampleFormats = {SampleFormat::Float32, SampleFormat::Int32, SampleFormat::Int24, SampleFormat::Int16, SampleFormat::Unsigned8};
		caps.supportedExclusiveModeSampleFormats = {SampleFormat::Float32, SampleFormat::Int32, SampleFormat::Int24, SampleFormat::Int16, SampleFormat::Unsigned8};
	}
	if(GetDeviceIndex() > 0)
	{  // direct mode
		if((GetSysInfo().IsOriginal() && GetSysInfo().WindowsVersion.IsAtLeast(mpt::osinfo::windows::Version::WinVista)) || !GetSysInfo().IsOriginal())
		{  // emulated on WASAPI, or Wine
			WAVEOUTCAPS woc = {};
			caps.supportedExclusiveModeSampleFormats.clear();
			if(waveOutGetDevCaps(GetDeviceIndex() - 1, &woc, sizeof(woc)) == MMSYSERR_NOERROR)
			{
				if(woc.dwFormats & (WAVE_FORMAT_96M08 | WAVE_FORMAT_96M16 | WAVE_FORMAT_96S08 | WAVE_FORMAT_96S16))
				{
					caps.supportedExclusiveSampleRates.push_back(96000);
				}
				if(woc.dwFormats & (WAVE_FORMAT_48M08 | WAVE_FORMAT_48M16 | WAVE_FORMAT_48S08 | WAVE_FORMAT_48S16))
				{
					caps.supportedExclusiveSampleRates.push_back(48000);
				}
				if(woc.dwFormats & (WAVE_FORMAT_4M08 | WAVE_FORMAT_4M16 | WAVE_FORMAT_4S08 | WAVE_FORMAT_4S16))
				{
					caps.supportedExclusiveSampleRates.push_back(44100);
				}
				if(woc.dwFormats & (WAVE_FORMAT_2M08 | WAVE_FORMAT_2M16 | WAVE_FORMAT_2S08 | WAVE_FORMAT_2S16))
				{
					caps.supportedExclusiveSampleRates.push_back(22050);
				}
				if(woc.dwFormats & (WAVE_FORMAT_1M08 | WAVE_FORMAT_1M16 | WAVE_FORMAT_1S08 | WAVE_FORMAT_1S16))
				{
					caps.supportedExclusiveSampleRates.push_back(11025);
				}
				if(woc.dwFormats & (WAVE_FORMAT_1M08 | WAVE_FORMAT_2M08 | WAVE_FORMAT_4M08 | WAVE_FORMAT_48M08 | WAVE_FORMAT_96M08 | WAVE_FORMAT_1S08 | WAVE_FORMAT_2S08 | WAVE_FORMAT_4S08 | WAVE_FORMAT_48S08 | WAVE_FORMAT_96S08))
				{
					caps.supportedExclusiveModeSampleFormats.push_back(SampleFormat::Unsigned8);
				}
				if(woc.dwFormats & (WAVE_FORMAT_1M16 | WAVE_FORMAT_2M16 | WAVE_FORMAT_4M16 | WAVE_FORMAT_48M16 | WAVE_FORMAT_96M16 | WAVE_FORMAT_1S16 | WAVE_FORMAT_2S16 | WAVE_FORMAT_4S16 | WAVE_FORMAT_48S16 | WAVE_FORMAT_96S16))
				{
					caps.supportedExclusiveModeSampleFormats.push_back(SampleFormat::Int16);
				}
			}
		} else
		{  // native WDM/VDX
			caps.supportedExclusiveSampleRates.clear();
			caps.supportedExclusiveModeSampleFormats.clear();
			std::set<uint32> supportedSampleRates;
			std::set<SampleFormat> supportedSampleFormats;
			std::array<SampleFormat, 5> baseSampleFormats = {SampleFormat::Float32, SampleFormat::Int32, SampleFormat::Int24, SampleFormat::Int16, SampleFormat::Unsigned8};
			for(const uint32 sampleRate : baseSampleRates)
			{
				for(const SampleFormat sampleFormat : baseSampleFormats)
				{
					WAVEFORMATEXTENSIBLE wfex = {};
					Settings settings;
					settings.Samplerate = sampleRate;
					settings.Channels = 2;
					settings.sampleFormat = sampleFormat;
					if(FillWaveFormatExtensible(wfex, settings))
					{
						if(waveOutOpen(NULL, GetDeviceIndex() - 1, &wfex.Format, NULL, NULL, CALLBACK_NULL | WAVE_FORMAT_DIRECT | WAVE_FORMAT_QUERY) == MMSYSERR_NOERROR)
						{
							supportedSampleRates.insert(sampleRate);
							supportedSampleFormats.insert(sampleFormat);
						}
					}
				}
			}
			for(const uint32 sampleRate : baseSampleRates)
			{
				if(supportedSampleRates.count(sampleRate) > 0)
				{
					caps.supportedExclusiveSampleRates.push_back(sampleRate);
				}
			}
			for(const SampleFormat sampleFormat : baseSampleFormats)
			{
				if(supportedSampleFormats.count(sampleFormat) > 0)
				{
					caps.supportedExclusiveModeSampleFormats.push_back(sampleFormat);
				}
			}
		}
	}
	return caps;
}


bool CWaveDevice::InternalOpen()
{
	MPT_SOUNDDEV_TRACE_SCOPE();
	if(m_Settings.InputChannels > 0)
	{
		return false;
	}
	WAVEFORMATEXTENSIBLE wfext;
	if(!FillWaveFormatExtensible(wfext, m_Settings))
	{
		return false;
	}
	WAVEFORMATEX *pwfx = &wfext.Format;
	UINT nWaveDev = GetDeviceIndex();
	nWaveDev = (nWaveDev > 0) ? nWaveDev - 1 : WAVE_MAPPER;
	m_ThreadWakeupEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if(m_ThreadWakeupEvent == INVALID_HANDLE_VALUE)
	{
		InternalClose();
		return false;
	}
	m_Failed = false;
	m_DriverBugs = 0;
	m_hWaveOut = NULL;
	if(waveOutOpen(&m_hWaveOut, nWaveDev, pwfx, (DWORD_PTR)WaveOutCallBack, (DWORD_PTR)this, CALLBACK_FUNCTION | (m_Settings.ExclusiveMode ? WAVE_FORMAT_DIRECT : 0)) != MMSYSERR_NOERROR)
	{
		InternalClose();
		return false;
	}
	if(waveOutPause(m_hWaveOut) != MMSYSERR_NOERROR)
	{
		InternalClose();
		return false;
	}
	m_nWaveBufferSize = mpt::saturate_round<int32>(m_Settings.UpdateInterval * pwfx->nAvgBytesPerSec);
	m_nWaveBufferSize = mpt::align_up<uint32>(m_nWaveBufferSize, pwfx->nBlockAlign);
	m_nWaveBufferSize = std::clamp(m_nWaveBufferSize, static_cast<uint32>(WAVEOUT_MINBUFFERFRAMECOUNT * pwfx->nBlockAlign), static_cast<uint32>(mpt::align_down<uint32>(WAVEOUT_MAXBUFFERSIZE, pwfx->nBlockAlign)));
	std::size_t numBuffers = mpt::saturate_round<int32>(m_Settings.Latency * pwfx->nAvgBytesPerSec / m_nWaveBufferSize);
	numBuffers = std::clamp(numBuffers, WAVEOUT_MINBUFFERS, WAVEOUT_MAXBUFFERS);
	m_nPreparedHeaders = 0;
	m_WaveBuffers.resize(numBuffers);
	m_WaveBuffersData.resize(numBuffers);
	for(std::size_t buf = 0; buf < numBuffers; ++buf)
	{
		m_WaveBuffers[buf] = {};
		m_WaveBuffersData[buf].resize(m_nWaveBufferSize);
		m_WaveBuffers[buf].dwFlags = 0;
		m_WaveBuffers[buf].lpData = &m_WaveBuffersData[buf][0];
		m_WaveBuffers[buf].dwBufferLength = m_nWaveBufferSize;
		if(waveOutPrepareHeader(m_hWaveOut, &m_WaveBuffers[buf], sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
		{
			break;
		}
		m_WaveBuffers[buf].dwFlags |= WHDR_DONE;
		m_nPreparedHeaders++;
	}
	if(!m_nPreparedHeaders)
	{
		InternalClose();
		return false;
	}
	if(m_Settings.sampleFormat == SampleFormat::Int8)
	{
		m_Settings.sampleFormat = SampleFormat::Unsigned8;
	}
	m_nBuffersPending = 0;
	m_nWriteBuffer = 0;
	m_nDoneBuffer = 0;
	{
		mpt::lock_guard<mpt::mutex> guard(m_PositionWraparoundMutex);
		m_PositionLast = {};
		m_PositionWrappedCount = 0;
	}
	SetWakeupEvent(m_ThreadWakeupEvent);
	SetWakeupInterval(m_nWaveBufferSize * 1.0 / m_Settings.GetBytesPerSecond());
	m_Flags.WantsClippedOutput = (GetSysInfo().IsOriginal() && GetSysInfo().WindowsVersion.IsAtLeast(mpt::osinfo::windows::Version::WinVista));
	return true;
}


bool CWaveDevice::InternalClose()
{
	MPT_SOUNDDEV_TRACE_SCOPE();
	if(m_hWaveOut)
	{
		waveOutReset(m_hWaveOut);
		m_JustStarted = false;
		InterlockedExchange(&m_nBuffersPending, 0);
		m_nWriteBuffer = 0;
		m_nDoneBuffer = 0;
		while(m_nPreparedHeaders > 0)
		{
			m_nPreparedHeaders--;
			waveOutUnprepareHeader(m_hWaveOut, &m_WaveBuffers[m_nPreparedHeaders], sizeof(WAVEHDR));
		}
		waveOutClose(m_hWaveOut);
		m_hWaveOut = NULL;
	}
#ifdef MPT_BUILD_DEBUG
	if(m_DriverBugs.load())
	{
		SendDeviceMessage(LogError, MPT_USTRING("Errors were detected while playing sound:\n") + GetStatistics().text);
	}
#endif
	m_DriverBugs = 0;
	m_Failed = false;
	if(m_ThreadWakeupEvent)
	{
		CloseHandle(m_ThreadWakeupEvent);
		m_ThreadWakeupEvent = NULL;
	}
	{
		mpt::lock_guard<mpt::mutex> guard(m_PositionWraparoundMutex);
		m_PositionLast = {};
		m_PositionWrappedCount = 0;
	}
	return true;
}


void CWaveDevice::StartFromSoundThread()
{
	MPT_SOUNDDEV_TRACE_SCOPE();
	if(m_hWaveOut)
	{
		{
			mpt::lock_guard<mpt::mutex> guard(m_PositionWraparoundMutex);
			m_PositionLast = {};
			m_PositionWrappedCount = 0;
		}
		m_JustStarted = true;
		// Actual starting is done in InternalFillAudioBuffer to avoid crackling with tiny buffers.
	}
}


void CWaveDevice::StopFromSoundThread()
{
	MPT_SOUNDDEV_TRACE_SCOPE();
	if(m_hWaveOut)
	{
		CheckResult(waveOutPause(m_hWaveOut));
		m_JustStarted = false;
		{
			mpt::lock_guard<mpt::mutex> guard(m_PositionWraparoundMutex);
			m_PositionLast = {};
			m_PositionWrappedCount = 0;
		}
	}
}


bool CWaveDevice::CheckResult(MMRESULT result)
{
	if(result == MMSYSERR_NOERROR)
	{
		return true;
	}
	if(!m_Failed)
	{  // only show the first error
		m_Failed = true;
		TCHAR errortext[MAXERRORLENGTH + 1] = {};
		waveOutGetErrorText(result, errortext, MAXERRORLENGTH);
		SendDeviceMessage(LogError, MPT_UFORMAT_MESSAGE("WaveOut error: 0x{}: {}")(mpt::format<mpt::ustring>::hex0<8>(result), mpt::transcode<mpt::ustring>(static_cast<mpt::winstring>(mpt::ReadWinBuf(errortext)))));
	}
	RequestClose();
	return false;
}


bool CWaveDevice::CheckResult(MMRESULT result, DWORD param)
{
	if(result == MMSYSERR_NOERROR)
	{
		return true;
	}
	if(!m_Failed)
	{  // only show the first error
		m_Failed = true;
		TCHAR errortext[MAXERRORLENGTH + 1] = {};
		waveOutGetErrorText(result, errortext, MAXERRORLENGTH);
		SendDeviceMessage(LogError, MPT_UFORMAT_MESSAGE("WaveOut error: 0x{} (param 0x{}): {}")(mpt::format<mpt::ustring>::hex0<8>(result), mpt::format<mpt::ustring>::hex0<8>(param), mpt::transcode<mpt::ustring>(static_cast<mpt::winstring>(mpt::ReadWinBuf(errortext)))));
	}
	RequestClose();
	return false;
}


void CWaveDevice::InternalFillAudioBuffer()
{
	MPT_SOUNDDEV_TRACE_SCOPE();
	if(!m_hWaveOut)
	{
		return;
	}

	const std::size_t bytesPerFrame = m_Settings.GetBytesPerFrame();

	ULONG oldBuffersPending = InterlockedExchangeAdd(&m_nBuffersPending, 0);  // read
	ULONG nLatency = oldBuffersPending * m_nWaveBufferSize;

	[[maybe_unused]] ULONG nBytesWritten = 0;
	while((oldBuffersPending < m_nPreparedHeaders) && !m_Failed)
	{
#if MPT_WINNT_AT_LEAST(MPT_WIN_VISTA)
		DWORD oldFlags = InterlockedOr(interlocked_access(&m_WaveBuffers[m_nWriteBuffer].dwFlags), 0);
#else
		DWORD oldFlags = _InterlockedOr(interlocked_access(&m_WaveBuffers[m_nWriteBuffer].dwFlags), 0);
#endif
		uint32 driverBugs = 0;
		if(oldFlags & WHDR_INQUEUE)
		{
			driverBugs |= DriverBugBufferFillAndHeaderInQueue;
		}
		if(!(oldFlags & WHDR_DONE))
		{
			driverBugs |= DriverBugBufferFillAndHeaderNotDone;
		}
		driverBugs |= m_DriverBugs.fetch_or(driverBugs);
		if(oldFlags & WHDR_INQUEUE)
		{
			if(driverBugs & DriverBugDoneNotificationOutOfOrder)
			{
				//  Some drivers/setups can return WaveHeader notifications out of
				// order. WaveHeaders which have not yet been notified to be ready stay
				// in the INQUEUE and !DONE state internally and cannot be reused yet
				// even though they causally should be able to. waveOutWrite fails for
				// them.
				//  In this case we skip filling the buffers until we actually see the
				// next expected buffer to be ready for refilling.
				//  This problem has been spotted on Wine 1.7.46 (non-official packages)
				// running on Debian 8 Jessie 32bit. It may also be related to WaveOut
				// playback being too fast and crackling which had benn reported on
				// Wine 1.6 + WinePulse on UbuntuStudio 12.04 32bit (this has not been
				// verified yet because the problem is not always reproducable on the
				// system in question).
				return;
			}
		}
		nLatency += m_nWaveBufferSize;
		CallbackLockedAudioReadPrepare(m_nWaveBufferSize / bytesPerFrame, nLatency / bytesPerFrame);
		CallbackLockedAudioProcessVoid(m_WaveBuffers[m_nWriteBuffer].lpData, nullptr, m_nWaveBufferSize / bytesPerFrame);
		nBytesWritten += m_nWaveBufferSize;
#if MPT_WINNT_AT_LEAST(MPT_WIN_VISTA)
		InterlockedAnd(interlocked_access(&m_WaveBuffers[m_nWriteBuffer].dwFlags), ~static_cast<DWORD>(WHDR_INQUEUE | WHDR_DONE));
#else
		_InterlockedAnd(interlocked_access(&m_WaveBuffers[m_nWriteBuffer].dwFlags), ~static_cast<DWORD>(WHDR_INQUEUE | WHDR_DONE));
#endif
		InterlockedExchange(interlocked_access(&m_WaveBuffers[m_nWriteBuffer].dwBufferLength), m_nWaveBufferSize);
		InterlockedIncrement(&m_nBuffersPending);
		oldBuffersPending++;  // increment separately to avoid looping without leaving at all when rendering takes more than 100% CPU
		CheckResult(waveOutWrite(m_hWaveOut, &m_WaveBuffers[m_nWriteBuffer], sizeof(WAVEHDR)), oldFlags);
		m_nWriteBuffer++;
		m_nWriteBuffer %= m_nPreparedHeaders;
		CallbackLockedAudioProcessDone();
	}

	if(m_JustStarted && !m_Failed)
	{
		// Fill the buffers completely before starting the stream.
		// This avoids buffer underruns which result in audible crackling with small buffers.
		m_JustStarted = false;
		CheckResult(waveOutRestart(m_hWaveOut));
	}
}


int64 CWaveDevice::InternalGetStreamPositionFrames() const
{
	// Apparently, at least with Windows XP, TIME_SAMPLES wraps aroud at 0x7FFFFFF (see
	// http://www.tech-archive.net/Archive/Development/microsoft.public.win32.programmer.mmedia/2005-02/0070.html
	// ).
	// We may also, additionally, default to TIME_BYTES which would wraparound the earliest.
	// We could thereby try to avoid any potential wraparound inside the driver on older
	// Windows versions, which would be, once converted into other units, really
	// difficult to detect or handle.
	static constexpr UINT timeType = TIME_SAMPLES;  // should work for sane systems
	//static constexpr std::size_t valid_bits = 32; // should work for sane systems
	//static constexpr UINT timeType = TIME_BYTES; // safest
	static constexpr std::size_t valid_bits = 27;  // safe for WinXP TIME_SAMPLES
	static constexpr uint32 valid_mask = static_cast<uint32>((uint64(1) << valid_bits) - 1u);
	static constexpr uint32 valid_watermark = static_cast<uint32>(uint64(1) << (valid_bits - 1u));  // half the valid range in order to be able to catch backwards fluctuations

	MMTIME mmtime = {};
	mmtime.wType = timeType;
	if(waveOutGetPosition(m_hWaveOut, &mmtime, sizeof(mmtime)) != MMSYSERR_NOERROR)
	{
		return 0;
	}
	if(mmtime.wType != TIME_MS && mmtime.wType != TIME_BYTES && mmtime.wType != TIME_SAMPLES)
	{  // unsupported time format
		return 0;
	}
	int64 offset = 0;
	{
		// handle wraparound
		mpt::lock_guard<mpt::mutex> guard(m_PositionWraparoundMutex);
		if(!m_PositionLast.wType)
		{
			// first call
			m_PositionWrappedCount = 0;
		} else if(mmtime.wType != m_PositionLast.wType)
		{
			// what? value type changed, do not try handling that for now.
			m_PositionWrappedCount = 0;
		} else
		{
			DWORD oldval = 0;
			DWORD curval = 0;
			switch(mmtime.wType)
			{
				case TIME_MS:
					oldval = m_PositionLast.u.ms;
					curval = mmtime.u.ms;
					break;
				case TIME_BYTES:
					oldval = m_PositionLast.u.cb;
					curval = mmtime.u.cb;
					break;
				case TIME_SAMPLES:
					oldval = m_PositionLast.u.sample;
					curval = mmtime.u.sample;
					break;
			}
			oldval &= valid_mask;
			curval &= valid_mask;
			if(((curval - oldval) & valid_mask) >= valid_watermark)  // guard against driver problems resulting in time jumping backwards for short periods of time. BEWARE of integer wraparound when refactoring
			{
				curval = oldval;
			}
			switch(mmtime.wType)
			{
				case TIME_MS: mmtime.u.ms = curval; break;
				case TIME_BYTES: mmtime.u.cb = curval; break;
				case TIME_SAMPLES: mmtime.u.sample = curval; break;
			}
			if((curval ^ oldval) & valid_watermark)  // MSB flipped
			{
				if(!(curval & valid_watermark))  // actually wrapped
				{
					m_PositionWrappedCount += 1;
				}
			}
		}
		m_PositionLast = mmtime;
		offset = (static_cast<uint64>(m_PositionWrappedCount) << valid_bits);
	}
	int64 result = 0;
	switch(mmtime.wType)
	{
		case TIME_MS: result += (static_cast<int64>(mmtime.u.ms & valid_mask) + offset) * m_Settings.GetBytesPerSecond() / (1000 * m_Settings.GetBytesPerFrame()); break;
		case TIME_BYTES: result += (static_cast<int64>(mmtime.u.cb & valid_mask) + offset) / m_Settings.GetBytesPerFrame(); break;
		case TIME_SAMPLES: result += (static_cast<int64>(mmtime.u.sample & valid_mask) + offset); break;
	}
	return result;
}


void CWaveDevice::HandleWaveoutDone(WAVEHDR *hdr)
{
	MPT_SOUNDDEV_TRACE_SCOPE();
#if MPT_WINNT_AT_LEAST(MPT_WIN_VISTA)
	DWORD flags = InterlockedOr(interlocked_access(&hdr->dwFlags), 0);
#else
	DWORD flags = _InterlockedOr(interlocked_access(&hdr->dwFlags), 0);
#endif
	std::size_t hdrIndex = hdr - &(m_WaveBuffers[0]);
	uint32 driverBugs = 0;
	if(hdrIndex != m_nDoneBuffer)
	{
		driverBugs |= DriverBugDoneNotificationOutOfOrder;
	}
	if(!(flags & WHDR_DONE))
	{
		driverBugs |= DriverBugDoneNotificationAndHeaderNotDone;
	}
	if(flags & WHDR_INQUEUE)
	{
		driverBugs |= DriverBugDoneNotificationAndHeaderInQueue;
	}
	if(driverBugs)
	{
		m_DriverBugs.fetch_or(driverBugs);
	}
	m_nDoneBuffer += 1;
	m_nDoneBuffer %= m_nPreparedHeaders;
	InterlockedDecrement(&m_nBuffersPending);
	SetEvent(m_ThreadWakeupEvent);
}


void CWaveDevice::WaveOutCallBack(HWAVEOUT, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR param1, DWORD_PTR /* param2 */)
{
	MPT_SOUNDDEV_TRACE_SCOPE();
	if((uMsg == WOM_DONE) && (dwUser))
	{
		CWaveDevice *that = (CWaveDevice *)dwUser;
		that->HandleWaveoutDone((WAVEHDR *)param1);
	}
}


SoundDevice::BufferAttributes CWaveDevice::InternalGetEffectiveBufferAttributes() const
{
	SoundDevice::BufferAttributes bufferAttributes;
	bufferAttributes.Latency = m_nWaveBufferSize * m_nPreparedHeaders * 1.0 / m_Settings.GetBytesPerSecond();
	bufferAttributes.UpdateInterval = m_nWaveBufferSize * 1.0 / m_Settings.GetBytesPerSecond();
	bufferAttributes.NumBuffers = m_nPreparedHeaders;
	return bufferAttributes;
}


SoundDevice::Statistics CWaveDevice::GetStatistics() const
{
	MPT_SOUNDDEV_TRACE_SCOPE();
	SoundDevice::Statistics result;
	result.InstantaneousLatency = InterlockedExchangeAdd(&m_nBuffersPending, 0) * m_nWaveBufferSize * 1.0 / m_Settings.GetBytesPerSecond();
	result.LastUpdateInterval = 1.0 * m_nWaveBufferSize / m_Settings.GetBytesPerSecond();
	uint32 bugs = m_DriverBugs.load();
	if(bugs != 0)
	{
		result.text = MPT_UFORMAT_MESSAGE("Problematic driver detected! Error flags: {}")(mpt::format<mpt::ustring>::hex0<8>(bugs));
	} else
	{
		result.text = MPT_UFORMAT_MESSAGE("Driver working as expected.")();
	}
	return result;
}


std::vector<SoundDevice::Info> CWaveDevice::EnumerateDevices(ILogger &logger, SoundDevice::SysInfo sysInfo)
{
#if 0
	auto GetLogger = [&]() -> ILogger &
	{
		return logger;
	};
#else
	MPT_UNUSED(logger);
#endif
	MPT_SOUNDDEV_TRACE_SCOPE();
	std::vector<SoundDevice::Info> devices;
	UINT numDevs = waveOutGetNumDevs();
	for(UINT index = 0; index <= numDevs; ++index)
	{
		SoundDevice::Info info;
		info.type = TypeWAVEOUT;
		info.internalID = mpt::format<mpt::ustring>::dec(index);
		info.apiName = MPT_USTRING("MME");
		info.useNameAsIdentifier = true;
		WAVEOUTCAPS woc = {};
		if(waveOutGetDevCaps((index == 0) ? WAVE_MAPPER : (index - 1), &woc, sizeof(woc)) == MMSYSERR_NOERROR)
		{
			info.name = mpt::transcode<mpt::ustring>(static_cast<mpt::winstring>(mpt::ReadWinBuf(woc.szPname)));
			info.extraData[MPT_USTRING("DriverID")] = MPT_UFORMAT_MESSAGE("{}:{}")(mpt::format<mpt::ustring>::hex0<4>(woc.wMid), mpt::format<mpt::ustring>::hex0<4>(woc.wPid));
			info.extraData[MPT_USTRING("DriverVersion")] = MPT_UFORMAT_MESSAGE("{}.{}")(mpt::format<mpt::ustring>::dec((static_cast<uint32>(woc.vDriverVersion) >> 24) & 0xff), mpt::format<mpt::ustring>::dec((static_cast<uint32>(woc.vDriverVersion) >> 0) & 0xff));
		}
		if(info.name.empty())
		{
			if(index == 0)
			{
				info.name = MPT_UFORMAT_MESSAGE("Auto (Wave Mapper)")();
			} else
			{
				info.name = MPT_UFORMAT_MESSAGE("Device {}")(index - 1);
			}
		}
		info.default_ = ((index == 0) ? Info::Default::Managed : Info::Default::None);
		// clang-format off
		info.flags = {
			sysInfo.SystemClass == mpt::osinfo::osclass::Windows ? sysInfo.IsWindowsOriginal() && sysInfo.WindowsVersion.IsBefore(mpt::osinfo::windows::Version::Win7) ? Info::Usability::Usable : Info::Usability::Legacy : Info::Usability::NotAvailable,
			Info::Level::Primary,
			sysInfo.SystemClass == mpt::osinfo::osclass::Windows && sysInfo.IsWindowsOriginal() ? Info::Compatible::Yes : Info::Compatible::No,
			sysInfo.SystemClass == mpt::osinfo::osclass::Windows ? sysInfo.IsWindowsWine() ? Info::Api::Emulated : sysInfo.WindowsVersion.IsAtLeast(mpt::osinfo::windows::Version::WinVista) ? Info::Api::Emulated : Info::Api::Native : Info::Api::Emulated,
			Info::Io::OutputOnly,
			Info::Mixing::Software,
			Info::Implementor::OpenMPT
		};
		// clang-format on
		devices.push_back(info);
	}
	return devices;
}

#endif  // MPT_OS_WINDOWS


}  // namespace SoundDevice


OPENMPT_NAMESPACE_END
