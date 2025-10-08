/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_DETECT_LTDL_HPP
#define MPT_DETECT_LTDL_HPP

#include "mpt/base/compiletime_warning.hpp"

#if defined(MPT_WITH_LTDL)
#if !defined(CPPCHECK)
#if !__has_include(<ltdl.h>)
#error "MPT_WITH_LTDL defined but <ltdl.h> not found."
#endif
#endif
#define MPT_DETECTED_LTDL 1
#else
#if defined(CPPCHECK)
#define MPT_DETECTED_LTDL 1
#else
#if __has_include(<ltdl.h>)
#define MPT_DETECTED_LTDL 1
#else
#define MPT_DETECTED_LTDL 0
#endif
#endif
#endif

#endif // MPT_DETECT_LTDL_HPP
