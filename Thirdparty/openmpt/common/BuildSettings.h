/*
 * BuildSettings.h
 * ---------------
 * Purpose: Global, user settable compile time flags (and some global system header configuration)
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#pragma once



#include "BuildSettingsCompiler.h"



#include "mpt/base/detect_arch.hpp"
#include "mpt/base/detect_compiler.hpp"
#include "mpt/base/detect_os.hpp"
#include "mpt/base/detect_quirks.hpp"



#if defined(MODPLUG_TRACKER) && defined(LIBOPENMPT_BUILD)
#error "either MODPLUG_TRACKER or LIBOPENMPT_BUILD has to be defined"
#elif defined(MODPLUG_TRACKER)
// nothing
#define MPT_INLINE_NS mptx
#elif defined(LIBOPENMPT_BUILD)
// nothing
#define MPT_INLINE_NS mpt_libopenmpt
#else
#error "either MODPLUG_TRACKER or LIBOPENMPT_BUILD has to be defined"
#endif // MODPLUG_TRACKER || LIBOPENMPT_BUILD



#if defined(MODPLUG_TRACKER)

#if defined(MPT_BUILD_RETRO)
#define OPENMPT_BUILD_VARIANT "Retro"
#define OPENMPT_BUILD_VARIANT_MONIKER " RETRO"
#else
#if MPT_OS_WINDOWS
#if MPT_WINNT_AT_LEAST(MPT_WIN_10)
#define OPENMPT_BUILD_VARIANT "Standard"
#define OPENMPT_BUILD_VARIANT_MONIKER ""
#else
#define OPENMPT_BUILD_VARIANT "Legacy"
#define OPENMPT_BUILD_VARIANT_MONIKER ""
#endif
#else
#define OPENMPT_BUILD_VARIANT "Unknown"
#define OPENMPT_BUILD_VARIANT_MONIKER ""
#endif
#endif

#define MPT_WITH_DMO

#define MPT_WITH_VST

#if MPT_OS_WINDOWS
#if MPT_WINNT_AT_LEAST(MPT_WIN_7)
#define MPT_WITH_MEDIAFOUNDATION
#endif
#endif

#if MPT_OS_WINDOWS
#if MPT_WINNT_AT_LEAST(MPT_WIN_10)
#define MPT_WITH_WINDOWS10
#endif
#endif

#endif // MODPLUG_TRACKER



#if defined(MODPLUG_TRACKER)

// Enable built-in test suite.
#if defined(MPT_BUILD_DEBUG) || defined(MPT_BUILD_CHECKED)
#define ENABLE_TESTS
#endif

// Enable generation and verification of playback traces
#define MPT_ENABLE_PLAYBACK_TRACE

// Disable any file saving functionality (not really useful except for the player library)
//#define MODPLUG_NO_FILESAVE

// Disable any debug logging
#if !defined(MPT_BUILD_DEBUG) && !defined(MPT_BUILD_CHECKED) && !defined(MPT_BUILD_WINESUPPORT)
#define MPT_LOG_GLOBAL_LEVEL_STATIC
#define MPT_LOG_GLOBAL_LEVEL 0
#endif

// Enable all individual logging macros and MPT_LOG calls
//#define MPT_ALL_LOGGING

// Disable all runtime asserts
#if !defined(MPT_BUILD_DEBUG) && !defined(MPT_BUILD_CHECKED) && !defined(MPT_BUILD_WINESUPPORT)
#define NO_ASSERTS
#endif

// Enable global ComponentManager
#define MPT_COMPONENT_MANAGER 1

// Support for externally linked samples e.g. in MPTM files
#define MPT_EXTERNAL_SAMPLES

// Support mpt::CharsetLocale
#define MPT_ENABLE_CHARSET_LOCALE

// Use architecture-specific intrinsics
#define MPT_ENABLE_ARCH_INTRINSICS

#define MPT_ENABLE_UPDATE

#if defined(MPT_BUILD_DEBUG)
#define MPT_ENABLE_PLAYBACK_TEST_MENU
#endif

// Disable unarchiving support
//#define NO_ARCHIVE_SUPPORT

// Disable the built-in reverb effect
//#define NO_REVERB

// Disable built-in miscellaneous DSP effects (surround, mega bass, noise reduction)
//#define NO_DSP

// Disable the built-in equalizer.
//#define NO_EQ

// Disable the built-in automatic gain control
//#define NO_AGC

// (HACK) Define to build without any plugin support
//#define NO_PLUGINS

#endif // MODPLUG_TRACKER



#if defined(LIBOPENMPT_BUILD)

#ifdef MPT_WITH_FLAC
#error "Building libopenmpt with FLAC is useless and not a supported configuration. Please fix your build system to not list libflac as a dependency for libopenmpt itself. It is only a dependency of openmpt123."
#endif

#ifndef LIBOPENMPT_NO_DEPRECATE
#define LIBOPENMPT_NO_DEPRECATE
#endif

#if (defined(_DEBUG) || defined(DEBUG)) && !defined(MPT_BUILD_DEBUG)
#define MPT_BUILD_DEBUG
#endif

#if defined(LIBOPENMPT_BUILD_TEST)
#define ENABLE_TESTS
#define MPT_ENABLE_PLAYBACK_TRACE
#else
#define MODPLUG_NO_FILESAVE
#endif
#if defined(MPT_BUILD_ANALZYED) || defined(MPT_BUILD_DEBUG) || defined(MPT_BUILD_CHECKED) || defined(ENABLE_TESTS)
// enable asserts
#else
#define NO_ASSERTS
#endif
//#define MPT_ALL_LOGGING
#define MPT_COMPONENT_MANAGER 0
//#define MPT_EXTERNAL_SAMPLES
#if defined(ENABLE_TESTS) || defined(MPT_BUILD_HACK_ARCHIVE_SUPPORT)
#define MPT_ENABLE_CHARSET_LOCALE
#else
//#define MPT_ENABLE_CHARSET_LOCALE
#endif
// Do not use architecture-specifid intrinsics in library builds. There is just about no codepath which would use it anyway.
//#define MPT_ENABLE_ARCH_INTRINSICS
#if defined(MPT_BUILD_HACK_ARCHIVE_SUPPORT)
//#define NO_ARCHIVE_SUPPORT
#else
#define NO_ARCHIVE_SUPPORT
#endif
//#define NO_REVERB
#define NO_DSP
#define NO_EQ
#define NO_AGC
//#define NO_PLUGINS

#endif // LIBOPENMPT_BUILD



#if MPT_OS_WINDOWS

	#ifndef MPT_ENABLE_CHARSET_LOCALE
	#define MPT_ENABLE_CHARSET_LOCALE
	#endif

#elif MPT_OS_LINUX

#elif MPT_OS_ANDROID

#elif MPT_OS_EMSCRIPTEN

#elif MPT_OS_MACOSX_OR_IOS

#elif MPT_OS_DJGPP

#endif



#define MPT_TIME_UTC_ON_DISK 0
#define MPT_TIME_UTC_ON_DISK_VERSION MPT_V("1.31.00.13")



// fixing stuff up

#if defined(MPT_BUILD_ANALYZED) || defined(MPT_BUILD_CHECKED) 
#ifdef NO_ASSERTS
#undef NO_ASSERTS // static or dynamic analyzers want assertions on
#endif
#endif

#if defined(MPT_BUILD_FUZZER)
#ifndef MPT_FUZZ_TRACKER
#define MPT_FUZZ_TRACKER
#endif
#endif

#if defined(MPT_ENABLE_ARCH_INTRINSICS)
#if MPT_ARCH_X86

#define MPT_ENABLE_ARCH_X86

#define MPT_WANT_ARCH_INTRINSICS_X86_SSE
#define MPT_WANT_ARCH_INTRINSICS_X86_SSE2

#elif MPT_ARCH_AMD64

#define MPT_ENABLE_ARCH_AMD64

#define MPT_WANT_ARCH_INTRINSICS_X86_SSE
#define MPT_WANT_ARCH_INTRINSICS_X86_SSE2

#endif // arch
#endif // MPT_ENABLE_ARCH_INTRINSICS

#if defined(ENABLE_TESTS) && defined(MODPLUG_NO_FILESAVE)
#undef MODPLUG_NO_FILESAVE // tests recommend file saving
#endif

#if defined(MPT_WITH_ZLIB) && defined(MPT_WITH_MINIZ)
// Only one deflate implementation should be used. Prefer zlib.
#undef MPT_WITH_MINIZ
#endif

#if !MPT_OS_WINDOWS && defined(MPT_WITH_MEDIAFOUNDATION)
#undef MPT_WITH_MEDIAFOUNDATION // MediaFoundation requires Windows
#endif

#if !MPT_COMPILER_MSVC && !MPT_COMPILER_CLANG && defined(MPT_WITH_MEDIAFOUNDATION)
#undef MPT_WITH_MEDIAFOUNDATION // MediaFoundation requires MSVC or Clang due to ATL (no MinGW support)
#endif

#if (defined(MPT_WITH_MPG123) || defined(MPT_WITH_MINIMP3)) && !defined(MPT_ENABLE_MP3_SAMPLES)
#define MPT_ENABLE_MP3_SAMPLES
#endif

#if defined(ENABLE_TESTS)
#define MPT_ENABLE_FILEIO // Test suite requires PathString for file loading.
#endif

#if defined(MODPLUG_TRACKER) && !defined(MPT_ENABLE_FILEIO)
#define MPT_ENABLE_FILEIO // Tracker requires disk file io
#endif

#if defined(MPT_EXTERNAL_SAMPLES) && !defined(MPT_ENABLE_FILEIO)
#define MPT_ENABLE_FILEIO // External samples require disk file io
#endif

#if defined(NO_PLUGINS)
// Any plugin type requires NO_PLUGINS to not be defined.
#if defined(MPT_WITH_VST)
#undef MPT_WITH_VST
#endif
#endif



#if defined(MODPLUG_TRACKER) && !defined(MPT_BUILD_WINESUPPORT) && !defined(MPT_BUILD_WINESUPPORT_WRAPPER)
#ifndef MPT_NO_NAMESPACE
#define MPT_NO_NAMESPACE
#endif
#endif

#if defined(MPT_NO_NAMESPACE)

#ifdef OPENMPT_NAMESPACE
#undef OPENMPT_NAMESPACE
#endif
#define OPENMPT_NAMESPACE

#ifdef OPENMPT_NAMESPACE_BEGIN
#undef OPENMPT_NAMESPACE_BEGIN
#endif
#define OPENMPT_NAMESPACE_BEGIN

#ifdef OPENMPT_NAMESPACE_END
#undef OPENMPT_NAMESPACE_END
#endif
#define OPENMPT_NAMESPACE_END

#else

#ifndef OPENMPT_NAMESPACE
#define OPENMPT_NAMESPACE OpenMPT
#endif

#ifndef OPENMPT_NAMESPACE_BEGIN
#define OPENMPT_NAMESPACE_BEGIN namespace OPENMPT_NAMESPACE {
#endif
#ifndef OPENMPT_NAMESPACE_END
#define OPENMPT_NAMESPACE_END   }
#endif

#endif

#define MPT_CONFIGURATION_IO_READ_FILEDATA_NO_64BIT



// platform configuration

#if MPT_OS_WINDOWS

#define WIN32_LEAN_AND_MEAN

// windows.h excludes
#define NOMEMMGR          // GMEM_*, LMEM_*, GHND, LHND, associated routines
#ifndef NOMINMAX
#define NOMINMAX          // Macros min(a,b) and max(a,b)
#endif
#define NOSERVICE         // All Service Controller routines, SERVICE_ equates, etc.
#define NOCOMM            // COMM driver routines
#define NOKANJI           // Kanji support stuff.
#define NOPROFILER        // Profiler interface.
#define NOMCX             // Modem Configuration Extensions

// mmsystem.h excludes
#define MMNODRV
//#define MMNOSOUND
//#define MMNOWAVE
//#define MMNOMIDI
#define MMNOAUX
#define MMNOMIXER
//#define MMNOTIMER
#define MMNOJOY
#define MMNOMCI
//#define MMNOMMIO
//#define MMNOMMSYSTEM

// mmreg.h excludes
#define NOMMIDS
//#define NONEWWAVE
#define NONEWRIFF
#define NOJPEGDIB
#define NONEWIC
#define NOBITMAP

#endif // MPT_OS_WINDOWS



// third-party library configuration

#ifdef MPT_WITH_STBVORBIS
#ifndef STB_VORBIS_HEADER_ONLY
#define STB_VORBIS_HEADER_ONLY
#endif
#endif

#ifdef MPT_WITH_VORBISFILE
#ifndef OV_EXCLUDE_STATIC_CALLBACKS
#define OV_EXCLUDE_STATIC_CALLBACKS
#endif
#endif



#ifdef __cplusplus

#include "mpt/base/namespace.hpp"

OPENMPT_NAMESPACE_BEGIN

namespace mpt {

#ifndef MPT_NO_NAMESPACE
using namespace ::mpt;
#endif

} // namespace mpt

OPENMPT_NAMESPACE_END

#endif
