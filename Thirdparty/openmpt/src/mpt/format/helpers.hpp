/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_FORMAT_HELPERS_HPP
#define MPT_FORMAT_HELPERS_HPP



#include "mpt/base/detect.hpp"
#include "mpt/base/namespace.hpp"
#include "mpt/string/types.hpp"
#include "mpt/string_transcode/transcode.hpp"

#include <string>
#include <type_traits>



namespace mpt {
inline namespace MPT_INLINE_NS {


template <typename Tdststring, typename Tsrcstring>
inline Tdststring convert_formatted_simple(const Tsrcstring & src) {
	if constexpr (std::is_same<Tdststring, Tsrcstring>::value) {
		return src;
	} else {
		Tdststring dst;
		dst.reserve(src.length());
		for (std::size_t i = 0; i < src.length(); ++i) {
			dst.push_back(mpt::unsafe_char_convert<typename Tdststring::value_type>(src[i]));
		}
		return dst;
	}
}


template <typename Tchar>
struct select_format_char_type {
	using type = char;
};

#if !defined(MPT_COMPILER_QUIRK_NO_WCHAR)
template <>
struct select_format_char_type<wchar_t> {
	using type = wchar_t;
};
#if MPT_USTRING_MODE_WIDE
#if MPT_CXX_AT_LEAST(20)
template <>
struct select_format_char_type<char8_t> {
	using type = wchar_t;
};
#endif // C++20
template <>
struct select_format_char_type<char16_t> {
	using type = wchar_t;
};
template <>
struct select_format_char_type<char32_t> {
	using type = wchar_t;
};
#endif // MPT_USTRING_MODE_WIDE
#endif // !MPT_COMPILER_QUIRK_NO_WCHAR


template <typename Tstring>
struct select_format_string_type {
	using type = mpt::ustring;
};

template <>
struct select_format_string_type<std::string> {
	using type = std::string;
};

#if !defined(MPT_COMPILER_QUIRK_NO_WCHAR)
template <>
struct select_format_string_type<std::wstring> {
	using type = std::wstring;
};
#endif // !MPT_COMPILER_QUIRK_NO_WCHAR

#if MPT_CXX_AT_LEAST(20)
template <>
struct select_format_string_type<std::u8string> {
	using type = std::u8string;
};
#endif // C++20

template <>
struct select_format_string_type<std::u16string> {
	using type = std::u16string;
};

template <>
struct select_format_string_type<std::u32string> {
	using type = std::u32string;
};


} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_FORMAT_HELPERS_HPP
