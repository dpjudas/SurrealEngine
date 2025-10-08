/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_BASE_CONSTEXPR_THROW_HPP
#define MPT_BASE_CONSTEXPR_THROW_HPP



#include "mpt/base/detect.hpp"
#include "mpt/base/namespace.hpp"

#include <utility>



namespace mpt {
inline namespace MPT_INLINE_NS {


// Work-around for the requirement of at least 1 non-throwing function argument combination in C++ (17,2a).

template <typename Exception>
constexpr bool constexpr_throw_helper(Exception && e, bool really = true) {
	//return !really ? really : throw std::forward<Exception>(e);
	if (really) {
		throw std::forward<Exception>(e);
	}
	// cppcheck-suppress identicalConditionAfterEarlyExit
	return really;
}

template <typename Exception>
constexpr bool constexpr_throw(Exception && e) {
	return mpt::constexpr_throw_helper(std::forward<Exception>(e));
}

template <typename T, typename Exception>
constexpr T constexpr_throw_helper(Exception && e, bool really = true) {
	//return !really ? really : throw std::forward<Exception>(e);
	if (really) {
		throw std::forward<Exception>(e);
	}
	return T{};
}

template <typename T, typename Exception>
constexpr T constexpr_throw(Exception && e) {
	return mpt::constexpr_throw_helper<T>(std::forward<Exception>(e));
}


} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_BASE_CONSTEXPR_THROW_HPP
