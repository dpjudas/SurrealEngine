/*
 * mptTime.cpp
 * -----------
 * Purpose: Various time utility functions.
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"
#include "mptTime.h"

#if defined(MODPLUG_TRACKER) && MPT_OS_WINDOWS
#include "mpt/osinfo/windows_wine_version.hpp"
#endif // MODPLUG_TRACKER && MPT_OS_WINDOWS

#include "mptStringBuffer.h"

#if MPT_CXX_AT_LEAST(20) && !defined(MPT_LIBCXX_QUIRK_NO_CHRONO) && !defined(MPT_LIBCXX_QUIRK_NO_CHRONO_DATE)
#include <chrono>
#endif

#if defined(MODPLUG_TRACKER) && MPT_OS_WINDOWS
#include <optional>
#endif // MODPLUG_TRACKER && MPT_OS_WINDOWS

#if defined(MPT_FALLBACK_TIMEZONE_C)
#include <ctime>
#endif // MPT_FALLBACK_TIMEZONE_C

#if MPT_OS_WINDOWS
#include <windows.h>
#endif


OPENMPT_NAMESPACE_BEGIN


namespace mpt
{
namespace Date
{



#if defined(MODPLUG_TRACKER)

#if MPT_OS_WINDOWS

namespace ANSI
{

uint64 Now()
{
	FILETIME filetime;
#if MPT_WIN_AT_LEAST(MPT_WIN_8)
	GetSystemTimePreciseAsFileTime(&filetime);
#else
	GetSystemTimeAsFileTime(&filetime);
#endif
	return ((uint64)filetime.dwHighDateTime << 32 | filetime.dwLowDateTime);
}

mpt::ustring ToUString(uint64 time100ns)
{
	constexpr std::size_t bufsize = 256;

	mpt::ustring result;

	FILETIME filetime;
	SYSTEMTIME systime;
	filetime.dwHighDateTime = (DWORD)(((uint64)time100ns) >> 32);
	filetime.dwLowDateTime = (DWORD)((uint64)time100ns);
	FileTimeToSystemTime(&filetime, &systime);

	TCHAR buf[bufsize];

	GetDateFormat(LOCALE_SYSTEM_DEFAULT, 0, &systime, TEXT("yyyy-MM-dd"), buf, bufsize);
	result.append(mpt::ToUnicode(mpt::String::ReadWinBuf(buf)));

	result.append(U_(" "));

	GetTimeFormat(LOCALE_SYSTEM_DEFAULT, TIME_FORCE24HOURFORMAT, &systime, TEXT("HH:mm:ss"), buf, bufsize);
	result.append(mpt::ToUnicode(mpt::String::ReadWinBuf(buf)));

	result.append(U_("."));

	result.append(mpt::ufmt::dec0<3>((unsigned)systime.wMilliseconds));

	return result;

}

} // namespace ANSI

#endif // MPT_OS_WINDOWS

#endif // MODPLUG_TRACKER



namespace nochrono
{

static int32 ToDaynum(int32 year, int32 month, int32 day)
{
	month = (month + 9) % 12;
	year = year - (month / 10);
	int32 daynum = year*365 + year/4 - year/100 + year/400 + (month*306 + 5)/10 + (day - 1);
	return daynum;
}

static void FromDaynum(int32 d, int32 & year, int32 & month, int32 & day)
{
	int64 g = d;
	int64 y,ddd,mi,mm,dd;

	y = (10000*g + 14780)/3652425;
	ddd = g - (365*y + y/4 - y/100 + y/400);
	if(ddd < 0)
	{
		y = y - 1;
		ddd = g - (365*y + y/4 - y/100 + y/400);
	}
	mi = (100*ddd + 52)/3060;
	mm = (mi + 2)%12 + 1;
	y = y + (mi + 2)/12;
	dd = ddd - (mi*306 + 5)/10 + 1;

	year = static_cast<int32>(y);
	month = static_cast<int32>(mm);
	day = static_cast<int32>(dd);
}

Unix UnixFromUTC(UTC timeUtc)
{
	int32 daynum = ToDaynum(timeUtc.year, timeUtc.month, timeUtc.day);
	int64 seconds = static_cast<int64>(daynum - ToDaynum(1970, 1, 1)) * 24 * 60 * 60 + timeUtc.hours * 60 * 60 + timeUtc.minutes * 60 + timeUtc.seconds;
	return Unix{seconds};
}

UTC UnixAsUTC(Unix tp)
{
	int64 tmp = tp.value;
	int64 seconds = tmp % 60; tmp /= 60;
	int64 minutes = tmp % 60; tmp /= 60;
	int64 hours   = tmp % 24; tmp /= 24;
	int32 year = 0, month = 0, day = 0;
	FromDaynum(static_cast<int32>(tmp) + ToDaynum(1970,1,1), year, month, day);
	UTC result = {};
	result.year = year;
	result.month = month;
	result.day = day;
	result.hours = static_cast<int32>(hours);
	result.minutes = static_cast<int32>(minutes);
	result.seconds = seconds;
	return result;
}

#if defined(MODPLUG_TRACKER)

struct tz_error
{
};

Unix UnixFromLocal(Local timeLocal)
{
#if defined(MPT_FALLBACK_TIMEZONE_WINDOWS_HISTORIC)
	try
	{
		if(mpt::osinfo::windows::current_is_wine())
		{
			throw tz_error{};
		}
		SYSTEMTIME sys_local{};
		sys_local.wYear = static_cast<uint16>(timeLocal.year);
		sys_local.wMonth = static_cast<uint16>(timeLocal.month);
		sys_local.wDay = static_cast<uint16>(timeLocal.day);
		sys_local.wHour = static_cast<uint16>(timeLocal.hours);
		sys_local.wMinute = static_cast<uint16>(timeLocal.minutes);
		sys_local.wSecond = static_cast<uint16>(timeLocal.seconds);
		sys_local.wMilliseconds = 0;
		DYNAMIC_TIME_ZONE_INFORMATION dtzi{};
		if(GetDynamicTimeZoneInformation(&dtzi) == TIME_ZONE_ID_INVALID) // WinVista
		{
			throw tz_error{};
		}
		SYSTEMTIME sys_utc{};
		if(TzSpecificLocalTimeToSystemTimeEx(&dtzi, &sys_local, &sys_utc) == FALSE) // Win7/Win8
		{
			throw tz_error{};
		}
		FILETIME ft{};
		if(SystemTimeToFileTime(&sys_utc, &ft) == FALSE) // Win 2000
		{
			throw tz_error{};
		}
		ULARGE_INTEGER time_value{};
		time_value.LowPart = ft.dwLowDateTime;
		time_value.HighPart = ft.dwHighDateTime;
		return UnixFromSeconds(static_cast<int64>((time_value.QuadPart - 116444736000000000LL) / 10000000LL));
	} catch(const tz_error &)
	{
		// nothing
	}
#endif
#if defined(MPT_FALLBACK_TIMEZONE_WINDOWS_CURRENT)
	try
	{
		SYSTEMTIME sys_local{};
		sys_local.wYear = static_cast<uint16>(timeLocal.year);
		sys_local.wMonth = static_cast<uint16>(timeLocal.month);
		sys_local.wDay = static_cast<uint16>(timeLocal.day);
		sys_local.wHour = static_cast<uint16>(timeLocal.hours);
		sys_local.wMinute = static_cast<uint16>(timeLocal.minutes);
		sys_local.wSecond = static_cast<uint16>(timeLocal.seconds);
		sys_local.wMilliseconds = 0;
		SYSTEMTIME sys_utc{};
		if(TzSpecificLocalTimeToSystemTime(NULL, &sys_local, &sys_utc) == FALSE) // WinXP
		{
			throw tz_error{};
		}
		FILETIME ft{};
		if(SystemTimeToFileTime(&sys_utc, &ft) == FALSE) // Win 2000
		{
			throw tz_error{};
		}
		ULARGE_INTEGER time_value{};
		time_value.LowPart = ft.dwLowDateTime;
		time_value.HighPart = ft.dwHighDateTime;
		return UnixFromSeconds(static_cast<int64>((time_value.QuadPart - 116444736000000000LL) / 10000000LL));
	} catch(const tz_error &)
	{
		// nothing
	}
#endif
#if defined(MPT_FALLBACK_TIMEZONE_C)
	std::tm tmp{};
	tmp.tm_year = timeLocal.year - 1900;
	tmp.tm_mon = timeLocal.month - 1;
	tmp.tm_mday = timeLocal.day;
	tmp.tm_hour = timeLocal.hours;
	tmp.tm_min = timeLocal.minutes;
	tmp.tm_sec = static_cast<int>(timeLocal.seconds);
	return UnixFromSeconds(static_cast<int64>(std::mktime(&tmp)));
#endif
}

Local UnixAsLocal(Unix tp)
{
#if defined(MPT_FALLBACK_TIMEZONE_WINDOWS_HISTORIC)
	try
	{
		if(mpt::osinfo::windows::current_is_wine())
		{
			throw tz_error{};
		}
		ULARGE_INTEGER time_value{};
		time_value.QuadPart = static_cast<int64>(UnixAsSeconds(tp)) * 10000000LL + 116444736000000000LL;
		FILETIME ft{};
		ft.dwLowDateTime = time_value.LowPart;
		ft.dwHighDateTime = time_value.HighPart;
		SYSTEMTIME sys_utc{};
		if(FileTimeToSystemTime(&ft, &sys_utc) == FALSE) // WinXP
		{
			throw tz_error{};
		}
		DYNAMIC_TIME_ZONE_INFORMATION dtzi{};
		if(GetDynamicTimeZoneInformation(&dtzi) == TIME_ZONE_ID_INVALID) // WinVista
		{
			throw tz_error{};
		}
		SYSTEMTIME sys_local{};
		if(SystemTimeToTzSpecificLocalTimeEx(&dtzi, &sys_utc, &sys_local) == FALSE) // Win7/Win8
		{
			throw tz_error{};
		}
		Local result{};
		result.year = sys_local.wYear;
		result.month = sys_local.wMonth;
		result.day = sys_local.wDay;
		result.hours = sys_local.wHour;
		result.minutes = sys_local.wMinute;
		result.seconds = sys_local.wSecond;
		return result;
	} catch(const tz_error&)
	{
		// nothing
	}
#endif
#if defined(MPT_FALLBACK_TIMEZONE_WINDOWS_CURRENT)
	try
	{
		ULARGE_INTEGER time_value{};
		time_value.QuadPart = static_cast<int64>(UnixAsSeconds(tp)) * 10000000LL + 116444736000000000LL;
		FILETIME ft{};
		ft.dwLowDateTime = time_value.LowPart;
		ft.dwHighDateTime = time_value.HighPart;
		SYSTEMTIME sys_utc{};
		if(FileTimeToSystemTime(&ft, &sys_utc) == FALSE) // WinXP
		{
			throw tz_error{};
		}
		SYSTEMTIME sys_local{};
		if(SystemTimeToTzSpecificLocalTime(NULL, &sys_utc, &sys_local) == FALSE) // Win2000
		{
			throw tz_error{};
		}
		Local result{};
		result.year = sys_local.wYear;
		result.month = sys_local.wMonth;
		result.day = sys_local.wDay;
		result.hours = sys_local.wHour;
		result.minutes = sys_local.wMinute;
		result.seconds = sys_local.wSecond;
		return result;
	} catch(const tz_error&)
	{
		// nothing
	}
#endif
#if defined(MPT_FALLBACK_TIMEZONE_C)
	std::time_t time_tp = static_cast<std::time_t>(UnixAsSeconds(tp));
	std::tm *tmp = std::localtime(&time_tp);
	if(!tmp)
	{
		return Local{};
	}
	std::tm local = *tmp;
	Local result{};
	result.year = local.tm_year + 1900;
	result.month = local.tm_mon + 1;
	result.day = local.tm_mday;
	result.hours = local.tm_hour;
	result.minutes = local.tm_min;
	result.seconds = local.tm_sec;
	return result;
#endif
}

#endif // MODPLUG_TRACKER

} // namespace nochrono

template <LogicalTimezone TZ>
static mpt::ustring ToShortenedISO8601Impl(mpt::Date::Gregorian<TZ> date)
{
	mpt::ustring result;
	mpt::ustring tz;
	if constexpr(TZ == LogicalTimezone::Unspecified)
	{
		tz = U_("");
	} else if constexpr(TZ == LogicalTimezone::UTC)
	{
		tz = U_("Z");
	} else
	{
		tz = U_("");
	}
	if(date.year == 0)
	{
		return result;
	}
	result += mpt::ufmt::dec0<4>(date.year);
	result += U_("-") + mpt::ufmt::dec0<2>(date.month);
	result += U_("-") + mpt::ufmt::dec0<2>(date.day);
	if(date.hours == 0 && date.minutes == 0 && date.seconds)
	{
		return result;
	}
	result += U_("T");
	result += mpt::ufmt::dec0<2>(date.hours) + U_(":") + mpt::ufmt::dec0<2>(date.minutes);
	if(date.seconds == 0)
	{
		return result + tz;
	}
	result += U_(":") + mpt::ufmt::dec0<2>(date.seconds);
	result += tz;
	return result;
}

mpt::ustring ToShortenedISO8601(mpt::Date::AnyGregorian date)
{
	return ToShortenedISO8601Impl(date);
}

mpt::ustring ToShortenedISO8601(mpt::Date::UTC date)
{
	return ToShortenedISO8601Impl(date);
}

#ifdef MODPLUG_TRACKER
mpt::ustring ToShortenedISO8601(Local date)
{
	return ToShortenedISO8601Impl(date);
}
#endif // MODPLUG_TRACKER

} // namespace Date
} // namespace mpt



OPENMPT_NAMESPACE_END
