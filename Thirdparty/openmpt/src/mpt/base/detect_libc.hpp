/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_BASE_DETECT_LIBC_HPP
#define MPT_BASE_DETECT_LIBC_HPP



#include "mpt/base/detect_compiler.hpp"
#include "mpt/base/detect_os.hpp"

#include <cstddef>



// Version numbers try to follow MinGW32
// <https://sourceforge.net/p/mingw/mingw-org-wsl/ci/5.1-trunk/tree/mingwrt/include/msvcrtver.h>,
// and MinGW-w64 version numbers are adjusted for
// <https://github.com/mingw-w64/mingw-w64/commit/94a28397eec79e54ce5a8354512f43da88f815d2>,
#define MPT_LIBC_MS_VER_CRTDLL   0x0000
#define MPT_LIBC_MS_VER_MSVCR10  0x0100
#define MPT_LIBC_MS_VER_MSVCR20  0x0200
#define MPT_LIBC_MS_VER_MSVCR40  0x0400
#define MPT_LIBC_MS_VER_MSVCRT40 0x0400
#define MPT_LIBC_MS_VER_MSVCRT   0x0600
#define MPT_LIBC_MS_VER_MSVCR60  0x0600
#define MPT_LIBC_MS_VER_MSVCR61  0x0601
#define MPT_LIBC_MS_VER_MSVCR70  0x0700
#define MPT_LIBC_MS_VER_MSVCR71  0x0701
#define MPT_LIBC_MS_VER_MSVCR80  0x0800
#define MPT_LIBC_MS_VER_MSVCR90  0x0900
#define MPT_LIBC_MS_VER_MSVCR100 0x1000
#define MPT_LIBC_MS_VER_MSVCR110 0x1100
#define MPT_LIBC_MS_VER_MSVCR120 0x1200
#define MPT_LIBC_MS_VER_UCRT     0x1400



