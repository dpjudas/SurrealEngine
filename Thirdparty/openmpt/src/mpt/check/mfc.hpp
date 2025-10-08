/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_CHECK_MFC_HPP
#define MPT_CHECK_MFC_HPP

#include "mpt/base/compiletime_warning.hpp"
#include "mpt/detect/mfc.hpp"

#if MPT_DETECTED_MFC

#ifndef _CSTRING_DISABLE_NARROW_WIDE_CONVERSION
#ifndef MPT_CHECK_MFC_IGNORE_WARNING_NO_CSTRING_DISABLE_NARROW_WIDE_CONVERSION
MPT_WARNING("MFC uses CString with automatic encoding conversions. Please #define _CSTRING_DISABLE_NARROW_WIDE_CONVERSION.")
#endif
#endif

#endif // MPT_DETECTED_MFC

#endif // MPT_CHECK_MFC_HPP
