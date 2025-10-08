/*
 * Logging.h
 * ---------
 * Purpose: General logging
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */

#pragma once

#include "openmpt/all/BuildSettings.hpp"

#include "openmpt/logging/Logger.hpp"

#include "mptPathString.h"
#include "mptString.h"

#if defined(MODPLUG_TRACKER) && MPT_OS_WINDOWS
#include <atomic>
#endif

OPENMPT_NAMESPACE_BEGIN


/*


Build time logging configuration:

 *  #define MPT_LOG_GLOBAL_LEVEL_STATIC
    #define MPT_LOG_GLOBAL_LEVEL #
    Define the former (to anything) and the latter (to one of the log levels
    below) in order to statically select the verbosity of logging at build time.
    MPT_LOG calls that exceed the specified logging level will get dead-code
    eliminated at compile time.
    This especially means that, when setting MPT_LOG_GLOBAL_LEVEL to 0, no
    MPT_LOG call (with a constant level parameter) remains in the resulting
    binary, however, they still do get parsed and properly type checked by the
    compiler.


Logging:

If the context is related to a particular CSoundfile instance, use
CSoundfile::AddToLog.

Logging a simple message:
MPT_LOG_GLOBAL(LogWarning, "sounddev", "some message");
MPT_LOG_GLOBAL(LogWarning, "sounddev", U_("some message"));
Facility is some course grained code section identifier (more coarse grained
than the current file name probably), useful to do some selective logging.

Logging a more complex message:
MPT_LOG_GLOBAL(LogWarning, "sounddev", MPT_UFORMAT("Some message: foo={}, bar=0x{}")(foo, mpt::ufmt::hex0<8>(bar)));

Note that even with full enabled logging and a runtime configurable logging
level, the runtime overhead of a MPT_LOG_GLOBAL(level, facility, text) call is just a
single conditional in case the verbosity does not require logging the respective
message. Even the expression "text" is not evaluated.


*/


inline mpt::ustring LogLevelToString(LogLevel level)
{
	switch(level)
	{
	case LogError:        return U_("error");   break;
	case LogWarning:      return U_("warning"); break;
	case LogNotification: return U_("notify");  break;
	case LogInformation:  return U_("info");    break;
	case LogDebug:        return U_("debug");   break;
	}
	return U_("unknown");
}


class ILog
{
protected:
	virtual ~ILog() { }
public:
	virtual	void AddToLog(LogLevel level, const mpt::ustring &text) const = 0;
};



namespace mpt
{
namespace log
{



#if defined(MPT_LOG_GLOBAL_LEVEL_STATIC)
#if (MPT_LOG_GLOBAL_LEVEL <= 0)
// All logging has beeen statically disabled.
// All logging code gets compiled and immediately dead-code eliminated.
#define MPT_LOG_IS_DISABLED
#endif
inline constexpr int GlobalLogLevel = MPT_LOG_GLOBAL_LEVEL ;
#else
extern int GlobalLogLevel;
#endif


#if defined(MODPLUG_TRACKER) && !defined(MPT_LOG_IS_DISABLED)
extern bool FileEnabled;
extern bool DebuggerEnabled;
extern bool ConsoleEnabled;
void SetFacilities(const std::string &solo, const std::string &blocked);
bool IsFacilityActive(const char *facility) noexcept;
#else
MPT_FORCEINLINE bool IsFacilityActive(const char * /*facility*/ ) noexcept { return true; }
#endif



class GlobalLogger final
	: public ILogger
{
public:
	GlobalLogger() = default;
	~GlobalLogger() final = default;
public:
	bool IsLevelActive(LogLevel level) const noexcept override
	{
		return (mpt::log::GlobalLogLevel >= level);
	}
	bool IsFacilityActive(const char *facility) const noexcept override
	{
		return mpt::log::IsFacilityActive(facility);
	}
	void SendLogMessage(const mpt::source_location &loc, LogLevel level, const char *facility, const mpt::ustring &message) const override;
};


#define MPT_LOG_GLOBAL(level, facility, text) MPT_LOG(mpt::log::GlobalLogger{}, (level), (facility), (text))



#if defined(MODPLUG_TRACKER) && MPT_OS_WINDOWS

namespace Trace {

// This is not strictly thread safe in all corner cases because of missing barriers.
// We do not care in order to not harm the fast path with additional barriers.
// Enabled tracing incurs a runtime overhead with multiple threads as a global atomic variable
//  gets modified.
// This cacheline bouncing does not matter at all
//  if there are not multiple thread adding trace points at high frequency (way greater than 1000Hz),
//  which, in OpenMPT, is only ever the case for just a single thread (the audio thread), if at all.
extern std::atomic<bool> g_Enabled;
inline bool IsEnabled() { return g_Enabled; }

enum class Direction : int8
{
	Unknown =  0,
	Enter   =  1,
	Leave   = -1,
};

MPT_NOINLINE void Trace(const mpt::source_location & loc, Direction direction = Direction::Unknown) noexcept;

enum ThreadKind {
	ThreadKindGUI,
	ThreadKindAudio,
	ThreadKindNotify,
	ThreadKindWatchdir,
};

void Enable(std::size_t numEntries);
void Disable();

void SetThreadId(mpt::log::Trace::ThreadKind kind, uint32 id);
uint32 GetThreadId(mpt::log::Trace::ThreadKind kind);

void Seal();
bool Dump(const mpt::PathString &filename);

class Scope
{
private:
	const mpt::source_location loc;
public:
	MPT_FORCEINLINE Scope(mpt::source_location loc) noexcept
		: loc(loc)
	{
		if(mpt::log::Trace::g_Enabled)
		{
			mpt::log::Trace::Trace(loc, mpt::log::Trace::Direction::Enter);
		}
	}
	MPT_FORCEINLINE ~Scope() noexcept
	{
		if(mpt::log::Trace::g_Enabled)
		{
			mpt::log::Trace::Trace(loc, mpt::log::Trace::Direction::Leave);
		}
	}
};

#define MPT_TRACE_CONCAT_HELPER(x, y) x ## y
#define MPT_TRACE_CONCAT(x, y) MPT_TRACE_CONCAT_HELPER(x, y)

#define MPT_TRACE_SCOPE() mpt::log::Trace::Scope MPT_TRACE_CONCAT(MPT_TRACE_VAR, __LINE__)(MPT_SOURCE_LOCATION_CURRENT())

#define MPT_TRACE() do { if(mpt::log::Trace::g_Enabled) { mpt::log::Trace::Trace(MPT_SOURCE_LOCATION_CURRENT()); } } while(0)

} // namespace Trace

#else // !MODPLUG_TRACKER

#define MPT_TRACE_SCOPE() do { } while(0)

#define MPT_TRACE() do { } while(0)

#endif // MODPLUG_TRACKER



} // namespace log
} // namespace mpt


OPENMPT_NAMESPACE_END
