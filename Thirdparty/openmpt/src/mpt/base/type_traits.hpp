/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_BASE_TYPE_TRAITS_HPP
#define MPT_BASE_TYPE_TRAITS_HPP



#include "mpt/base/detect.hpp"
#include "mpt/base/namespace.hpp"

#include <type_traits>



namespace mpt {
inline namespace MPT_INLINE_NS {



#if MPT_CXX_AT_LEAST(20)

using std::type_identity;

#else // ! C++20

template <typename T>
struct type_identity {
	using type = T;
};

#endif // C++20



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_BASE_TYPE_TRAITS_HPP
