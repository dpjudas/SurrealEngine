/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_ENDIAN_TYPE_TRAITS_HPP
#define MPT_ENDIAN_TYPE_TRAITS_HPP



#include "mpt/base/bit.hpp"
#include "mpt/base/namespace.hpp"



namespace mpt {
inline namespace MPT_INLINE_NS {



template <mpt::endian endian, typename T>
struct make_endian {
};

template <typename T>
struct make_le {
	using type = typename mpt::make_endian<mpt::endian::little, T>::type;
};

template <typename T>
struct make_be {
	using type = typename mpt::make_endian<mpt::endian::big, T>::type;
};



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_ENDIAN_TYPE_TRAITS_HPP
