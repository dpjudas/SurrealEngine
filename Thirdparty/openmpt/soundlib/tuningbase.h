/*
 * tuningbase.h
 * ------------
 * Purpose: Alternative sample tuning.
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#pragma once

#include "openmpt/all/BuildSettings.hpp"


#include <limits>


OPENMPT_NAMESPACE_BEGIN


namespace Tuning {


enum class SerializationResult : int {
	Success = 1,
	NoMagic = 0,
	Failure = -1
};


using NOTEINDEXTYPE = int16;    // Some signed integer-type.
using UNOTEINDEXTYPE = uint16;  // Unsigned NOTEINDEXTYPE.

using RATIOTYPE = somefloat32;      // Some 'real figure' type able to present ratios. If changing RATIOTYPE, serialization methods may need modifications.

// Counter of steps between notes. If there is no 'finetune'(finestepcount == 0),
// then 'step difference' between notes is the
// same as differences in NOTEINDEXTYPE. In a way similar to ticks and rows in pattern -
// ticks <-> STEPINDEX, rows <-> NOTEINDEX
using STEPINDEXTYPE = int32;
using USTEPINDEXTYPE = uint32;

struct NoteRange
{
  NOTEINDEXTYPE first;
  NOTEINDEXTYPE last;
};


// Derived from old IsStepCountRangeSufficient(), this is actually a more
// sensible value than what was calculated in earlier versions.
inline constexpr STEPINDEXTYPE FINESTEPCOUNT_MAX = 0xffff;

inline constexpr auto NOTEINDEXTYPE_MIN = std::numeric_limits<NOTEINDEXTYPE>::min();
inline constexpr auto NOTEINDEXTYPE_MAX = std::numeric_limits<NOTEINDEXTYPE>::max();
inline constexpr auto UNOTEINDEXTYPE_MAX = std::numeric_limits<UNOTEINDEXTYPE>::max();
inline constexpr auto STEPINDEXTYPE_MIN = std::numeric_limits<STEPINDEXTYPE>::min();
inline constexpr auto STEPINDEXTYPE_MAX = std::numeric_limits<STEPINDEXTYPE>::max();
inline constexpr auto USTEPINDEXTYPE_MAX = std::numeric_limits<USTEPINDEXTYPE>::max();


enum class Type : uint16
{
	GENERAL        = 0,
	GROUPGEOMETRIC = 1,
	GEOMETRIC      = 3,
};


class CTuning;


} // namespace Tuning


using CTuning = Tuning::CTuning;


OPENMPT_NAMESPACE_END
