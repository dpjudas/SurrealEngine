/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_BASE_NUMERIC_HPP
#define MPT_BASE_NUMERIC_HPP



#include "mpt/base/detect_compiler.hpp"
#include "mpt/base/namespace.hpp"

#include "mpt/base/bit.hpp"
#include "mpt/base/saturate_cast.hpp"

#include <algorithm>
#include <limits>
#include <type_traits>



namespace mpt {
inline namespace MPT_INLINE_NS {


template <typename Tmod, Tmod m>
struct ModIfNotZeroImpl {
	template <typename Tval>
	constexpr Tval mod(Tval x) {
		static_assert(std::numeric_limits<Tmod>::is_integer);
		static_assert(!std::numeric_limits<Tmod>::is_signed);
		static_assert(std::numeric_limits<Tval>::is_integer);
		static_assert(!std::numeric_limits<Tval>::is_signed);
		return static_cast<Tval>(x % m);
	}
};
template <>
struct ModIfNotZeroImpl<uint8, 0> {
	template <typename Tval>
	constexpr Tval mod(Tval x) {
		return x;
	}
};
template <>
struct ModIfNotZeroImpl<uint16, 0> {
	template <typename Tval>
	constexpr Tval mod(Tval x) {
		return x;
	}
};
template <>
struct ModIfNotZeroImpl<uint32, 0> {
	template <typename Tval>
	constexpr Tval mod(Tval x) {
		return x;
	}
};
template <>
struct ModIfNotZeroImpl<uint64, 0> {
	template <typename Tval>
	constexpr Tval mod(Tval x) {
		return x;
	}
};

// Returns x % m if m != 0, x otherwise.
// i.e. "return (m == 0) ? x : (x % m);", but without causing a warning with stupid older compilers
template <typename Tmod, Tmod m, typename Tval>
constexpr Tval modulo_if_not_zero(Tval x) {
	return ModIfNotZeroImpl<Tmod, m>().mod(x);
}

// rounds x up to multiples of target
template <typename T>
constexpr T align_up(T x, T target) {
	return ((x + (target - 1)) / target) * target;
}

// rounds x down to multiples of target
template <typename T>
constexpr T align_down(T x, T target) {
	return (x / target) * target;
}

// rounds x up to multiples of target or saturation of T
template <typename T>
constexpr T saturate_align_up(T x, T target) {
	if (x > (std::numeric_limits<T>::max() - (target - 1))) {
		return std::numeric_limits<T>::max();
	}
	return ((x + (target - 1)) / target) * target;
}

// Returns sign of a number (-1 for negative numbers, 1 for positive numbers, 0 for 0)
template <class T>
constexpr int signum(T value) {
	return (value > T(0)) - (value < T(0));
}


} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_BASE_ALGORITHM_HPP
