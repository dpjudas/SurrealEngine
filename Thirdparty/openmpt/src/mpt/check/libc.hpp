/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_CHECK_LIBC_HPP
#define MPT_CHECK_LIBC_HPP

#include "mpt/base/detect_libc.hpp"
#include "mpt/base/detect_os.hpp"
#include "mpt/base/detect_quirks.hpp"
#include "mpt/base/compiletime_warning.hpp"

#ifndef MPT_CHECK_LIBC_IGNORE_WARNING_NO_MTRT
#if MPT_PLATFORM_MULTITHREADED
#if MPT_LIBC_MINGW
// MinGW only has `#define _MT` in header files instead of `#define _MT 1`.
#if !defined(_MT)
MPT_WARNING("C stdlib is not multi-threaded.")
#endif
#elif MPT_LIBC_MS
#if defined(_MT)
#if (_MT != 1)
MPT_WARNING("C stdlib is not multi-threaded.")
#endif
#else
MPT_WARNING("C stdlib is not multi-threaded.")
#endif
//#elif !MPT_LIBC_MS && !MPT_LIBC_MINGW && !MPT_LIBC_GENERIC
//#if (!defined(_REENTRANT) || (_REENTRANT != 1))
//MPT_WARNING("C stdlib is not multi-threaded.")
//#endif
#endif
#endif
#endif

#ifndef MPT_CHECK_LIBC_IGNORE_WARNING_UNICODE_MISMATCH
#if MPT_OS_WINDOWS
#ifdef UNICODE
#ifndef _UNICODE
MPT_WARNING("UNICODE is defined but _UNICODE is not defined. Please #define _UNICODE.")
#endif
#endif
#endif // MPT_OS_WINDOWS
#endif

#endif // MPT_CHECK_LIBC_HPP
