/* SPDX-License-Identifier: BSD-3-Clause */
/* SPDX-FileCopyrightText: Olivier Lapicque */
/* SPDX-FileCopyrightText: OpenMPT Project Developers and Contributors */


#include "openmpt/all/BuildSettings.hpp"
#include "openmpt/all/PlatformFixes.hpp"

#include "SoundDeviceUtilities.hpp"

#include "SoundDevice.hpp"

#include "mpt/base/detect.hpp"
#include "mpt/base/macros.hpp"
#include "mpt/format/message_macros.hpp"
#include "mpt/out_of_memory/out_of_memory.hpp"
#include "mpt/string/types.hpp"
#include "mpt/string_transcode/transcode.hpp"
#include "openmpt/base/Types.hpp"
#include "openmpt/logging/Logger.hpp"

#include <thread>
#include <utility>

#include <cassert>

#if MPT_OS_WINDOWS
#if MPT_WINNT_AT_LEAST(MPT_WIN_VISTA)
#include <avrt.h>
#endif
#include <mmsystem.h>
#include <windows.h>
#endif  // MPT_OS_WINDOWS

#if !MPT_OS_WINDOWS
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#ifdef _POSIX_PRIORITY_SCHEDULING  // from unistd.h
#include <sched.h>
#endif
#endif

#if defined(MPT_WITH_DBUS)
#include <dbus/dbus.h>
#endif
#if defined(MPT_WITH_RTKIT)
#include "rtkit/rtkit.h"
#endif


OPENMPT_NAMESPACE_BEGIN


namespace SoundDevice
{


#if MPT_OS_WINDOWS

bool FillWaveFormatExtensible(WAVEFORMATEXTENSIBLE &WaveFormat, const SoundDevice::Settings &m_Settings)
{
	WaveFormat = {};
	WaveFormat.Format.wFormatTag = m_Settings.sampleFormat.IsFloat() ? WAVE_FORMAT_IEEE_FLOAT : WAVE_FORMAT_PCM;
	WaveFormat.Format.nChannels = (WORD)m_Settings.Channels;
	WaveFormat.Format.nSamplesPerSec = m_Settings.Samplerate;
	WaveFormat.Format.nAvgBytesPerSec = (DWORD)m_Settings.GetBytesPerSecond();
	WaveFormat.Format.nBlockAlign = (WORD)m_Settings.GetBytesPerFrame();
	WaveFormat.Format.wBitsPerSample = (WORD)m_Settings.sampleFormat.GetBitsPerSample();
	WaveFormat.Format.cbSize = 0;
	if((WaveFormat.Format.wBitsPerSample > 16 && m_Settings.sampleFormat.IsInt()) || (WaveFormat.Format.nChannels > 2))
	{
		WaveFormat.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
		WaveFormat.Format.cbSize = sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX);
		WaveFormat.Samples.wValidBitsPerSample = WaveFormat.Format.wBitsPerSample;
		switch(WaveFormat.Format.nChannels)
		{
			case 1: WaveFormat.dwChannelMask = SPEAKER_FRONT_CENTER; break;
			case 2: WaveFormat.dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT; break;
			case 3: WaveFormat.dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_BACK_CENTER; break;
			case 4: WaveFormat.dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT; break;
			default:
				WaveFormat.dwChannelMask = 0;
				return false;
				break;
		}
		const GUID guid_MEDIASUBTYPE_PCM = {
			0x00000001, 0x0000, 0x0010, {0x80, 0x00, 0x0, 0xAA, 0x0, 0x38, 0x9B, 0x71}
        };
		const GUID guid_MEDIASUBTYPE_IEEE_FLOAT = {
			0x00000003, 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71}
        };
		WaveFormat.SubFormat = m_Settings.sampleFormat.IsFloat() ? guid_MEDIASUBTYPE_IEEE_FLOAT : guid_MEDIASUBTYPE_PCM;
	}
	return true;
}

#endif  // MPT_OS_WINDOWS


#if MPT_OS_WINDOWS

