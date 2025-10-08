/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_PATH_OS_PATH_HPP
#define MPT_PATH_OS_PATH_HPP



#include "mpt/base/detect.hpp"
#include "mpt/base/namespace.hpp"
#include "mpt/string/types.hpp"

#if MPT_OS_WINDOWS
#include <windows.h>
#endif // MPT_OS_WINDOWS



namespace mpt {
inline namespace MPT_INLINE_NS {



// mpt::os_path is an alias to a string type that represents native operating system path encoding.
// Note that this differs from std::filesystem::path::string_type on both Windows and Posix.
// On Windows, we actually honor UNICODE and thus allow os_path.c_str() to be usable with WinAPI functions.
// On Posix, we use a type-safe string type in locale encoding, in contrast to the encoding-confused supposedly UTF8 std::string in std::filesystem::path.

#if MPT_OS_WINDOWS
using os_path = mpt::winstring;
#else  // !MPT_OS_WINDOWS
using os_path = mpt::lstring;
#endif // MPT_OS_WINDOWS



// mpt::os_path literals that do not involve runtime conversion.

#if MPT_OS_WINDOWS
#define MPT_OS_PATH_CHAR(x)    TEXT(x)
#define MPT_OS_PATH_LITERAL(x) TEXT(x)
#define MPT_OS_PATH(x) \
	mpt::winstring { \
		TEXT(x) \
	}
#else // !MPT_OS_WINDOWS
#define MPT_OS_PATH_CHAR(x)    x
#define MPT_OS_PATH_LITERAL(x) x
#define MPT_OS_PATH(x) \
	mpt::lstring { \
		x \
	}
#endif // MPT_OS_WINDOWS



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_PATH_OS_PATH_HPP