// order of checks is important!
#if MPT_COMPILER_GENERIC
#define MPT_LIBC_GENERIC 1
#elif (defined(__MINGW32__) || defined(__MINGW64__))
#define MPT_LIBC_MINGW 1
#if defined(__MINGW64_VERSION_MAJOR) && defined(__MINGW64_VERSION_MINOR)
#define MPT_LIBC_MINGW_MINGWW64                        1
#define MPT_LIBC_MINGW_MINGWW64_AT_LEAST(major, minor) (((major) > __MINGW64_VERSION_MAJOR) || (((major) == __MINGW64_VERSION_MAJOR) && ((minor) >= __MINGW64_VERSION_MINOR)))
#define MPT_LIBC_MINGW_MINGWW64_BEFORE(major, minor)   (((major) < __MINGW64_VERSION_MAJOR) || (((major) == __MINGW64_VERSION_MAJOR) && ((minor) < __MINGW64_VERSION_MINOR)))
#elif defined(__MINGW64_VERSION_MAJOR)
#define MPT_LIBC_MINGW_MINGWW64                        1
#define MPT_LIBC_MINGW_MINGWW64_AT_LEAST(major, minor) ((major) >= __MINGW64_VERSION_MAJOR)
#define MPT_LIBC_MINGW_MINGWW64_BEFORE(major, minor)   ((major) < __MINGW64_VERSION_MAJOR)
#elif defined(__MINGW64_VERSION_MINOR)
#define MPT_LIBC_MINGW_MINGWW64                        1
#define MPT_LIBC_MINGW_MINGWW64_AT_LEAST(major, minor) 0
#define MPT_LIBC_MINGW_MINGWW64_BEFORE(major, minor)   1
#elif defined(__MINGW32_VERSION_MAJOR) && defined(__MINGW32_VERSION_MINOR)
#define MPT_LIBC_MINGW_MINGW32                        1
#define MPT_LIBC_MINGW_MINGW32_AT_LEAST(major, minor) (((major) > __MINGW32_VERSION_MAJOR) || (((major) == __MINGW32_VERSION_MAJOR) && ((minor) >= __MINGW32_VERSION_MINOR)))
#define MPT_LIBC_MINGW_MINGW32_BEFORE(major, minor)   (((major) < __MINGW32_VERSION_MAJOR) || (((major) == __MINGW32_VERSION_MAJOR) && ((minor) < __MINGW32_VERSION_MINOR)))
#elif defined(__MINGW32_VERSION_MAJOR)
#define MPT_LIBC_MINGW_MINGW32                        1
#define MPT_LIBC_MINGW_MINGW32_AT_LEAST(major, minor) ((major) >= __MINGW32_VERSION_MAJOR)
#define MPT_LIBC_MINGW_MINGW32_BEFORE(major, minor)   ((major) < __MINGW32_VERSION_MAJOR)
#elif defined(__MINGW32_VERSION_MINOR)
#define MPT_LIBC_MINGW_MINGW32                        1
#define MPT_LIBC_MINGW_MINGW32_AT_LEAST(major, minor) 0
#define MPT_LIBC_MINGW_MINGW32_BEFORE(major, minor)   1
#endif
#if defined(MPT_LIBC_MINGW_MINGW32)
#ifdef __MSVCR60_DLL
static_assert(__MSVCR60_DLL == MPT_LIBC_MS_VER_MSVCR60);
#endif
#ifdef __MSVCR61_DLL
static_assert(__MSVCR61_DLL == MPT_LIBC_MS_VER_MSVCR61);
#endif
#ifdef __MSVCR70_DLL
static_assert(__MSVCR70_DLL == MPT_LIBC_MS_VER_MSVCR70);
#endif
#ifdef __MSVCR71_DLL
static_assert(__MSVCR71_DLL == MPT_LIBC_MS_VER_MSVCR71);
#endif
#ifdef __MSVCR80_DLL
static_assert(__MSVCR80_DLL == MPT_LIBC_MS_VER_MSVCR80);
#endif
#ifdef __MSVCR90_DLL
static_assert(__MSVCR90_DLL == MPT_LIBC_MS_VER_MSVCR90);
#endif
#ifdef __MSVCR100_DLL
static_assert(__MSVCR100_DLL == MPT_LIBC_MS_VER_MSVCR100);
#endif
#ifdef __MSVCR110_DLL
static_assert(__MSVCR110_DLL == MPT_LIBC_MS_VER_MSVCR110);
#endif
#ifdef __MSVCR120_DLL
static_assert(__MSVCR120_DLL == MPT_LIBC_MS_VER_MSVCR120);
#endif
#endif
#if defined(_UCRT)
#define MPT_LIBC_MS_UCRT    1
#define MPT_LIBC_MS_VERSION MPT_LIBC_MS_VER_UCRT
#elif defined(__MSVCRT__)
#define MPT_LIBC_MS_MSVCRT 1
#if defined(__MSVCRT_VERSION__)
#if defined(MPT_LIBC_MINGW_MINGWW64)
#if (__MSVCRT_VERSION__ > 0x09ff)
// MinGW-w64 completely fucked up version numbers in
// <https://github.com/mingw-w64/mingw-w64/commit/94a28397eec79e54ce5a8354512f43da88f815d2>,
// so we need to compensate for that mother of brainfarts.
#define MPT_LIBC_MS_VERSION (__MSVCRT_VERSION__ + (0x1000 - 0x0A00))
#else
#define MPT_LIBC_MS_VERSION __MSVCRT_VERSION__
#endif
#define MPT_LIBC_MS_VERSION __MSVCRT_VERSION__
#elif defined(MPT_LIBC_MINGW_MINGW32)
#define MPT_LIBC_MS_VERSION __MSVCRT_VERSION__
#else
#define MPT_LIBC_MS_VERSION MPT_LIBC_MS_VER_MSVCRT
#endif
#endif
#elif defined(__CRTDLL__)
#define MPT_LIBC_MS_CRTDLL  1
#define MPT_LIBC_MS_VERSION MPT_LIBC_MS_VER_CRTDLL
#endif
#elif (defined(__GLIBC__) || defined(__GNU_LIBRARY__))
#define MPT_LIBC_GLIBC 1
#elif defined(_UCRT)
#define MPT_LIBC_MS         1
#define MPT_LIBC_MS_UCRT    1
#define MPT_LIBC_MS_VERSION MPT_LIBC_MS_VER_UCRT
#elif MPT_COMPILER_MSVC
#define MPT_LIBC_MS        1
#define MPT_LIBC_MS_MSVCRT 1
#if MPT_MSVC_AT_LEAST(2015, 0)
#define MPT_LIBC_MS_VERSION MPT_LIBC_MS_VER_UCRT
#elif MPT_MSVC_AT_LEAST(2013, 0)
#define MPT_LIBC_MS_VERSION MPT_LIBC_MS_VER_MSVCR120
#elif MPT_MSVC_AT_LEAST(2012, 0)
#define MPT_LIBC_MS_VERSION MPT_LIBC_MS_VER_MSVCR110
#elif MPT_MSVC_AT_LEAST(2010, 0)
#define MPT_LIBC_MS_VERSION MPT_LIBC_MS_VER_MSVCR100
#elif MPT_MSVC_AT_LEAST(2008, 0)
#define MPT_LIBC_MS_VERSION MPT_LIBC_MS_VER_MSVCR90
#elif MPT_MSVC_AT_LEAST(2005, 0)
#define MPT_LIBC_MS_VERSION MPT_LIBC_MS_VER_MSVCR80
#elif MPT_MSVC_AT_LEAST(2003, 0)
#define MPT_LIBC_MS_VERSION MPT_LIBC_MS_VER_MSVCR71
#elif MPT_MSVC_AT_LEAST(2002, 0)
#define MPT_LIBC_MS_VERSION MPT_LIBC_MS_VER_MSVCR70
#else
#define MPT_LIBC_MS_VERSION MPT_LIBC_MS_VER_MSVCRT
#endif
#elif MPT_COMPILER_CLANG && MPT_OS_WINDOWS
#define MPT_LIBC_MS 1
#elif defined(__BIONIC__)
#define MPT_LIBC_BIONIC 1
#elif defined(__APPLE__)
#define MPT_LIBC_APPLE 1
#elif defined(__DJGPP__)
#define MPT_LIBC_DJGPP 1
#else
#define MPT_LIBC_GENERIC 1
#endif

