/* SPDX-License-Identifier: BSD-3-Clause */
/* SPDX-FileCopyrightText: OpenMPT Project Developers and Contributors */


#pragma once



#if defined(MODPLUG_TRACKER) || defined(LIBOPENMPT_BUILD)
#include "BuildSettingsCompiler.h"
#endif



#include "mpt/base/detect_compiler.hpp"
#include "mpt/base/detect_os.hpp"
#include "mpt/base/detect_quirks.hpp"



#if defined(MODPLUG_TRACKER) || defined(LIBOPENMPT_BUILD)
#include "BuildSettings.h"
#else



#include "mpt/base/namespace.hpp"



#ifndef OPENMPT_NAMESPACE
#define OPENMPT_NAMESPACE OpenMPT
#endif

#ifndef OPENMPT_NAMESPACE_BEGIN
#define OPENMPT_NAMESPACE_BEGIN \
	namespace OPENMPT_NAMESPACE \
	{ \
	inline namespace MPT_INLINE_NS \
	{
#endif
#ifndef OPENMPT_NAMESPACE_END
#define OPENMPT_NAMESPACE_END \
	} \
	}
#endif



#ifdef __cplusplus
OPENMPT_NAMESPACE_BEGIN
namespace mpt
{
#ifndef MPT_NO_NAMESPACE
using namespace ::mpt;
#endif
}  // namespace mpt
OPENMPT_NAMESPACE_END
#endif



#endif
