/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_MUTEX_MUTEX_HPP
#define MPT_MUTEX_MUTEX_HPP



#include "mpt/base/detect.hpp"
#include "mpt/base/namespace.hpp"


#if !MPT_PLATFORM_MULTITHREADED
#define MPT_MUTEX_NONE 1
#elif defined(MPT_LIBCXX_QUIRK_NO_STD_THREAD)
#define MPT_MUTEX_WIN32 1
#else
#define MPT_MUTEX_STD 1
#endif

#ifndef MPT_MUTEX_STD
#define MPT_MUTEX_STD 0
#endif
#ifndef MPT_MUTEX_WIN32
#define MPT_MUTEX_WIN32 0
#endif
#ifndef MPT_MUTEX_NONE
#define MPT_MUTEX_NONE 0
#endif

#if MPT_MUTEX_STD
#include <mutex>
#ifdef MPT_LIBCXX_QUIRK_COMPLEX_STD_MUTEX
#include <shared_mutex>
#include <type_traits>
#endif
#elif MPT_MUTEX_WIN32
#include <windows.h>
#endif // MPT_MUTEX



namespace mpt {
inline namespace MPT_INLINE_NS {



#if MPT_MUTEX_STD

#ifdef MPT_LIBCXX_QUIRK_COMPLEX_STD_MUTEX
using mutex = std::conditional<sizeof(std::shared_mutex) < sizeof(std::mutex), std::shared_mutex, std::mutex>::type;
#else
using mutex = std::mutex;
#endif
using recursive_mutex = std::recursive_mutex;

#elif MPT_MUTEX_WIN32

#if MPT_WINNT_AT_LEAST(MPT_WIN_VISTA)

// compatible with c++11 std::mutex, can eventually be replaced without touching any usage site
class mutex {
private:
	SRWLOCK impl = SRWLOCK_INIT;

public:
	mutex() = default;
	~mutex() = default;
	void lock() {
		AcquireSRWLockExclusive(&impl);
	}
	bool try_lock() {
		return TryAcquireSRWLockExclusive(&impl) ? true : false;
	}
	void unlock() {
		ReleaseSRWLockExclusive(&impl);
	}
};

#else // !_WIN32_WINNT_VISTA

// compatible with c++11 std::mutex, can eventually be replaced without touching any usage site
class mutex {
private:
	CRITICAL_SECTION impl;

public:
	mutex() {
		InitializeCriticalSection(&impl);
	}
	~mutex() {
		DeleteCriticalSection(&impl);
	}
	void lock() {
		EnterCriticalSection(&impl);
	}
	bool try_lock() {
		return TryEnterCriticalSection(&impl) ? true : false;
	}
	void unlock() {
		LeaveCriticalSection(&impl);
	}
};

#endif // _WIN32_WINNT_VISTA

// compatible with c++11 std::recursive_mutex, can eventually be replaced without touching any usage site
class recursive_mutex {
private:
	CRITICAL_SECTION impl;

public:
	recursive_mutex() {
		InitializeCriticalSection(&impl);
	}
	~recursive_mutex() {
		DeleteCriticalSection(&impl);
	}
	void lock() {
		EnterCriticalSection(&impl);
	}
	bool try_lock() {
		return TryEnterCriticalSection(&impl) ? true : false;
	}
	void unlock() {
		LeaveCriticalSection(&impl);
	}
};

#else // MPT_MUTEX_NONE

class mutex {
public:
	mutex() {
		return;
	}
	~mutex() {
		return;
	}
	void lock() {
		return;
	}
	bool try_lock() {
		return true;
	}
	void unlock() {
		return;
	}
};

class recursive_mutex {
public:
	recursive_mutex() {
		return;
	}
	~recursive_mutex() {
		return;
	}
	void lock() {
		return;
	}
	bool try_lock() {
		return true;
	}
	void unlock() {
		return;
	}
};

#endif // MPT_MUTEX

#if MPT_MUTEX_STD

template <typename T>
using lock_guard = std::lock_guard<T>;

#else // !MPT_MUTEX_STD

// compatible with c++11 std::lock_guard, can eventually be replaced without touching any usage site
template <typename mutex_type>
class lock_guard {
private:
	mutex_type & mutex;

public:
	lock_guard(mutex_type & m)
		: mutex(m) {
		mutex.lock();
	}
	~lock_guard() {
		mutex.unlock();
	}
};

#endif // MPT_MUTEX_STD



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_MUTEX_MUTEX_HPP
