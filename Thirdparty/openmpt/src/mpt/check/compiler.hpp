/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_CHECK_COMPILER_HPP
#define MPT_CHECK_COMPILER_HPP

#include "mpt/base/detect_compiler.hpp"
#include "mpt/base/detect_quirks.hpp"
#include "mpt/base/compiletime_warning.hpp"

#ifndef MPT_CHECK_CXX_IGNORE_PREPROCESSOR
#if defined(MPT_COMPILER_QUIRK_MSVC_OLD_PREPROCESSOR)
MPT_WARNING("C++ preprocessor is not standard conformings.")
#endif
#endif

#ifndef MPT_CHECK_CXX_IGNORE_WARNING_FASTMATH
#if MPT_COMPILER_MSVC
#if (defined(_M_FP_FAST) && (_M_FP_FAST == 1))
MPT_WARNING("C++ compiler has fast-math support enabled. This is not standard-conforming.")
#endif
#elif MPT_COMPILER_GCC || MPT_COMPILER_CLANG
#if (defined(__FAST_MATH__) && (__FAST_MATH__ == 1))
MPT_WARNING("C++ compiler has fast-math support enabled. This is not standard-conforming.")
#endif
#endif
#endif

#ifndef MPT_CHECK_CXX_IGNORE_WARNING_FINITEMATH
#if MPT_COMPILER_MSVC
#if (defined(_M_FP_FAST) && (_M_FP_FAST == 1))
MPT_WARNING("C++ compiler assumes finite math only. This is not standard-conforming.")
#endif
#elif MPT_COMPILER_GCC || MPT_COMPILER_CLANG
#if (defined(__FINITE_MATH_ONLY__) && (__FINITE_MATH_ONLY__ == 1))
MPT_WARNING("C++ compiler assumes finite math only. This is not standard-conforming.")
#endif
#endif
#endif

#ifndef MPT_CHECK_CXX_IGNORE_WARNING_NO_EXCEPTIONS
#if MPT_COMPILER_MSVC
#if !defined(_CPPUNWIND)
MPT_WARNING("C++ compiler has no exception support.")
#endif
#elif MPT_COMPILER_GCC
#if (!defined(__EXCEPTIONS) || (__EXCEPTIONS != 1))
MPT_WARNING("C++ compiler has no exception support.")
#endif
#elif MPT_COMPILER_CLANG
#if (!__has_feature(cxx_exceptions) && (!defined(__EXCEPTIONS) || (__EXCEPTIONS != 1)) && !defined(_CPPUNWIND))
MPT_WARNING("C++ compiler has no exception support.")
#else
#if (MPT_CXX_AT_LEAST(20) && !defined(__cpp_exceptions))
MPT_WARNING("C++ compiler has no exception support.")
#endif
#endif
#endif
#endif

#ifndef MPT_CHECK_CXX_IGNORE_WARNING_NO_RTTI
#if MPT_COMPILER_MSVC
#if (!defined(_CPPRTTI) || (_CPPRTTI != 1))
MPT_WARNING("C++ compiler has no RTTI support.")
#endif
#elif MPT_COMPILER_GCC
#if (!defined(__GXX_RTTI) || (__GXX_RTTI != 1))
MPT_WARNING("C++ compiler has no RTTI support.")
#endif
#elif MPT_COMPILER_CLANG && !defined(_MSC_VER)
#if (!defined(__GXX_RTTI) || (__GXX_RTTI != 1))
MPT_WARNING("C++ compiler has no RTTI support.")
#endif
#endif
#endif

#ifndef MPT_CHECK_CXX_IGNORE_WARNING_NO_UNICODE
#if !defined(MPT_COMPILER_QUIRK_NO_WCHAR)
#if (!defined(__STDC_ISO_10646__) || (__STDC_ISO_10646__ <= 198700L))
#if !MPT_COMPILER_MSVC && !MPT_COMPILER_GCC && !MPT_COMPILER_CLANG
// Disabled for all known compilers, as none of them defines __STDC_ISO_10646__, even though all of them provide Unicode wchar_t.
MPT_WARNING("C++ compiler uses non-Unicode wchar_t.")
#endif
#endif
#endif
#endif

#ifndef MPT_CHECK_CXX_IGNORE_WARNING_EBCDIC
#if defined(__STDC_MB_MIGHT_NEQ_WC__) && (__STDC_MB_MIGHT_NEQ_WC__ == 1)
#if !(MPT_COMPILER_CLANG && MPT_OS_FREEBSD)
// Disabled on FreeBSD because <https://github.com/llvm/llvm-project/blob/c7a56af3072c2fa89f0968d7f00b22f7bff0812b/clang/lib/Basic/Targets/OSTargets.h#L225>.
MPT_WARNING("C++ compiler uses a weird 8bit charset, maybe EBCDIC.")
#endif
#endif
#endif

#ifndef MPT_CHECK_CXX_IGNORE_WARNING_NO_STDCPP_THREADS
#if MPT_PLATFORM_MULTITHREADED
#if !defined(MPT_COMPILER_QUIRK_NO_STDCPP_THREADS)
#if !defined(__STDCPP_THREADS__)
MPT_WARNING("C++ __STDCPP_THREADS__ is not defined. Non-conforming compiler detected.")
#else
#if (__STDCPP_THREADS__ != 1)
MPT_WARNING("C++ compiler has no thread support.")
#endif
#endif
#endif
#endif
#endif

#ifndef MPT_CHECK_CXX_IGNORE_WARNING_SINGLETHREADED_THREADSAFE_STATICS
#if !MPT_PLATFORM_MULTITHREADED
#if MPT_CXX_AT_LEAST(20) && MPT_COMPILER_GCC
#if (defined(__cpp_threadsafe_static_init) && (__cpp_threadsafe_static_init >= 200806L))
MPT_WARNING("C++ compiler provides threadsafe initialization of static variables, however the platform is single-threaded.")
#endif
#endif
#endif
#endif

#endif // MPT_CHECK_COMPILER_HPP
