/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_BASE_DETECT_QUIRKS_HPP
#define MPT_BASE_DETECT_QUIRKS_HPP



#include "mpt/base/detect_arch.hpp"
#include "mpt/base/detect_compiler.hpp"
#include "mpt/base/detect_libc.hpp"
#include "mpt/base/detect_libcxx.hpp"
#include "mpt/base/detect_os.hpp"

#if MPT_CXX_AT_LEAST(20)
#include <version>
#else // !C++20
#include <array>
#endif // C++20

#if MPT_LIBCXX_MS
// for _ITERATOR_DEBUG_LEVEL
#include <array>
#endif // MPT_LIBCXX_MS



#if MPT_OS_DJGPP
#define MPT_ARCH_QUIRK_NO_SIMD256
#endif



#if MPT_COMPILER_MSVC
#if !defined(_MSVC_TRADITIONAL)
#define MPT_COMPILER_QUIRK_MSVC_OLD_PREPROCESSOR
#else
#if _MSVC_TRADITIONAL
#define MPT_COMPILER_QUIRK_MSVC_OLD_PREPROCESSOR
#endif
#endif
#endif



#if MPT_GCC_BEFORE(9, 1, 0)
#define MPT_COMPILER_QUIRK_NO_CONSTEXPR_THROW
#endif



#if MPT_COMPILER_MSVC && MPT_MSVC_AT_LEAST(2022, 6) && MPT_MSVC_BEFORE(2022, 8) && MPT_ARCH_AARCH64
// VS2022 17.6.0 ARM64 gets confused about alignment in std::bit_cast (or equivalent code),
// causing an ICE.
// See <https://developercommunity.visualstudio.com/t/ICE-when-compiling-for-ARM64-due-to-alig/10367205>.
#define MPT_COMPILER_QUIRK_BROKEN_BITCAST
#endif



#if MPT_CXX_BEFORE(20) && MPT_COMPILER_MSVC
// Compiler has multiplication/division semantics when shifting signed integers.
// In C++20, this behaviour is required by the standard.
#define MPT_COMPILER_SHIFT_SIGNED 1
#endif

#ifndef MPT_COMPILER_SHIFT_SIGNED
#define MPT_COMPILER_SHIFT_SIGNED 0
#endif



// This should really be based on __STDCPP_THREADS__,
// but that is not defined consistently by GCC or clang. Stupid.
// Just assume multithreaded and disable for platforms we know are
// singlethreaded later on.
#define MPT_PLATFORM_MULTITHREADED 1

#if MPT_OS_DJGPP
#undef MPT_PLATFORM_MULTITHREADED
#define MPT_PLATFORM_MULTITHREADED 0
#endif

#if (MPT_OS_EMSCRIPTEN && !defined(__EMSCRIPTEN_PTHREADS__))
#undef MPT_PLATFORM_MULTITHREADED
#define MPT_PLATFORM_MULTITHREADED 0
#endif



#if MPT_MSVC_BEFORE(2019, 0) || MPT_GCC_BEFORE(8, 1, 0)
#define MPT_COMPILER_QUIRK_NO_AUTO_TEMPLATE_ARGUMENT
#endif



#if MPT_GCC_BEFORE(11, 1, 0)
#define MPT_COMPILER_QUIRK_NO_STDCPP_THREADS
#elif MPT_CLANG_BEFORE(12, 0, 0)
#define MPT_COMPILER_QUIRK_NO_STDCPP_THREADS
#elif (defined(__MINGW32__) || defined(__MINGW64__)) && MPT_LIBCXX_GNU && defined(_GLIBCXX_HAS_GTHREADS) && !defined(__STDCPP_THREADS__)
#define MPT_COMPILER_QUIRK_NO_STDCPP_THREADS
#endif



#if !MPT_PLATFORM_MULTITHREADED
#define MPT_LIBCXX_QUIRK_NO_STD_THREAD
#elif !MPT_COMPILER_GENERIC && MPT_OS_WINDOWS && MPT_LIBCXX_GNU && !defined(_GLIBCXX_HAS_GTHREADS)
#define MPT_LIBCXX_QUIRK_NO_STD_THREAD
#endif



#if MPT_OS_WINDOWS && MPT_COMPILER_MSVC
#if MPT_WINNT_AT_LEAST(MPT_WIN_VISTA)
#define MPT_LIBCXX_QUIRK_COMPLEX_STD_MUTEX
#endif
#endif



