/*
 * mptBaseTypes.h
 * --------------
 * Purpose: Basic data type definitions.
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#pragma once

#include "openmpt/all/BuildSettings.hpp"

#include "mpt/base/check_platform.hpp"
#include "mpt/base/float.hpp"
#include "mpt/base/integer.hpp"
#include "mpt/base/pointer.hpp"
#include "mpt/base/size.hpp"
#include "mpt/base/source_location.hpp"
#include "openmpt/base/Types.hpp"

#include "mptBaseMacros.h"

#include <array>
#include <limits>
#include <type_traits>

#include <cstddef>
#include <cstdint>

#include <stdint.h>



OPENMPT_NAMESPACE_BEGIN


constexpr inline int8 int8_min     = std::numeric_limits<int8>::min();
constexpr inline int16 int16_min   = std::numeric_limits<int16>::min();
constexpr inline int32 int32_min   = std::numeric_limits<int32>::min();
constexpr inline int64 int64_min   = std::numeric_limits<int64>::min();

constexpr inline int8 int8_max     = std::numeric_limits<int8>::max();
constexpr inline int16 int16_max   = std::numeric_limits<int16>::max();
constexpr inline int32 int32_max   = std::numeric_limits<int32>::max();
constexpr inline int64 int64_max   = std::numeric_limits<int64>::max();

constexpr inline uint8 uint8_max   = std::numeric_limits<uint8>::max();
constexpr inline uint16 uint16_max = std::numeric_limits<uint16>::max();
constexpr inline uint32 uint32_max = std::numeric_limits<uint32>::max();
constexpr inline uint64 uint64_max = std::numeric_limits<uint64>::max();


OPENMPT_NAMESPACE_END
