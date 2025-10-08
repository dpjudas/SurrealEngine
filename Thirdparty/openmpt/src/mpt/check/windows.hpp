/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_CHECK_WINDOWS_HPP
#define MPT_CHECK_WINDOWS_HPP

#include "mpt/base/detect_os.hpp"
#include "mpt/base/compiletime_warning.hpp"

#if MPT_OS_WINDOWS

#ifndef MPT_CHECK_WINDOWS_IGNORE_WARNING_NO_UNICODE
#if MPT_OS_WINDOWS_WINNT
#ifndef UNICODE
MPT_WARNING("windows.h uses MBCS TCHAR. Please #define UNICODE.")
#endif
#elif MPT_OS_WINDOWS_WIN9X
#ifdef UNICODE
MPT_WARNING("Targeting Win9x but windows.h uses UNICODE TCHAR. Please do not #define UNICODE.")
#endif
#endif
#endif

#ifndef MPT_CHECK_WINDOWS_IGNORE_WARNING_UNICODE_MISMATCH
#ifdef _UNICODE
#ifndef UNICODE
MPT_WARNING("_UNICODE is defined but UNICODE is not defined. Please enable UNICODE support in your compiler, or do not #define _UNICODE.")
#endif
#endif
#endif

#ifndef NOMINMAX
#ifndef MPT_CHECK_WINDOWS_IGNORE_WARNING_NO_NOMINMAX
MPT_WARNING("windows.h defines min and max which conflicts with C++. Please #define NOMINMAX.")
#endif
#endif

#endif // MPT_OS_WINDOWS

#endif // MPT_CHECK_WINDOWS_HPP
