/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_PARSE_SPLIT_HPP
#define MPT_PARSE_SPLIT_HPP



#include "mpt/base/namespace.hpp"
#include "mpt/parse/parse.hpp"
#include "mpt/string/utility.hpp"

#include <vector>

#include <cstddef>



namespace mpt {
inline namespace MPT_INLINE_NS {



template <typename T, typename Tstring>
std::vector<T> split_parse(const Tstring & str, const Tstring & sep = Tstring(1, char_constants<typename Tstring::value_type>::comma)) {
	std::vector<T> vals;
	std::size_t pos = 0;
	while (str.find(sep, pos) != std::string::npos) {
		vals.push_back(mpt::parse<T>(str.substr(pos, str.find(sep, pos) - pos)));
		pos = str.find(sep, pos) + sep.length();
	}
	if (!vals.empty() || (str.substr(pos).length() > 0)) {
		vals.push_back(mpt::parse<T>(str.substr(pos)));
	}
	return vals;
}



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_PARSE_SPLIT_HPP
