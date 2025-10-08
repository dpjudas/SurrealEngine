/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_OSINFO_CLASS_HPP
#define MPT_OSINFO_CLASS_HPP



#include "mpt/base/detect_os.hpp"
#include "mpt/base/namespace.hpp"
#if !MPT_OS_WINDOWS
#include "mpt/string/buffer.hpp"
#endif // !MPT_OS_WINDOWS

#include <string>

#if !MPT_OS_WINDOWS
#include <sys/utsname.h>
#endif // !MPT_OS_WINDOWS



namespace mpt {
inline namespace MPT_INLINE_NS {



namespace osinfo {

enum class osclass {
	Unknown,
	Windows,
	Linux,
	Darwin,
	BSD_,
	Haiku,
	DOS,
};

inline mpt::osinfo::osclass get_class_from_sysname(const std::string & sysname) {
	mpt::osinfo::osclass result = mpt::osinfo::osclass::Unknown;
	if (sysname == "") {
		result = mpt::osinfo::osclass::Unknown;
	} else if (sysname == "Windows" || sysname == "WindowsNT" || sysname == "Windows_NT") {
		result = mpt::osinfo::osclass::Windows;
	} else if (sysname == "Linux") {
		result = mpt::osinfo::osclass::Linux;
	} else if (sysname == "Darwin") {
		result = mpt::osinfo::osclass::Darwin;
	} else if (sysname == "FreeBSD" || sysname == "DragonFly" || sysname == "NetBSD" || sysname == "OpenBSD" || sysname == "MidnightBSD") {
		result = mpt::osinfo::osclass::BSD_;
	} else if (sysname == "Haiku") {
		result = mpt::osinfo::osclass::Haiku;
	} else if (sysname == "IBMPcDos" || sysname == "CompqDOS" || sysname == "MsoftDOS" || sysname == "AT&T DOS" || sysname == "ZenitDOS" || sysname == "HP DOS" || sysname == "GrBulDOS" || sysname == "PBellDOS" || sysname == "DEC DOS" || sysname == "OlivtDOS" || sysname == "TI DOS" || sysname == "Toshiba" || sysname == "NWin3Dev" || sysname == "MSWinDev" || sysname == "RxDOS" || sysname == "PTS-DOS" || sysname == "GenSoft" || sysname == "DR-DOS" || sysname == "NovelDOS" || sysname == "FreeDOS" || sysname == "MS-DOS") {
		result = mpt::osinfo::osclass::DOS;
	}
	return result;
}

inline std::string get_sysname() {
#if MPT_OS_WINDOWS
	return "Windows";
#else  // !MPT_OS_WINDOWS
	utsname uname_result;
	if (uname(&uname_result) != 0) {
		return {};
	}
	return mpt::ReadAutoBuf(uname_result.sysname);
#endif // MPT_OS_WINDOWS
}

inline mpt::osinfo::osclass get_class() {
#if MPT_OS_WINDOWS
	return mpt::osinfo::osclass::Windows;
#else  // !MPT_OS_WINDOWS
	return mpt::osinfo::get_class_from_sysname(mpt::osinfo::get_sysname());
#endif // MPT_OS_WINDOWS
}

inline std::string get_class_name(mpt::osinfo::osclass c) {
	std::string result;
	switch (c) {
		case mpt::osinfo::osclass::Unknown:
			result = "unknown";
			break;
		case mpt::osinfo::osclass::Windows:
			result = "Windows";
			break;
		case mpt::osinfo::osclass::Linux:
			result = "Linux";
			break;
		case mpt::osinfo::osclass::Darwin:
			result = "Darwin";
			break;
		case mpt::osinfo::osclass::BSD_:
			result = "BSD";
			break;
		case mpt::osinfo::osclass::Haiku:
			result = "Haiku";
			break;
		case mpt::osinfo::osclass::DOS:
			result = "DOS";
			break;
	}
	return result;
}

} // namespace osinfo



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_OSINFO_CLASS_HPP
