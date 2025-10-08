/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_FORMAT_JOIN_HPP
#define MPT_FORMAT_JOIN_HPP



#include "mpt/base/namespace.hpp"
#include "mpt/format/simple.hpp"
#include "mpt/string/utility.hpp"

#include <vector>

#include <cstddef>



namespace mpt {
inline namespace MPT_INLINE_NS {



template <typename Tstring, typename T>
Tstring join_format(const std::vector<T> & vals, const Tstring & sep = Tstring(1, char_constants<typename Tstring::value_type>::comma)) {
	Tstring str;
	for (std::size_t i = 0; i < vals.size(); ++i) {
		if (i > 0) {
			str += sep;
		}
		str += mpt::format<Tstring>::val(vals[i]);
	}
	return str;
}



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_FORMAT_JOIN_HPP
