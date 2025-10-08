/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_FORMAT_DETAULT_FLOATINGPOINT_HPP
#define MPT_FORMAT_DETAULT_FLOATINGPOINT_HPP



#include "mpt/base/detect.hpp"

#if !defined(MPT_LIBCXX_QUIRK_NO_TO_CHARS_FLOAT)
#define MPT_FORMAT_FORMAT_DEFAULT_FLOAT_CXX17 1
#else
#define MPT_FORMAT_FORMAT_DEFAULT_FLOAT_CXX17 0
#endif

#if MPT_FORMAT_FORMAT_DEFAULT_FLOAT_CXX17
#include "mpt/base/algorithm.hpp"
#endif
#include "mpt/base/namespace.hpp"
#include "mpt/format/helpers.hpp"
#include "mpt/string_transcode/transcode.hpp"

#if MPT_FORMAT_FORMAT_DEFAULT_FLOAT_CXX17
#include <charconv>
#endif
#if !MPT_FORMAT_FORMAT_DEFAULT_FLOAT_CXX17
#include <iomanip>
#include <ios>
#include <limits>
#include <locale>
#include <sstream>
#endif
#include <string>
#if MPT_FORMAT_FORMAT_DEFAULT_FLOAT_CXX17
#include <system_error>
#endif
#include <type_traits>



namespace mpt {
inline namespace MPT_INLINE_NS {


#if MPT_FORMAT_FORMAT_DEFAULT_FLOAT_CXX17
template <typename Tstring, typename T, std::enable_if_t<std::is_floating_point<T>::value, bool> = true>
inline Tstring to_chars_string(const T & x) {
	std::string str(1, '\0');
	bool done = false;
	while (!done) {
		std::to_chars_result result = std::to_chars(str.data(), str.data() + str.size(), x);
		if (result.ec != std::errc{}) {
			str.resize(mpt::exponential_grow(str.size()), '\0');
		} else {
			str.resize(result.ptr - str.data());
			done = true;
		}
	}
	return mpt::convert_formatted_simple<Tstring>(str);
}

template <typename Tstring, typename T, std::enable_if_t<std::is_floating_point<T>::value, bool> = true>
inline Tstring format_value_default(const T & x) {
	return mpt::transcode<Tstring>(mpt::to_chars_string<typename mpt::select_format_string_type<Tstring>::type>(x));
}
#endif


#if !MPT_FORMAT_FORMAT_DEFAULT_FLOAT_CXX17
template <typename Tstring, typename T, std::enable_if_t<std::is_floating_point<T>::value, bool> = true>
inline Tstring to_stream_string(const T & x) {
	using stream_char_type = typename mpt::select_format_char_type<typename Tstring::value_type>::type;
	std::basic_ostringstream<stream_char_type> s;
	s.imbue(std::locale::classic());
	s << std::setprecision(std::numeric_limits<T>::max_digits10) << x;
	return mpt::convert_formatted_simple<Tstring>(s.str());
}

template <typename Tstring, typename T, std::enable_if_t<std::is_floating_point<T>::value, bool> = true>
inline Tstring format_value_default(const T & x) {
	return mpt::transcode<Tstring>(mpt::to_stream_string<typename mpt::select_format_string_type<Tstring>::type>(x));
}
#endif


} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_FORMAT_DETAULT_FLOATINGPOINT_HPP
