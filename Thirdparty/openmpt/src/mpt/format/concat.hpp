/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_FORMAT_CONCAT_HPP
#define MPT_FORMAT_CONCAT_HPP



#include "mpt/base/namespace.hpp"
#include "mpt/format/default_formatter.hpp"

#include <utility>



namespace mpt {
inline namespace MPT_INLINE_NS {



template <typename Tstring, typename... Ts>
inline Tstring concat(Ts &&... xs) {
	return (default_formatter::template format<Tstring>(std::forward<Ts>(xs)) + ...);
}



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_FORMAT_CONCAT_HPP
