/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_BASE_SATURATE_CAST_HPP
#define MPT_BASE_SATURATE_CAST_HPP



#include "mpt/base/detect.hpp"
#include "mpt/base/namespace.hpp"

#include <algorithm>
#include <limits>
#if MPT_CXX_AT_LEAST(26)
#include <numeric>
#endif



namespace mpt {
inline namespace MPT_INLINE_NS {


#if MPT_CXX_AT_LEAST(26)

using std::saturate_cast;

#else

// Saturate the value of src to the domain of Tdst
template <typename Tdst, typename Tsrc>
constexpr Tdst saturate_cast(Tsrc src) noexcept {
	// This code tries not only to obviously avoid overflows but also to avoid signed/unsigned comparison warnings and type truncation warnings (which in fact would be safe here) by explicit casting.
	static_assert(std::numeric_limits<Tdst>::is_integer);
	static_assert(std::numeric_limits<Tsrc>::is_integer);
	if constexpr (std::numeric_limits<Tdst>::is_signed && std::numeric_limits<Tsrc>::is_signed) {
		if constexpr (sizeof(Tdst) >= sizeof(Tsrc)) {
			return static_cast<Tdst>(src);
		} else {
			return static_cast<Tdst>(std::max(static_cast<Tsrc>(std::numeric_limits<Tdst>::min()), std::min(src, static_cast<Tsrc>(std::numeric_limits<Tdst>::max()))));
		}
	} else if constexpr (!std::numeric_limits<Tdst>::is_signed && !std::numeric_limits<Tsrc>::is_signed) {
		if constexpr (sizeof(Tdst) >= sizeof(Tsrc)) {
			return static_cast<Tdst>(src);
		} else {
			return static_cast<Tdst>(std::min(src, static_cast<Tsrc>(std::numeric_limits<Tdst>::max())));
		}
	} else if constexpr (std::numeric_limits<Tdst>::is_signed && !std::numeric_limits<Tsrc>::is_signed) {
		if constexpr (sizeof(Tdst) > sizeof(Tsrc)) {
			return static_cast<Tdst>(src);
		} else if constexpr (sizeof(Tdst) == sizeof(Tsrc)) {
			return static_cast<Tdst>(std::min(src, static_cast<Tsrc>(std::numeric_limits<Tdst>::max())));
		} else {
			return static_cast<Tdst>(std::min(src, static_cast<Tsrc>(std::numeric_limits<Tdst>::max())));
		}
	} else { // Tdst unsigned, Tsrc signed
		if constexpr (sizeof(Tdst) >= sizeof(Tsrc)) {
			return static_cast<Tdst>(std::max(static_cast<Tsrc>(0), src));
		} else {
			return static_cast<Tdst>(std::max(static_cast<Tsrc>(0), std::min(src, static_cast<Tsrc>(std::numeric_limits<Tdst>::max()))));
		}
	}
}

#endif


} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_BASE_SATURATE_CAST_HPP
