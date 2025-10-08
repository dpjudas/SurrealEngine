/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_MAIN_MAIN_HPP
#define MPT_MAIN_MAIN_HPP



#include "mpt/base/check_platform.hpp"
#include "mpt/base/detect.hpp"
#include "mpt/base/integer.hpp"
#include "mpt/base/macros.hpp"
#include "mpt/base/namespace.hpp"
#include "mpt/string/types.hpp"
#include "mpt/string_transcode/transcode.hpp"

#include <type_traits>
#include <vector>

#if MPT_OS_DJGPP
#include <cassert>
#endif // MPT_OS_DJGPP

#if MPT_OS_DJGPP
#include <crt0.h>
#endif // MPT_OS_DJGPP

#if MPT_OS_WINDOWS
#include <windows.h>
#endif // MPT_OS_WINDOWS



namespace mpt {
inline namespace MPT_INLINE_NS {



namespace main {



#if MPT_OS_DJGPP
/* Work-around <https://gcc.gnu.org/bugzilla/show_bug.cgi?id=45977> */
/* clang-format off */
#define MPT_MAIN_PREFIX \
	extern "C" { \
		int _crt0_startup_flags = 0 \
			| _CRT0_FLAG_NONMOVE_SBRK          /* force interrupt compatible allocation */ \
			| _CRT0_DISABLE_SBRK_ADDRESS_WRAP  /* force NT compatible allocation */ \
			| _CRT0_FLAG_LOCK_MEMORY           /* lock all code and data at program startup */ \
			| 0; \
	} \
/* clang-format on */
#endif /* MPT_OS_DJGPP */

#if !defined(MPT_MAIN_PREFIX)
#define MPT_MAIN_PREFIX
#endif



#if MPT_OS_WINDOWS && defined(UNICODE)
#define MPT_MAIN_NAME wmain
#elif defined(MPT_OS_WINDOWS)
#define MPT_MAIN_NAME main
#endif

#if !defined(MPT_MAIN_NAME)
#define MPT_MAIN_NAME main
#endif



#if MPT_OS_WINDOWS && defined(UNICODE)
#define MPT_MAIN_ARGV_TYPE wchar_t
#elif defined(MPT_OS_WINDOWS)
#define MPT_MAIN_ARGV_TYPE char
#endif

#if !defined(MPT_MAIN_NAME)
#define MPT_MAIN_ARGV_TYPE char
#endif



#if MPT_OS_WINDOWS && (MPT_COMPILER_GCC || MPT_COMPILER_CLANG)
#if defined(UNICODE)
// mingw64 does only default to special C linkage for "main", but not for "wmain".
#define MPT_MAIN_DECL extern "C" int wmain(int argc, wchar_t * argv[]);
#endif
#endif

#if !defined(MPT_MAIN_DECL)
#define MPT_MAIN_DECL
#endif



#if MPT_OS_DJGPP
/* clang-format off */
#define MPT_MAIN_PROLOG() \
	do { \
		assert(mpt::platform::libc().is_ok()); \
		_crt0_startup_flags &= ~_CRT0_FLAG_LOCK_MEMORY; \
	} while (0) \
/**/
/* clang-format on */
#endif // MPT_OS_DJGPP

#if !defined(MPT_MAIN_PROLOG)
/* clang-format off */
#define MPT_MAIN_PROLOG() do { } while(0)
/* clang-format on */
#endif



#if MPT_OS_WINDOWS && (MPT_COMPILER_GCC || MPT_COMPILER_CLANG)
#if defined(UNICODE)
#define MPT_MAIN_DEF_PREFIX extern "C"
#endif
#endif // MPT_OS_WINDOWS && (MPT_COMPILER_GCC || MPT_COMPILER_CLANG)

#if !defined(MPT_MAIN_DEF_PREFIX)
#define MPT_MAIN_DEF_PREFIX
#endif



inline mpt::ustring transcode_arg(char * arg) {
	return mpt::transcode<mpt::ustring>(mpt::logical_encoding::locale, arg);
}

#if !defined(MPT_COMPILER_QUIRK_NO_WCHAR)
inline mpt::ustring transcode_arg(wchar_t * arg) {
	return mpt::transcode<mpt::ustring>(arg);
}
#endif

template <typename Tchar>
inline std::vector<mpt::ustring> transcode_argv(int argc, Tchar * argv[]) {
	std::vector<mpt::ustring> args;
	args.reserve(argc);
	for (int arg = 0; arg < argc; ++arg) {
		args.push_back(transcode_arg(argv[arg]));
	}
	return args;
}



#if !defined(MPT_MAIN_POSTFIX)
#define MPT_MAIN_POSTFIX
#endif



/* clang-format off */
#define MPT_MAIN_IMPLEMENT_MAIN(ns) \
	MPT_MAIN_PREFIX \
	MPT_MAIN_DECL \
	MPT_MAIN_DEF_PREFIX int MPT_MAIN_NAME(int argc, MPT_MAIN_ARGV_TYPE * argv[]) { \
		MPT_MAIN_PROLOG(); \
		static_assert(std::is_same<decltype(ns::main), mpt::uint8(std::vector<mpt::ustring>)>::value); \
		return static_cast<int>(ns::main(mpt::main::transcode_argv(argc, argv))); \
	} \
	MPT_MAIN_POSTFIX \
/**/
/* clang-format on */



/* clang-format off */
#define MPT_MAIN_IMPLEMENT_MAIN_NO_ARGS(ns) \
	MPT_MAIN_PREFIX \
	MPT_MAIN_DECL \
	MPT_MAIN_DEF_PREFIX int MPT_MAIN_NAME(int argc, MPT_MAIN_ARGV_TYPE * argv[]) { \
		MPT_MAIN_PROLOG(); \
		static_assert(std::is_same<decltype(ns::main), mpt::uint8(void)>::value); \
		MPT_UNUSED(argc); \
		MPT_UNUSED(argv); \
		return static_cast<int>(ns::main()); \
	} \
	MPT_MAIN_POSTFIX \
/**/
/* clang-format on */



} // namespace main



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_MAIN_MAIN_HPP
