/*
 * StdAfx.h
 * --------
 * Purpose: Include file for standard system include files, or project specific include files that are used frequently, but are changed infrequently. Also includes the global build settings from openmpt/all/BuildSettings.hpp.
 * Notes  : (currently none)
 * Authors: Olivier Lapicque
 *          OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#pragma once


// has to be first
#include "openmpt/all/BuildSettings.hpp"
#include "openmpt/all/PlatformFixes.hpp"


#if defined(MODPLUG_TRACKER)

#if defined(MPT_WITH_MFC)

// cppcheck-suppress missingInclude
#include <afx.h>            // MFC core
// cppcheck-suppress missingInclude
#include <afxwin.h>         // MFC standard components
// cppcheck-suppress missingInclude
#include <afxext.h>         // MFC extensions
// cppcheck-suppress missingInclude
#include <afxcmn.h>         // MFC support for Windows Common Controls
// cppcheck-suppress missingInclude
#include <afxcview.h>
// cppcheck-suppress missingInclude
#include <afxdlgs.h>
#ifndef _AFX_NO_MFC_CONTROLS_IN_DIALOGS
// cppcheck-suppress missingInclude
#include <afxlistctrl.h>
#endif // !_AFX_NO_MFC_CONTROLS_IN_DIALOGS
// cppcheck-suppress missingInclude
#include <afxole.h>

#endif // MPT_WITH_MFC

#if MPT_OS_WINDOWS

#include <windows.h>
#include <windowsx.h>
#include <shlwapi.h>
#include <mmsystem.h>

#endif // MPT_OS_WINDOWS

#endif // MODPLUG_TRACKER


#if MPT_COMPILER_MSVC
#include <intrin.h>
#endif


#include "mpt/check/compiler.hpp"
#include "mpt/check/libc.hpp"
#include "mpt/check/libcxx.hpp"
#if defined(MPT_WITH_MFC)
#include "mpt/check/mfc.hpp"
#endif
#if MPT_OS_WINDOWS
#include "mpt/check/windows.hpp"
#endif

#include "mpt/base/span.hpp"
#include "mpt/exception/exception.hpp"
#include "mpt/exception/exception_text.hpp"
#include "mpt/out_of_memory/out_of_memory.hpp"
#include "mpt/string/types.hpp"
#include "mpt/system_error/system_error.hpp"

#include "openmpt/base/Types.hpp"
#include "openmpt/logging/Logger.hpp"


#include <memory>
#include <new>


// this will be available everywhere

#include "../common/mptBaseMacros.h"
// <version>
// <array>
// <iterator>
// <type_traits>
// <cstddef>
// <cstdint>

#include "../common/mptBaseTypes.h"
// "openmpt/base/Types.hpp"
// "mptBaseMacros.h"
// <array>
// <limits>
// <type_traits>
// <cstdint>

#include "../common/mptAssert.h"
// "mptBaseMacros.h"

#include "../common/mptBaseUtils.h"
// <algorithm>
// <bit>
// <limits>
// <numeric>
// <utility>

#include "../common/mptString.h"
// <algorithm>
// <limits>
// <string>
// <string_view>
// <type_traits>
// <cstring>

#include "../common/mptStringBuffer.h"

#include "../common/mptStringFormat.h"
// <stdexcept>

#include "../common/mptPathString.h"

#include "../common/Logging.h"
// "openmpt/logging/Logger.hpp"
// <atomic>

#include "../common/misc_util.h"
// <stdexcept>
// <optional>
// <vector>

// for std::abs
#include <cstdlib>
#include <stdlib.h>
#include <cmath>
#include <math.h>


//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.