CAudioThread::CAudioThread(CSoundDeviceWithThread &SoundDevice)
	: m_SoundDevice(SoundDevice)
{
	MPT_SOUNDDEV_TRACE_SCOPE();
	m_MMCSSClass = mpt::transcode<mpt::winstring>(m_SoundDevice.m_AppInfo.BoostedThreadMMCSSClassVista);
	m_WakeupInterval = 0.0;
	m_hPlayThread = NULL;
	m_dwPlayThreadId = 0;
	m_hAudioWakeUp = NULL;
	m_hAudioThreadTerminateRequest = NULL;
	m_hAudioThreadGoneIdle = NULL;
	m_hHardwareWakeupEvent = INVALID_HANDLE_VALUE;
	m_AudioThreadActive = 0;
	m_hAudioWakeUp = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_hAudioThreadTerminateRequest = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_hAudioThreadGoneIdle = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hPlayThread = CreateThread(NULL, 0, AudioThreadWrapper, (LPVOID)this, 0, &m_dwPlayThreadId);
}


CAudioThread::~CAudioThread()
{
	MPT_SOUNDDEV_TRACE_SCOPE();
	if(m_hPlayThread != NULL)
	{
		SetEvent(m_hAudioThreadTerminateRequest);
		WaitForSingleObject(m_hPlayThread, INFINITE);
		m_dwPlayThreadId = 0;
		m_hPlayThread = NULL;
	}
	if(m_hAudioThreadTerminateRequest)
	{
		CloseHandle(m_hAudioThreadTerminateRequest);
		m_hAudioThreadTerminateRequest = 0;
	}
	if(m_hAudioThreadGoneIdle != NULL)
	{
		CloseHandle(m_hAudioThreadGoneIdle);
		m_hAudioThreadGoneIdle = 0;
	}
	if(m_hAudioWakeUp != NULL)
	{
		CloseHandle(m_hAudioWakeUp);
		m_hAudioWakeUp = NULL;
	}
}


CPriorityBooster::CPriorityBooster(SoundDevice::SysInfo sysInfo, bool boostPriority, const mpt::winstring &priorityClass, int priority)
	: m_SysInfo(sysInfo)
	, m_BoostPriority(boostPriority)
#if MPT_WINNT_AT_LEAST(MPT_WIN_VISTA)
	, task_idx(0)
	, hTask(NULL)
#else  // < Vista
	, m_Priority(priority)
	, oldPriority(0)
#endif
{
	MPT_SOUNDDEV_TRACE_SCOPE();
#ifdef MPT_BUILD_DEBUG
	m_BoostPriority = false;
#endif
	if(m_BoostPriority)
	{
#if MPT_WINNT_AT_LEAST(MPT_WIN_VISTA)
		if(!priorityClass.empty())
		{
			hTask = AvSetMmThreadCharacteristics(priorityClass.c_str(), &task_idx);
		}
		MPT_UNUSED(priority);
#else  // < Vista
		oldPriority = GetThreadPriority(GetCurrentThread());
		SetThreadPriority(GetCurrentThread(), m_Priority);
		MPT_UNUSED(priorityClass);
#endif
	}
}


CPriorityBooster::~CPriorityBooster()
{
	MPT_SOUNDDEV_TRACE_SCOPE();
	if(m_BoostPriority)
	{
#if MPT_WINNT_AT_LEAST(MPT_WIN_VISTA)
		if(hTask)
		{
			AvRevertMmThreadCharacteristics(hTask);
		}
		hTask = NULL;
		task_idx = 0;
#else  // < Vista
		SetThreadPriority(GetCurrentThread(), oldPriority);
#endif
	}
}


class CPeriodicWaker
{
private:
	double sleepSeconds;
	long sleepMilliseconds;
	int64 sleep100Nanoseconds;

	bool periodic_nt_timer;

	HANDLE sleepEvent;

public:
	explicit CPeriodicWaker(double sleepSeconds_)
		: sleepSeconds(sleepSeconds_)
	{

		MPT_SOUNDDEV_TRACE_SCOPE();

		sleepMilliseconds = static_cast<long>(sleepSeconds * 1000.0);
		sleep100Nanoseconds = static_cast<int64>(sleepSeconds * 10000000.0);
		if(sleepMilliseconds < 1) sleepMilliseconds = 1;
		if(sleep100Nanoseconds < 1) sleep100Nanoseconds = 1;

		periodic_nt_timer = (sleep100Nanoseconds >= 10000);  // can be represented as a millisecond period, otherwise use non-periodic timers which allow higher precision but might me slower because we have to set them again in each period

		sleepEvent = NULL;

		if(periodic_nt_timer)
		{
			sleepEvent = CreateWaitableTimer(NULL, FALSE, NULL);
			if(!sleepEvent)
			{
				mpt::throw_out_of_memory();
			}
			LARGE_INTEGER dueTime;
			dueTime.QuadPart = 0 - sleep100Nanoseconds;  // negative time means relative
			SetWaitableTimer(sleepEvent, &dueTime, sleepMilliseconds, NULL, NULL, FALSE);
		} else
		{
			sleepEvent = CreateWaitableTimer(NULL, TRUE, NULL);
			if(!sleepEvent)
			{
				mpt::throw_out_of_memory();
			}
		}
	}

