/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_BASE_SATURATE_ROUND_HPP
#define MPT_BASE_SATURATE_ROUND_HPP



#include "mpt/base/namespace.hpp"

#include "mpt/base/math.hpp"

#include <limits>
#include <type_traits>

#include <cmath>



namespace mpt {
inline namespace MPT_INLINE_NS {



template <typename Tdst, typename Tsrc>
constexpr Tdst saturate_trunc(Tsrc src) {
	static_assert(std::is_floating_point<Tsrc>::value);
	if (src >= static_cast<Tsrc>(std::numeric_limits<Tdst>::max())) {
		return std::numeric_limits<Tdst>::max();
	}
	if (src <= static_cast<Tsrc>(std::numeric_limits<Tdst>::min())) {
		return std::numeric_limits<Tdst>::min();
	}
	return static_cast<Tdst>(src);
}


// Rounds given double value to nearest integer value of type T.
// Out-of-range values are saturated to the specified integer type's limits.

template <typename Tdst, typename Tsrc>
inline Tdst saturate_round(Tsrc val) {
	static_assert(std::is_floating_point<Tsrc>::value);
	static_assert(std::numeric_limits<Tdst>::is_integer);
	return mpt::saturate_trunc<Tdst>(mpt::round(val));
}


template <typename Tdst, typename Tsrc>
inline Tdst saturate_ceil(Tsrc val) {
	static_assert(std::is_floating_point<Tsrc>::value);
	static_assert(std::numeric_limits<Tdst>::is_integer);
	return mpt::saturate_trunc<Tdst>(std::ceil(val));
}


template <typename Tdst, typename Tsrc>
inline Tdst saturate_floor(Tsrc val) {
	static_assert(std::is_floating_point<Tsrc>::value);
	static_assert(std::numeric_limits<Tdst>::is_integer);
	return mpt::saturate_trunc<Tdst>(std::floor(val));
}


} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_BASE_SATURATE_ROUND_HPP
