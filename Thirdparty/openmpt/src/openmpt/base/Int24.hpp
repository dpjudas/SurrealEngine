/* SPDX-License-Identifier: BSD-3-Clause */
/* SPDX-FileCopyrightText: OpenMPT Project Developers and Contributors */


#pragma once

#include "openmpt/all/BuildSettings.hpp"

#include "mpt/endian/int24.hpp"
#include "openmpt/base/Types.hpp"

#include <limits>



OPENMPT_NAMESPACE_BEGIN



using uint24 = mpt::uint24;
static_assert(sizeof(uint24) == 3);
inline constexpr uint32 uint24_min = std::numeric_limits<uint24>::min();
inline constexpr uint32 uint24_max = std::numeric_limits<uint24>::max();
using int24 = mpt::int24;
static_assert(sizeof(int24) == 3);
inline constexpr int32 int24_min = std::numeric_limits<int24>::min();
inline constexpr int32 int24_max = std::numeric_limits<int24>::max();

using int24le = mpt::packed<int24, mpt::endian::little>;
using uint24le = mpt::packed<uint24, mpt::endian::little>;
using int24be = mpt::packed<int24, mpt::endian::big>;
using uint24be = mpt::packed<uint24, mpt::endian::big>;



OPENMPT_NAMESPACE_END
