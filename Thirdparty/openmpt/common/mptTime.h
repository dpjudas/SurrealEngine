/*
 * mptTime.h
 * ---------
 * Purpose: Various time utility functions.
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#pragma once

#include "openmpt/all/BuildSettings.hpp"

#if MPT_CXX_AT_LEAST(20) && !defined(MPT_LIBCXX_QUIRK_NO_CHRONO) && !defined(MPT_LIBCXX_QUIRK_NO_CHRONO_DATE)
#include <chrono>
#include <exception>
#elif MPT_CXX_AT_LEAST(17) && !defined(MPT_LIBCXX_QUIRK_NO_CHRONO) && defined(MODPLUG_TRACKER)
#include <chrono>
#endif

#if MPT_CXX_BEFORE(20) || defined(MPT_LIBCXX_QUIRK_NO_CHRONO) || defined(MPT_LIBCXX_QUIRK_NO_CHRONO_DATE)
#include <ctime>
#endif

#if MPT_WINNT_AT_LEAST(MPT_WIN_8)
#define MPT_FALLBACK_TIMEZONE_WINDOWS_HISTORIC
#define MPT_FALLBACK_TIMEZONE_WINDOWS_CURRENT
#define MPT_FALLBACK_TIMEZONE_C
#elif MPT_WINNT_AT_LEAST(MPT_WIN_XP)
#define MPT_FALLBACK_TIMEZONE_WINDOWS_CURRENT
#define MPT_FALLBACK_TIMEZONE_C
#else
#define MPT_FALLBACK_TIMEZONE_C
#endif



OPENMPT_NAMESPACE_BEGIN



#if defined(MODPLUG_TRACKER) && !defined(MPT_LIBCXX_QUIRK_NO_CHRONO)

namespace mpt {
namespace chrono {
#if MPT_CXX_AT_LEAST(20)
using days = std::chrono::days;
using weeks = std::chrono::weeks;
using years = std::chrono::years;
using months = std::chrono::months;
#else
using days = std::chrono::duration<int, std::ratio_multiply<std::ratio<24>, std::chrono::hours::period>>;
using weeks = std::chrono::duration<int, std::ratio_multiply<std::ratio<7>, mpt::chrono::days::period>>;
using years = std::chrono::duration<int, std::ratio_multiply<std::ratio<146097, 400>, mpt::chrono::days::period>>;
using months = std::chrono::duration<int, std::ratio_divide<mpt::chrono::years::period, std::ratio<12>>>;
#endif
}
}

#endif // !MPT_LIBCXX_QUIRK_NO_CHRONO



namespace mpt
{
namespace Date
{



#if defined(MODPLUG_TRACKER)

#if MPT_OS_WINDOWS

namespace ANSI
{
// uint64 counts 100ns since 1601-01-01T00:00Z

uint64 Now();

mpt::ustring ToUString(uint64 time100ns); // i.e. 2015-01-15 18:32:01.718

} // namespacee ANSI

#endif // MPT_OS_WINDOWS

#endif // MODPLUG_TRACKER



enum class LogicalTimezone
{
	Unspecified,
	UTC,
#if defined(MODPLUG_TRACKER)
	Local,
#endif // MODPLUG_TRACKER
};

template <LogicalTimezone tz>
struct Gregorian
{
	int          year    = 0;
	unsigned int month   = 0;
	unsigned int day     = 0;
	int32        hours   = 0;
	int32        minutes = 0;
	int64        seconds = 0;
	friend bool operator==(const Gregorian<tz>& lhs, const Gregorian<tz>& rhs)
	{
		return std::tie(lhs.year, lhs.month, lhs.day, lhs.hours, lhs.minutes, lhs.seconds)
			== std::tie(rhs.year, rhs.month, rhs.day, rhs.hours, rhs.minutes, rhs.seconds);
	}
	friend bool operator!=(const Gregorian<tz>& lhs, const Gregorian<tz>& rhs)
	{
		return std::tie(lhs.year, lhs.month, lhs.day, lhs.hours, lhs.minutes, lhs.seconds)
			!= std::tie(rhs.year, rhs.month, rhs.day, rhs.hours, rhs.minutes, rhs.seconds);
	}
	friend bool operator<(const Gregorian<tz> &lhs, const Gregorian<tz> &rhs)
	{
		return std::tie(lhs.year, lhs.month, lhs.day, lhs.hours, lhs.minutes, lhs.seconds)
			< std::tie(rhs.year, rhs.month, rhs.day, rhs.hours, rhs.minutes, rhs.seconds);
	}
};

using AnyGregorian = Gregorian<LogicalTimezone::Unspecified>;

using UTC = Gregorian<LogicalTimezone::UTC>;

#if defined(MODPLUG_TRACKER)
using Local = Gregorian<LogicalTimezone::Local>;
#endif // MODPLUG_TRACKER

template <LogicalTimezone TZ>
inline Gregorian<TZ> interpret_as_timezone(AnyGregorian gregorian)
{
	Gregorian<TZ> result;
	result.year = gregorian.year;
	result.month = gregorian.month;
	result.day = gregorian.day;
	result.hours = gregorian.hours;
	result.minutes = gregorian.minutes;
	result.seconds = gregorian.seconds;
	return result;
}

template <LogicalTimezone TZ>
inline Gregorian<LogicalTimezone::Unspecified> forget_timezone(Gregorian<TZ> gregorian)
{
	Gregorian<LogicalTimezone::Unspecified> result;
	result.year = gregorian.year;
	result.month = gregorian.month;
	result.day = gregorian.day;
	result.hours = gregorian.hours;
	result.minutes = gregorian.minutes;
	result.seconds = gregorian.seconds;
	return result;
}

namespace nochrono
{

// int64 counts 1s since 1970-01-01T00:00Z
struct Unix
{
	int64 value{};
	friend bool operator==(const Unix &a, const Unix &b)
	{
		return a.value == b.value;
	}
	friend bool operator!=(const Unix &a, const Unix &b)
	{
		return a.value != b.value;
	}
};

#if MPT_CXX_BEFORE(20) || defined(MPT_LIBCXX_QUIRK_NO_CHRONO) || defined(MPT_LIBCXX_QUIRK_NO_CHRONO_DATE)

inline Unix UnixNow()
{
	return Unix{static_cast<int64>(std::time(nullptr))};
}

#endif

inline int64 UnixAsSeconds(Unix tp)
{
	return tp.value;
}

inline Unix UnixFromSeconds(int64 seconds)
{
	return Unix{seconds};
}

Unix UnixFromUTC(UTC timeUtc);

UTC UnixAsUTC(Unix tp);

#if defined(MODPLUG_TRACKER)

Unix UnixFromLocal(Local timeLocal);

Local UnixAsLocal(Unix tp);

#endif // MODPLUG_TRACKER

} // namespace nochrono

#if MPT_CXX_AT_LEAST(20) && !defined(MPT_LIBCXX_QUIRK_NO_CHRONO) && !defined(MPT_LIBCXX_QUIRK_NO_CHRONO_DATE)

using Unix = std::chrono::system_clock::time_point;

inline Unix UnixNow()
{
	return std::chrono::system_clock::now();
}

inline int64 UnixAsSeconds(Unix tp)
{
	return std::chrono::duration_cast<std::chrono::seconds>(tp.time_since_epoch()).count();
}

inline Unix UnixFromSeconds(int64 seconds)
{
	return std::chrono::system_clock::time_point{std::chrono::seconds{seconds}};
}

inline mpt::Date::Unix UnixFromUTC(UTC utc)
{
	try
	{
		return std::chrono::system_clock::time_point{
			std::chrono::sys_days {
				std::chrono::year{ utc.year } /
				std::chrono::month{ utc.month } /
				std::chrono::day{ utc.day }
			} +
			std::chrono::hours{ utc.hours } +
			std::chrono::minutes{ utc.minutes } +
			std::chrono::seconds{ utc.seconds }};
	} catch(const std::exception &)
	{
		return mpt::Date::UnixFromSeconds(mpt::Date::nochrono::UnixAsSeconds(mpt::Date::nochrono::UnixFromUTC(utc)));
	}
}

inline mpt::Date::UTC UnixAsUTC(Unix tp)
{
	try
	{
		std::chrono::sys_days dp = std::chrono::floor<std::chrono::days>(tp);
		std::chrono::year_month_day ymd{dp};
		std::chrono::hh_mm_ss hms{tp - dp};
		mpt::Date::UTC result;
		result.year = static_cast<int>(ymd.year());
		result.month = static_cast<unsigned int>(ymd.month());
		result.day = static_cast<unsigned int>(ymd.day());
		result.hours = static_cast<int32>(hms.hours().count());
		result.minutes = static_cast<int32>(hms.minutes().count());
		result.seconds = static_cast<int64>(hms.seconds().count());
		return result;
	} catch(const std::exception &)
	{
		return mpt::Date::nochrono::UnixAsUTC(mpt::Date::nochrono::UnixFromSeconds(mpt::Date::UnixAsSeconds(tp)));
	}
}

#if defined(MODPLUG_TRACKER)

inline mpt::Date::Unix UnixFromLocal(Local local)
{
#if !defined(MPT_LIBCXX_QUIRK_CHRONO_DATE_NO_ZONED_TIME)
	try
	{
		std::chrono::time_point<std::chrono::local_t, std::chrono::seconds> local_tp =
			std::chrono::local_days {
				std::chrono::year{ local.year } /
				std::chrono::month{ local.month } /
				std::chrono::day{ local.day }
			} +
			std::chrono::hours{ local.hours } +
			std::chrono::minutes{ local.minutes } +
			std::chrono::seconds{ local.seconds };
#if defined(MPT_LIBCXX_QUIRK_CHRONO_DATE_BROKEN_ZONED_TIME)
		return std::chrono::zoned_time{std::chrono::current_zone(), std::chrono::current_zone()->to_sys(local_tp)}.get_sys_time();
#else
		return std::chrono::zoned_time{std::chrono::current_zone(), local_tp}.get_sys_time();
#endif
	} catch(const std::exception &)
#endif
	{
		return mpt::Date::UnixFromSeconds(mpt::Date::nochrono::UnixAsSeconds(mpt::Date::nochrono::UnixFromLocal(local)));
	}
}

inline mpt::Date::Local UnixAsLocal(Unix tp)
{
#if !defined(MPT_LIBCXX_QUIRK_CHRONO_DATE_NO_ZONED_TIME)
	try
	{
		std::chrono::zoned_time local_tp{ std::chrono::current_zone(), tp };
		std::chrono::local_days dp = std::chrono::floor<std::chrono::days>(local_tp.get_local_time());
		std::chrono::year_month_day ymd{dp};
		std::chrono::hh_mm_ss hms{local_tp.get_local_time() - dp};
		mpt::Date::Local result;
		result.year = static_cast<int>(ymd.year());
		result.month = static_cast<unsigned int>(ymd.month());
		result.day = static_cast<unsigned int>(ymd.day());
		result.hours = static_cast<int32>(hms.hours().count());
		result.minutes = static_cast<int32>(hms.minutes().count());
		result.seconds = static_cast<int64>(hms.seconds().count());
		return result;
	} catch(const std::exception &)
#endif
	{
		return mpt::Date::nochrono::UnixAsLocal(mpt::Date::nochrono::UnixFromSeconds(mpt::Date::UnixAsSeconds(tp)));
	}
}

#endif // MODPLUG_TRACKER

#else

using Unix = nochrono::Unix;

using nochrono::UnixNow;
using nochrono::UnixAsSeconds;
using nochrono::UnixFromSeconds;

using nochrono::UnixFromUTC;
using nochrono::UnixAsUTC;

#if defined(MODPLUG_TRACKER)

using nochrono::UnixFromLocal;
using nochrono::UnixAsLocal;

#endif // MODPLUG_TRACKER

#endif

mpt::ustring ToShortenedISO8601(AnyGregorian date); // i.e. 2015-01-15T18:32:01

mpt::ustring ToShortenedISO8601(UTC date); // i.e. 2015-01-15T18:32:01Z

#ifdef MODPLUG_TRACKER
mpt::ustring ToShortenedISO8601(Local date); // i.e. 2015-01-15T18:32:01
#endif // MODPLUG_TRACKER

} // namespace Date
} // namespace mpt



OPENMPT_NAMESPACE_END
