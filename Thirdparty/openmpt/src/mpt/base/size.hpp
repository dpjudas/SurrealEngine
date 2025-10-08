/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_BASE_SIZE_HPP
#define MPT_BASE_SIZE_HPP



#include "mpt/base/detect.hpp"

#if MPT_CXX_AT_LEAST(20)
#include "mpt/base/constexpr_throw.hpp"
#endif // C++20
#include "mpt/base/namespace.hpp"
#if MPT_CXX_AT_LEAST(20)
#include "mpt/base/utility.hpp"
#endif // C++20

#if MPT_CXX_AT_LEAST(20)
#include <stdexcept>
#endif // C++20

#include <cstddef>



namespace mpt {
inline namespace MPT_INLINE_NS {



using usize = std::size_t;
using ssize = std::ptrdiff_t;


namespace size_literals {
#if MPT_CXX_AT_LEAST(20)
consteval usize operator""_uz(unsigned long long val) noexcept {
	if (!mpt::in_range<usize>(val)) {
		mpt::constexpr_throw(std::domain_error(""));
	}
	return static_cast<usize>(val);
}
#else  // !C++20
constexpr usize operator""_uz(unsigned long long val) noexcept {
	return static_cast<usize>(val);
}
#endif // C++20
} // namespace size_literals

namespace size_literals {
#if MPT_CXX_AT_LEAST(20)
consteval ssize operator""_z(unsigned long long val) noexcept {
	if (!mpt::in_range<ssize>(val)) {
		mpt::constexpr_throw(std::domain_error(""));
	}
	return static_cast<ssize>(val);
}
#else  // !C++20
constexpr ssize operator""_z(unsigned long long val) noexcept {
	return static_cast<ssize>(val);
}
#endif // C++20
} // namespace size_literals



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_BASE_SIZE_HPP