#if MPT_OS_EMSCRIPTEN && defined(MPT_BUILD_AUDIOWORKLETPROCESSOR)
#define MPT_COMPILER_QUIRK_CHRONO_NO_HIGH_RESOLUTION_CLOCK
#endif



#if MPT_OS_EMSCRIPTEN && defined(MPT_BUILD_AUDIOWORKLETPROCESSOR)
#define MPT_COMPILER_QUIRK_RANDOM_NO_RANDOM_DEVICE
#endif



#if MPT_LIBCXX_GNU
#if !defined(_GLIBCXX_USE_WCHAR_T)
#ifndef MPT_COMPILER_QUIRK_NO_WCHAR
#define MPT_COMPILER_QUIRK_NO_WCHAR
#endif
#endif
#endif
#if defined(__MINGW32__) && !defined(__MINGW64__) && (MPT_OS_WINDOWS_WIN9X || MPT_OS_WINDOWS_WIN32)
#ifndef MPT_COMPILER_QUIRK_NO_WCHAR
#define MPT_COMPILER_QUIRK_NO_WCHAR
#endif
#endif



#if MPT_LIBCXX_LLVM_BEFORE(14000)
#define MPT_COMPILER_QUIRK_NO_STRING_VIEW_ITERATOR_CTOR
#endif



#if MPT_LIBCXX_GNU_BEFORE(9)
#define MPT_COMPILER_QUIRK_NO_FILESYSTEM
#endif



#if MPT_OS_WINDOWS && MPT_GCC_BEFORE(9, 1, 0)
// GCC C++ library has no wchar_t overloads
#define MPT_COMPILER_QUIRK_WINDOWS_FSTREAM_NO_WCHAR
#endif



#if MPT_COMPILER_GCC || MPT_COMPILER_CLANG

#if defined(__FAST_MATH__)
#define MPT_COMPILER_QUIRK_FASTMATH 1
#endif
#if defined(__FINITE_MATH_ONLY__)
#if (__FINITE_MATH_ONLY__ >= 1)
#define MPT_COMPILER_QUIRK_FINITEMATH 1
#endif
#endif

#elif MPT_COMPILER_MSVC

#if defined(_M_FP_FAST)
#define MPT_COMPILER_QUIRK_FASTMATH   1
#define MPT_COMPILER_QUIRK_FINITEMATH 1
#endif

#endif

#ifndef MPT_COMPILER_QUIRK_FASTMATH
#define MPT_COMPILER_QUIRK_FASTMATH 0
#endif

#ifndef MPT_COMPILER_QUIRK_FINITEMATH
#define MPT_COMPILER_QUIRK_FINITEMATH 0
#endif



#if MPT_COMPILER_GCC && !defined(__arm__)
#if defined(_SOFT_FLOAT)
#define MPT_COMPILER_QUIRK_FLOAT_EMULATED 1
#endif
#endif

#if defined(__arm__)

#if defined(__SOFTFP__)
#define MPT_COMPILER_QUIRK_FLOAT_EMULATED 1
#else
#define MPT_COMPILER_QUIRK_FLOAT_EMULATED 0
#endif
#if defined(__VFP_FP__)
// native-endian IEEE754
#define MPT_COMPILER_QUIRK_FLOAT_NOTNATIVEENDIAN 0
#define MPT_COMPILER_QUIRK_FLOAT_NOTIEEE754      0
#elif defined(__MAVERICK__)
// little-endian IEEE754, we assume native-endian though
#define MPT_COMPILER_QUIRK_FLOAT_NOTNATIVEENDIAN 1
#define MPT_COMPILER_QUIRK_FLOAT_NOTIEEE754      0
#else
// not IEEE754
#define MPT_COMPILER_QUIRK_FLOAT_NOTNATIVEENDIAN 1
#define MPT_COMPILER_QUIRK_FLOAT_NOTIEEE754      1
#endif

#elif defined(__mips__)

#if defined(__mips_soft_float)
#define MPT_COMPILER_QUIRK_FLOAT_EMULATED 1
#else
#define MPT_COMPILER_QUIRK_FLOAT_EMULATED 0
#endif

#endif

#if MPT_OS_EMSCRIPTEN
#define MPT_COMPILER_QUIRK_FLOAT_PREFER64 1
#endif