	CPeriodicWaker(const CPeriodicWaker &) = delete;
	CPeriodicWaker &operator=(const CPeriodicWaker &) = delete;

	long GetSleepMilliseconds() const
	{
		return sleepMilliseconds;
	}

	HANDLE GetWakeupEvent() const
	{
		return sleepEvent;
	}

	void Retrigger()
	{
		MPT_SOUNDDEV_TRACE_SCOPE();
		if(!periodic_nt_timer)
		{
			LARGE_INTEGER dueTime;
			dueTime.QuadPart = 0 - sleep100Nanoseconds;  // negative time means relative
			SetWaitableTimer(sleepEvent, &dueTime, 0, NULL, NULL, FALSE);
		}
	}

	~CPeriodicWaker()
	{
		MPT_SOUNDDEV_TRACE_SCOPE();
		if(periodic_nt_timer)
		{
			CancelWaitableTimer(sleepEvent);
		}
		CloseHandle(sleepEvent);
		sleepEvent = NULL;
	}
};


DWORD WINAPI CAudioThread::AudioThreadWrapper(LPVOID user)
{
	return ((CAudioThread *)user)->AudioThread();
}
DWORD CAudioThread::AudioThread()
{
	MPT_SOUNDDEV_TRACE_SCOPE();

	bool terminate = false;
	while(!terminate)
	{

		bool idle = true;
		while(!terminate && idle)
		{
			HANDLE waithandles[2] = {m_hAudioThreadTerminateRequest, m_hAudioWakeUp};
			SetEvent(m_hAudioThreadGoneIdle);
			switch(WaitForMultipleObjects(2, waithandles, FALSE, INFINITE))
			{
				case WAIT_OBJECT_0:
					terminate = true;
					break;
				case WAIT_OBJECT_0 + 1:
					idle = false;
					break;
			}
		}

		if(!terminate)
		{

			CPriorityBooster priorityBooster(m_SoundDevice.GetSysInfo(), m_SoundDevice.m_Settings.BoostThreadPriority, m_MMCSSClass, m_SoundDevice.m_AppInfo.BoostedThreadPriorityXP);
			CPeriodicWaker periodicWaker(m_WakeupInterval);

			m_SoundDevice.StartFromSoundThread();

			while(!terminate && IsActive())
			{

				m_SoundDevice.FillAudioBufferLocked();

				periodicWaker.Retrigger();

				if(m_hHardwareWakeupEvent != INVALID_HANDLE_VALUE)
				{
					HANDLE waithandles[4] = {m_hAudioThreadTerminateRequest, m_hAudioWakeUp, m_hHardwareWakeupEvent, periodicWaker.GetWakeupEvent()};
					switch(WaitForMultipleObjects(4, waithandles, FALSE, periodicWaker.GetSleepMilliseconds()))
					{
						case WAIT_OBJECT_0:
							terminate = true;
							break;
					}
				} else
				{
					HANDLE waithandles[3] = {m_hAudioThreadTerminateRequest, m_hAudioWakeUp, periodicWaker.GetWakeupEvent()};
					switch(WaitForMultipleObjects(3, waithandles, FALSE, periodicWaker.GetSleepMilliseconds()))
					{
						case WAIT_OBJECT_0:
							terminate = true;
							break;
					}
				}
			}

			m_SoundDevice.StopFromSoundThread();
		}
	}

	SetEvent(m_hAudioThreadGoneIdle);

	return 0;
}


void CAudioThread::SetWakeupEvent(HANDLE ev)
{
	MPT_SOUNDDEV_TRACE_SCOPE();
	m_hHardwareWakeupEvent = ev;
}


void CAudioThread::SetWakeupInterval(double seconds)
{
	MPT_SOUNDDEV_TRACE_SCOPE();
	m_WakeupInterval = seconds;
}


bool CAudioThread::IsActive()
{
	return InterlockedExchangeAdd(&m_AudioThreadActive, 0) ? true : false;
}


void CAudioThread::Activate()
{
	MPT_SOUNDDEV_TRACE_SCOPE();
	if(InterlockedExchangeAdd(&m_AudioThreadActive, 0))
	{
		assert(false);
		return;
	}
	ResetEvent(m_hAudioThreadGoneIdle);
	InterlockedExchange(&m_AudioThreadActive, 1);
	SetEvent(m_hAudioWakeUp);
}


