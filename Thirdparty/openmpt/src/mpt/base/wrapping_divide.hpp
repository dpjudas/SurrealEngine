/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_BASE_WRAPPING_DIVIDE_HPP
#define MPT_BASE_WRAPPING_DIVIDE_HPP



#include "mpt/base/namespace.hpp"



namespace mpt {
inline namespace MPT_INLINE_NS {


// Modulo with more intuitive behaviour for some contexts:
// Instead of being symmetrical around 0, the pattern for positive numbers is repeated in the negative range.
// For example, wrapping_modulo(-1, m) == (m - 1).
// Behaviour is undefined if m<=0.
template <typename T, typename M>
constexpr auto wrapping_modulo(T x, M m) -> decltype(x % m) {
	return (x >= 0) ? (x % m) : (m - 1 - ((-1 - x) % m));
}

template <typename T, typename D>
constexpr auto wrapping_divide(T x, D d) -> decltype(x / d) {
	return (x >= 0) ? (x / d) : (((x + 1) / d) - 1);
}


} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_BASE_WRAPPING_DIVIDE_HPP