#ifndef MPT_COMPILER_QUIRK_FLOAT_PREFER32
#define MPT_COMPILER_QUIRK_FLOAT_PREFER32 0
#endif
#ifndef MPT_COMPILER_QUIRK_FLOAT_PREFER64
#define MPT_COMPILER_QUIRK_FLOAT_PREFER64 0
#endif
#ifndef MPT_COMPILER_QUIRK_FLOAT_EMULATED
#define MPT_COMPILER_QUIRK_FLOAT_EMULATED 0
#endif
#ifndef MPT_COMPILER_QUIRK_FLOAT_NOTNATIVEENDIAN
#define MPT_COMPILER_QUIRK_FLOAT_NOTNATIVEENDIAN 0
#endif
#ifndef MPT_COMPILER_QUIRK_FLOAT_NOTIEEE754
#define MPT_COMPILER_QUIRK_FLOAT_NOTIEEE754 0
#endif



#if MPT_LIBC_MINGW
// MinGW32 runtime headers require __off64_t when including some C and/or C++ stdlib headers.
// This is declared in <sys/types.h>, which howeger is not included in some header chains.
#if (defined(__MINGW32__) && !defined(__MINGW64__))
#define MPT_LIBC_QUIRK_REQUIRES_SYS_TYPES_H
#endif
#endif



#if MPT_LIBC_DJGPP
#define MPT_LIBC_QUIRK_NO_FENV
#endif



#if MPT_OS_CYGWIN
#define MPT_LIBCXX_QUIRK_BROKEN_USER_LOCALE
// #define MPT_LIBCXX_QUIRK_ASSUME_USER_LOCALE_UTF8
#elif MPT_OS_HAIKU
#define MPT_LIBCXX_QUIRK_BROKEN_USER_LOCALE
#define MPT_LIBCXX_QUIRK_ASSUME_USER_LOCALE_UTF8
#endif



// #define MPT_LIBCXX_QUIRK_BROKEN_ACTIVE_LOCALE



#if MPT_OS_WINDOWS && MPT_LIBCXX_GNU
#define MPT_LIBCXX_QUIRK_INCOMPLETE_IS_FUNCTION
#endif



#if MPT_CXX_AT_LEAST(20)
#if MPT_LIBCXX_GNU_BEFORE(10) || MPT_LIBCXX_LLVM_BEFORE(13000) || (MPT_LIBCXX_MS && MPT_MSVC_BEFORE(2022, 0)) || (MPT_LIBCXX_MS && !MPT_COMPILER_MSVC)
#define MPT_LIBCXX_QUIRK_NO_CXX20_CONSTEXPR_ALGORITHM
#endif
#endif



#if MPT_CXX_AT_LEAST(20)
#if MPT_LIBCXX_GNU_BEFORE(12) || MPT_LIBCXX_LLVM_BEFORE(15000) || (MPT_LIBCXX_MS && MPT_MSVC_BEFORE(2022, 0)) || (MPT_LIBCXX_MS && !MPT_COMPILER_MSVC)
#ifndef MPT_LIBCXX_QUIRK_NO_CXX20_CONSTEXPR_CONTAINER
#define MPT_LIBCXX_QUIRK_NO_CXX20_CONSTEXPR_CONTAINER
#endif
#endif
#if MPT_LIBCXX_MS
// So, in 2025, Microsoft still ships a STL that by default is not standard-compliant with its own default Debug options.
// constexpr auto foo = std::vector<int>{}; does not compile with iterator debugging enabled (i.e. in Debug builds).
// See <https://developercommunity.visualstudio.com/t/Iterator-Debugging-breaks-C20-constexp/10861623>.
#if defined(_ITERATOR_DEBUG_LEVEL)
#if (_ITERATOR_DEBUG_LEVEL >= 1)
#ifndef MPT_LIBCXX_QUIRK_NO_CXX20_CONSTEXPR_CONTAINER
#define MPT_LIBCXX_QUIRK_NO_CXX20_CONSTEXPR_CONTAINER
#endif
#endif
#endif
#endif
#endif



#if MPT_CXX_AT_LEAST(20)
// Clang 14 is incompatible with libstdc++ 13 in C++20 mode
#if MPT_CLANG_BEFORE(15, 0, 0) && MPT_LIBCXX_GNU_AT_LEAST(13)
#define MPT_LIBCXX_QUIRK_NO_CHRONO
#endif
#endif