void CAudioThread::Deactivate()
{
	MPT_SOUNDDEV_TRACE_SCOPE();
	if(!InterlockedExchangeAdd(&m_AudioThreadActive, 0))
	{
		assert(false);
		return;
	}
	InterlockedExchange(&m_AudioThreadActive, 0);
	WaitForSingleObject(m_hAudioThreadGoneIdle, INFINITE);
}


CSoundDeviceWithThread::CSoundDeviceWithThread(ILogger &logger, SoundDevice::Info info, SoundDevice::SysInfo sysInfo)
	: SoundDevice::Base(logger, info, sysInfo)
	, m_AudioThread(*this)
{
	return;
}


CSoundDeviceWithThread::~CSoundDeviceWithThread()
{
	return;
}


void CSoundDeviceWithThread::FillAudioBufferLocked()
{
	MPT_SOUNDDEV_TRACE_SCOPE();
	CallbackFillAudioBufferLocked();
}


void CSoundDeviceWithThread::SetWakeupEvent(HANDLE ev)
{
	MPT_SOUNDDEV_TRACE_SCOPE();
	m_AudioThread.SetWakeupEvent(ev);
}


void CSoundDeviceWithThread::SetWakeupInterval(double seconds)
{
	MPT_SOUNDDEV_TRACE_SCOPE();
	m_AudioThread.SetWakeupInterval(seconds);
}


bool CSoundDeviceWithThread::InternalStart()
{
	MPT_SOUNDDEV_TRACE_SCOPE();
	m_AudioThread.Activate();
	return true;
}


void CSoundDeviceWithThread::InternalStop()
{
	MPT_SOUNDDEV_TRACE_SCOPE();
	m_AudioThread.Deactivate();
}

#endif  // MPT_OS_WINDOWS


#if MPT_OS_LINUX || MPT_OS_MACOSX_OR_IOS || MPT_OS_FREEBSD


#if MPT_COMPILER_CLANG
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-private-field"
#endif  // MPT_COMPILER_CLANG

class ThreadPriorityGuardImpl
{

private:
	ILogger &m_Logger;
	bool active;
	bool successfull;
	bool realtime;
	int niceness;
	int rt_priority;
#if defined(MPT_WITH_DBUS) && defined(MPT_WITH_RTKIT)
	DBusConnection *bus;
#endif  // MPT_WITH_DBUS && MPT_WITH_RTKIT

private:
	ILogger &GetLogger() const
	{
		return m_Logger;
	}

public:
	ThreadPriorityGuardImpl(ILogger &logger, bool active, bool realtime, int niceness, int rt_priority)
		: m_Logger(logger)
		, active(active)
		, successfull(false)
		, realtime(realtime)
		, niceness(niceness)
		, rt_priority(rt_priority)
#if defined(MPT_WITH_DBUS) && defined(MPT_WITH_RTKIT)
		, bus(NULL)
#endif  // MPT_WITH_DBUS && MPT_WITH_RTKIT
	{
		if(active)
		{
			if(realtime)
			{
#ifdef _POSIX_PRIORITY_SCHEDULING
				sched_param p = sched_param{};
				p.sched_priority = rt_priority;
#if MPT_OS_LINUX
				if(sched_setscheduler(0, SCHED_RR | SCHED_RESET_ON_FORK, &p) == 0)
				{
					successfull = true;
				} else
				{
#if defined(MPT_WITH_DBUS) && defined(MPT_WITH_RTKIT)
					MPT_LOG(GetLogger(), LogNotification, "sounddev", MPT_UFORMAT_MESSAGE("sched_setscheduler: {}")(errno));
#else
					MPT_LOG(GetLogger(), LogError, "sounddev", MPT_UFORMAT_MESSAGE("sched_setscheduler: {}")(errno));
#endif
				}
#else
				if(sched_setscheduler(0, SCHED_RR, &p) == 0)
				{
					successfull = true;
				} else
				{
#if defined(MPT_WITH_DBUS) && defined(MPT_WITH_RTKIT)
					MPT_LOG(GetLogger(), LogNotification, "sounddev", MPT_UFORMAT_MESSAGE("sched_setscheduler: {}")(errno));
#else
					MPT_LOG(GetLogger(), LogError, "sounddev", MPT_UFORMAT_MESSAGE("sched_setscheduler: {}")(errno));
#endif
				}
#endif
#endif
			} else
			{
				if(setpriority(PRIO_PROCESS, 0, niceness) == 0)
				{
					successfull = true;
				} else
				{
#if defined(MPT_WITH_DBUS) && defined(MPT_WITH_RTKIT)
					MPT_LOG(GetLogger(), LogNotification, "sounddev", MPT_UFORMAT_MESSAGE("setpriority: {}")(errno));
#else
					MPT_LOG(GetLogger(), LogError, "sounddev", MPT_UFORMAT_MESSAGE("setpriority: {}")(errno));
#endif
				}
			}
			if(!successfull)
			{
#if defined(MPT_WITH_DBUS) && defined(MPT_WITH_RTKIT)
				DBusError error;
				dbus_error_init(&error);
				bus = dbus_bus_get(DBUS_BUS_SYSTEM, &error);
				if(!bus)
				{
					MPT_LOG(GetLogger(), LogError, "sounddev", MPT_UFORMAT_MESSAGE("DBus: dbus_bus_get: {}")(mpt::transcode<mpt::ustring>(mpt::common_encoding::utf8, error.message)));
				}
				dbus_error_free(&error);
				if(bus)
				{
					if(realtime)
					{
						int e = rtkit_make_realtime(bus, 0, rt_priority);
						if(e != 0)
						{
							MPT_LOG(GetLogger(), LogError, "sounddev", MPT_UFORMAT_MESSAGE("RtKit: rtkit_make_realtime: {}")(e));
						} else
						{
							successfull = true;
						}
					} else
					{
						int e = rtkit_make_high_priority(bus, 0, niceness);
						if(e != 0)
						{
							MPT_LOG(GetLogger(), LogError, "sounddev", MPT_UFORMAT_MESSAGE("RtKit: rtkit_make_high_priority: {}")(e));
						} else
						{
							successfull = true;
						}
					}
				}
#endif  // MPT_WITH_DBUS && MPT_WITH_RTKIT
			}
		}
	}