#if defined(MPT_LIBC_MS_VERSION)
#define MPT_LIBC_MS_AT_LEAST(v) (MPT_LIBC_MS_VERSION >= (v))
#define MPT_LIBC_MS_BEFORE(v)   (MPT_LIBC_MS_VERSION < (v))
#endif

#ifndef MPT_LIBC_GENERIC
#define MPT_LIBC_GENERIC 0
#endif
#ifndef MPT_LIBC_GLIBC
#define MPT_LIBC_GLIBC 0
#endif
#ifndef MPT_LIBC_MINGW
#define MPT_LIBC_MINGW 0
#endif
#ifndef MPT_LIBC_MINGW_MINGWW64
#define MPT_LIBC_MINGW_MINGWW64 0
#endif
#ifndef MPT_LIBC_MINGW_MINGW32
#define MPT_LIBC_MINGW_MINGW32 0
#endif
#ifndef MPT_LIBC_MINGW_MINGWW64_AT_LEAST
#define MPT_LIBC_MINGW_MINGWW64_AT_LEAST(major, minor) 0
#endif
#ifndef MPT_LIBC_MINGW_MINGWW64_BEFORE
#define MPT_LIBC_MINGW_MINGWW64_BEFORE(major, minor) 0
#endif
#ifndef MPT_LIBC_MINGW_MINGW32_AT_LEAST
#define MPT_LIBC_MINGW_MINGW32_AT_LEAST(major, minor) 0
#endif
#ifndef MPT_LIBC_MINGW_MINGW32_BEFORE
#define MPT_LIBC_MINGW_MINGW32_BEFORE(major, minor) 0
#endif
#ifndef MPT_LIBC_MS
#define MPT_LIBC_MS 0
#endif
#ifndef MPT_LIBC_MS_UCRT
#define MPT_LIBC_MS_UCRT 0
#endif
#ifndef MPT_LIBC_MS_MSVCRT
#define MPT_LIBC_MS_MSVCRT 0
#endif
#ifndef MPT_LIBC_MS_CRTDLL
#define MPT_LIBC_MS_CRTDLL 0
#endif
#ifndef MPT_LIBC_MS_AT_LEAST
#define MPT_LIBC_MS_AT_LEAST(v) 0
#endif
#ifndef MPT_LIBC_MS_BEFORE
#define MPT_LIBC_MS_BEFORE(v) 0
#endif
#ifndef MPT_LIBC_BIONIC
#define MPT_LIBC_BIONIC 0
#endif
#ifndef MPT_LIBC_APPLE
#define MPT_LIBC_APPLE 0
#endif
#ifndef MPT_LIBC_DJGPP
#define MPT_LIBC_DJGPP 0
#endif

#if MPT_LIBC_MS
#if defined(_DEBUG)
#define MPT_LIBC_MS_DEBUG 1
#endif
#if defined(_DLL)
#define MPT_LIBC_MS_SHARED 1
#define MPT_LIBC_MS_STATIC 0
#else
#define MPT_LIBC_MS_SHARED 0
#define MPT_LIBC_MS_STATIC 1
#endif
#endif

#ifndef MPT_LIBC_MS_DEBUG
#define MPT_LIBC_MS_DEBUG 0
#endif
#ifndef MPT_LIBC_MS_SHARED
#define MPT_LIBC_MS_SHARED 0
#endif
#ifndef MPT_LIBC_MS_STATIC
#define MPT_LIBC_MS_STATIC 0
#endif



#endif // MPT_BASE_DETECT_LIBC_HPP
