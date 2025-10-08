/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_DETECT_DL_HPP
#define MPT_DETECT_DL_HPP

#include "mpt/base/compiletime_warning.hpp"

#if defined(MPT_WITH_DL)
#if !defined(CPPCHECK)
#if !__has_include(<dlfcn.h>)
#error "MPT_WITH_DL defined but <dlfcn.h> not found."
#endif
#endif
#define MPT_DETECTED_DL 1
#else
#if defined(CPPCHECK)
#define MPT_DETECTED_DL 1
#else
#if __has_include(<dlfcn.h>)
#define MPT_DETECTED_DL 1
#else
#define MPT_DETECTED_DL 0
#endif
#endif
#endif

#endif // MPT_DETECT_DL_HPP