	~ThreadPriorityGuardImpl()
	{
		if(active)
		{
#if defined(MPT_WITH_DBUS) && defined(MPT_WITH_RTKIT)
			if(bus)
			{
				// TODO: Do we want to reset priorities here?
				dbus_connection_unref(bus);
				bus = NULL;
			}
#endif  // MPT_WITH_DBUS && MPT_WITH_RTKIT
		}
	}
};

#if MPT_COMPILER_CLANG
#pragma clang diagnostic pop
#endif  // MPT_COMPILER_CLANG



ThreadPriorityGuard::ThreadPriorityGuard(ILogger &logger, bool active, bool realtime, int niceness, int rt_priority)
	: impl(std::make_unique<ThreadPriorityGuardImpl>(logger, active, realtime, niceness, rt_priority))
{
	return;
}


ThreadPriorityGuard::~ThreadPriorityGuard()
{
	return;
}


ThreadBase::ThreadBase(ILogger &logger, SoundDevice::Info info, SoundDevice::SysInfo sysInfo)
	: Base(logger, info, sysInfo)
	, m_ThreadStopRequest(false)
{
	return;
}

bool ThreadBase::InternalStart()
{
	m_ThreadStopRequest.store(false);
	m_Thread = std::thread(&ThreadProcStatic, this);
	m_ThreadStarted.wait();
	m_ThreadStarted.post();
	return true;
}

void ThreadBase::ThreadProcStatic(ThreadBase *this_)
{
	this_->ThreadProc();
}

void ThreadBase::ThreadProc()
{
	ThreadPriorityGuard priorityGuard(GetLogger(), m_Settings.BoostThreadPriority, m_AppInfo.BoostedThreadRealtimePosix, m_AppInfo.BoostedThreadNicenessPosix, m_AppInfo.BoostedThreadRealtimePosix);
	m_ThreadStarted.post();
	InternalStartFromSoundThread();
	while(!m_ThreadStopRequest.load())
	{
		CallbackFillAudioBufferLocked();
		InternalWaitFromSoundThread();
	}
	InternalStopFromSoundThread();
}

void ThreadBase::InternalStop()
{
	m_ThreadStopRequest.store(true);
	m_Thread.join();
	m_Thread = std::thread();
	m_ThreadStopRequest.store(false);
}

ThreadBase::~ThreadBase()
{
	return;
}


#endif  // MPT_OS_LINUX || MPT_OS_MACOSX_OR_IOS || MPT_OS_FREEBSD


}  // namespace SoundDevice


OPENMPT_NAMESPACE_END