#if MPT_CXX_AT_LEAST(20)
#if MPT_LIBCXX_MS && MPT_OS_WINDOWS
#if MPT_WIN_BEFORE(MPT_WIN_10_1903)
// std::chrono timezones require Windows 10 1903 with VS2022 as of 2022-01-22.
// See <https://github.com/microsoft/STL/issues/1911> and
// <https://github.com/microsoft/STL/issues/2163>.
#define MPT_LIBCXX_QUIRK_NO_CHRONO_DATE
#endif
#endif
#if MPT_LIBCXX_GNU_BEFORE(11)
#define MPT_LIBCXX_QUIRK_NO_CHRONO_DATE
#elif MPT_LIBCXX_LLVM_BEFORE(7000)
#define MPT_LIBCXX_QUIRK_NO_CHRONO_DATE
#endif
#if MPT_LIBCXX_MS && (MPT_MSVC_BEFORE(2022, 2) || !MPT_COMPILER_MSVC)
#elif MPT_LIBCXX_GNU
#define MPT_LIBCXX_QUIRK_NO_CHRONO_DATE_PARSE
#endif
#if MPT_LIBCXX_MS && (MPT_MSVC_BEFORE(2022, 15) || !MPT_COMPILER_MSVC)
// Causes massive memory leaks.
// See
// <https://developercommunity.visualstudio.com/t/stdchronoget-tzdb-list-memory-leak/1644641>
// / <https://github.com/microsoft/STL/issues/2504>.
#define MPT_LIBCXX_QUIRK_CHRONO_TZ_MEMLEAK
#endif
#endif
#if MPT_LIBCXX_GNU_BEFORE(13)
#define MPT_LIBCXX_QUIRK_CHRONO_DATE_NO_ZONED_TIME
#endif
#if MPT_LIBCXX_LLVM
// See <https://github.com/llvm/llvm-project/issues/99982>
#define MPT_LIBCXX_QUIRK_CHRONO_DATE_NO_ZONED_TIME
#endif
#if MPT_MSVC_AT_LEAST(2022, 6) && MPT_MSVC_BEFORE(2022, 7)
// std::chrono triggers ICE in VS2022 17.6.0, see <https://developercommunity.visualstudio.com/t/INTERNAL-COMPILER-ERROR-when-compiling-s/10366948>.
#define MPT_LIBCXX_QUIRK_CHRONO_DATE_BROKEN_ZONED_TIME
#endif



#if MPT_LIBCXX_GNU_BEFORE(8)
#define MPT_LIBCXX_QUIRK_NO_TO_CHARS_INT
#elif MPT_LIBCXX_LLVM_BEFORE(7000)
#define MPT_LIBCXX_QUIRK_NO_TO_CHARS_INT
#elif MPT_OS_ANDROID && MPT_LIBCXX_LLVM_BEFORE(8000)
#define MPT_LIBCXX_QUIRK_NO_TO_CHARS_INT
#elif MPT_OS_MACOSX_OR_IOS
#if defined(TARGET_OS_OSX)
#if TARGET_OS_OSX
#if !defined(MAC_OS_X_VERSION_10_15)
#define MPT_LIBCXX_QUIRK_NO_TO_CHARS_INT
#else
#if (MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_15)
#define MPT_LIBCXX_QUIRK_NO_TO_CHARS_INT
#endif
#endif
#endif
#endif
#endif



#if (MPT_LIBCXX_MS && (MPT_MSVC_BEFORE(2019, 4) || !MPT_COMPILER_MSVC)) || MPT_LIBCXX_GNU_BEFORE(11) || MPT_LIBCXX_LLVM || MPT_LIBCXX_GENERIC
#define MPT_LIBCXX_QUIRK_NO_TO_CHARS_FLOAT
#endif



#if MPT_OS_ANDROID && MPT_LIBCXX_LLVM_BEFORE(7000)
#define MPT_LIBCXX_QUIRK_NO_HAS_UNIQUE_OBJECT_REPRESENTATIONS
#endif



#if MPT_OS_ANDROID && MPT_LIBCXX_LLVM_BEFORE(17000)
#define MPT_LIBCXX_QUIRK_NO_NUMBERS
#endif



#if MPT_LIBCXX_GNU_BEFORE(13) || (MPT_LIBCXX_MS && !MPT_MSVC_AT_LEAST(2022, 7)) || MPT_LIBCXX_LLVM
#define MPT_LIBCXX_QUIRK_NO_STDFLOAT
#endif



#if MPT_OS_MACOSX_OR_IOS
#if defined(TARGET_OS_OSX)
#if TARGET_OS_OSX
#if !defined(MAC_OS_X_VERSION_10_14)
#define MPT_LIBCXX_QUIRK_NO_OPTIONAL_VALUE
#else
#if (MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_14)
#define MPT_LIBCXX_QUIRK_NO_OPTIONAL_VALUE
#endif
#endif
#endif
#endif
#endif



#endif // MPT_BASE_DETECT_QUIRKS_HPP
