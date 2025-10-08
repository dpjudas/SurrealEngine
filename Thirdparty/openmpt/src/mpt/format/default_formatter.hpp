/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_FORMAT_DEFAULT_FORMATTER_HPP
#define MPT_FORMAT_DEFAULT_FORMATTER_HPP



#include "mpt/base/namespace.hpp"
#include "mpt/format/default_floatingpoint.hpp"
#include "mpt/format/default_integer.hpp"
#include "mpt/format/default_string.hpp"



namespace mpt {
inline namespace MPT_INLINE_NS {



struct default_formatter {
	template <typename Tstring, typename T>
	static inline Tstring format(const T & value) {
		using namespace mpt;
		return format_value_default<Tstring>(value);
	}
};



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_FORMAT_DEFAULT_FORMATTER_HPP
