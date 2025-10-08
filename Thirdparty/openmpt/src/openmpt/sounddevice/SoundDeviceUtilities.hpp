/* SPDX-License-Identifier: BSD-3-Clause */
/* SPDX-FileCopyrightText: Olivier Lapicque */
/* SPDX-FileCopyrightText: OpenMPT Project Developers and Contributors */


#pragma once

#include "openmpt/all/BuildSettings.hpp"

#include "SoundDevice.hpp"
#include "SoundDeviceBase.hpp"

#include "mpt/base/detect.hpp"
#include "openmpt/logging/Logger.hpp"

#if MPT_OS_LINUX || MPT_OS_MACOSX_OR_IOS || MPT_OS_FREEBSD
// we use c++11 in native support library
#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>
#endif  // MPT_OS_LINUX || MPT_OS_MACOSX_OR_IOS || MPT_OS_FREEBSD

#if MPT_OS_WINDOWS
#include <mmreg.h>
#include <windows.h>
#endif  // MPT_OS_WINDOWS


OPENMPT_NAMESPACE_BEGIN


namespace SoundDevice
{


#if MPT_OS_WINDOWS
bool FillWaveFormatExtensible(WAVEFORMATEXTENSIBLE &WaveFormat, const SoundDevice::Settings &m_Settings);
#endif  // MPT_OS_WINDOWS


#if MPT_OS_WINDOWS


class CSoundDeviceWithThread;


class CPriorityBooster
{
private:
	SoundDevice::SysInfo m_SysInfo;
	bool m_BoostPriority;
#if MPT_WINNT_AT_LEAST(MPT_WIN_VISTA)
	DWORD task_idx;
	HANDLE hTask;
#else  // < Vista
	int m_Priority;
	int oldPriority;
#endif

public:
	CPriorityBooster(SoundDevice::SysInfo sysInfo, bool boostPriority, const mpt::winstring &priorityClass, int priority);
	~CPriorityBooster();
};


class CAudioThread
{
	friend class CPeriodicWaker;

private:
	CSoundDeviceWithThread &m_SoundDevice;
	mpt::winstring m_MMCSSClass;
	double m_WakeupInterval;
	HANDLE m_hAudioWakeUp;
	HANDLE m_hPlayThread;
	HANDLE m_hAudioThreadTerminateRequest;
	HANDLE m_hAudioThreadGoneIdle;
	HANDLE m_hHardwareWakeupEvent;
	DWORD m_dwPlayThreadId;
	LONG m_AudioThreadActive;
	static DWORD WINAPI AudioThreadWrapper(LPVOID user);
	DWORD AudioThread();
	bool IsActive();

public:
	CAudioThread(CSoundDeviceWithThread &SoundDevice);
	CAudioThread(const CAudioThread &) = delete;
	CAudioThread &operator=(const CAudioThread &) = delete;
	~CAudioThread();
	void Activate();
	void Deactivate();
	void SetWakeupEvent(HANDLE ev);
	void SetWakeupInterval(double seconds);
};


class CSoundDeviceWithThread
	: public SoundDevice::Base
{
	friend class CAudioThread;

protected:
	CAudioThread m_AudioThread;

private:
	void FillAudioBufferLocked();

protected:
	void SetWakeupEvent(HANDLE ev);
	void SetWakeupInterval(double seconds);

public:
	CSoundDeviceWithThread(ILogger &logger, SoundDevice::Info info, SoundDevice::SysInfo sysInfo);
	virtual ~CSoundDeviceWithThread();
	bool InternalStart();
	void InternalStop();
	virtual void StartFromSoundThread() = 0;
	virtual void StopFromSoundThread() = 0;
};


#endif  // MPT_OS_WINDOWS


#if MPT_OS_LINUX || MPT_OS_MACOSX_OR_IOS || MPT_OS_FREEBSD


class semaphore
{
private:
	unsigned int count;
	unsigned int waiters_count;
	std::mutex mutex;
	std::condition_variable count_nonzero;

public:
	semaphore(unsigned int initial_count = 0)
		: count(initial_count)
		, waiters_count(0)
	{
		return;
	}
	~semaphore()
	{
		return;
	}
	void wait()
	{
		std::unique_lock<std::mutex> l(mutex);
		waiters_count++;
		while(count == 0)
		{
			count_nonzero.wait(l);
		}
		waiters_count--;
		count--;
	}
	void post()
	{
		std::unique_lock<std::mutex> l(mutex);
		if(waiters_count > 0)
		{
			count_nonzero.notify_one();
		}
		count++;
	}
	void lock()
	{
		wait();
	}
	void unlock()
	{
		post();
	}
};


class ThreadPriorityGuardImpl;

class ThreadPriorityGuard
{
private:
	std::unique_ptr<ThreadPriorityGuardImpl> impl;

public:
	ThreadPriorityGuard(ILogger &logger, bool active, bool realtime, int niceness, int rt_priority);
	~ThreadPriorityGuard();
};


class ThreadBase
	: public SoundDevice::Base
{
private:
	semaphore m_ThreadStarted;
	std::atomic<bool> m_ThreadStopRequest;
	std::thread m_Thread;

private:
	static void ThreadProcStatic(ThreadBase *this_);
	void ThreadProc();

public:
	ThreadBase(ILogger &logger, SoundDevice::Info info, SoundDevice::SysInfo sysInfo);
	virtual ~ThreadBase();
	bool InternalStart();
	void InternalStop();
	virtual void InternalStartFromSoundThread() = 0;
	virtual void InternalWaitFromSoundThread() = 0;
	virtual void InternalStopFromSoundThread() = 0;
};


#endif  // MPT_OS_LINUX || MPT_OS_MACOSX_OR_IOS || MPT_OS_FREEBSD


}  // namespace SoundDevice


OPENMPT_NAMESPACE_END
