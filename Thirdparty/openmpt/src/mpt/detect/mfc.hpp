/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_DETECT_MFC_HPP
#define MPT_DETECT_MFC_HPP

#include "mpt/base/compiletime_warning.hpp"

#if defined(MPT_WITH_MFC)
#if !defined(CPPCHECK)
#if !__has_include(<afx.h>)
#error "MPT_WITH_MFC defined but <afx.h> not found."
#endif
#endif
#if !MPT_COMPILER_GENERIC && !MPT_COMPILER_MSVC && !MPT_COMPILER_CLANG
MPT_WARNING("Using MFC with unsupported compiler.")
#endif
#define MPT_DETECTED_MFC 1
#else
#define MPT_DETECTED_MFC 0
#endif

#endif // MPT_DETECT_MFC_HPP
