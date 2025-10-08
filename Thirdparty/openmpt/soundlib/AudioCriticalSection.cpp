/*
 * AudioCriticalSection.cpp
 * -----------
 * Purpose: Implementation of OpenMPT's critical section for access to CSoundFile.
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */

#include "stdafx.h"

#include "AudioCriticalSection.h"

#if defined(MODPLUG_TRACKER)
#include "../misc/mptMutex.h"
#endif

OPENMPT_NAMESPACE_BEGIN

#if defined(MODPLUG_TRACKER)

#if MPT_COMPILER_MSVC
_Acquires_lock_(m_refGlobalMutex.mutex)
#endif // MPT_COMPILER_MSVC
CriticalSection::CriticalSection()
	: m_refGlobalMutex(Tracker::GetGlobalMutexRef())
	, inSection(false)
{
	Enter();
}

CriticalSection::CriticalSection(CriticalSection &&other) noexcept
	: m_refGlobalMutex(other.m_refGlobalMutex)
	, inSection(other.inSection)
{
	other.inSection = false;
}

CriticalSection::CriticalSection(InitialState state)
	: m_refGlobalMutex(Tracker::GetGlobalMutexRef())
	, inSection(false)
{
	if(state == InitialState::Locked)
	{
		Enter();
	}
}

#if MPT_COMPILER_MSVC
_Acquires_lock_(m_refGlobalMutex.mutex)
#endif // MPT_COMPILER_MSVC
void CriticalSection::Enter()
{
	if(!inSection)
	{
		inSection = true;
		m_refGlobalMutex.lock();
	}
}

#if MPT_COMPILER_MSVC
_Requires_lock_held_(m_refGlobalMutex.mutex) _Releases_lock_(m_refGlobalMutex.mutex)
#endif // MPT_COMPILER_MSVC
void CriticalSection::Leave()
{
	if(inSection)
	{
		inSection = false;
		m_refGlobalMutex.unlock();
	}
}
CriticalSection::~CriticalSection()
{
	Leave();
}

#else

MPT_MSVC_WORKAROUND_LNK4221(AudioCriticalSection)

#endif

OPENMPT_NAMESPACE_END
