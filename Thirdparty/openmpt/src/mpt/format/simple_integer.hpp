/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_FORMAT_SIMPLE_INTEGER_HPP
#define MPT_FORMAT_SIMPLE_INTEGER_HPP


#include "mpt/base/detect.hpp"

#if !defined(MPT_LIBCXX_QUIRK_NO_TO_CHARS_INT)
#define MPT_FORMAT_FORMAT_SIMPLE_INT_CXX17 1
#else // MPT_LIBCXX_QUIRK_NO_TO_CHARS_INT
#define MPT_FORMAT_FORMAT_SIMPLE_INT_CXX17 0
#endif // !MPT_LIBCXX_QUIRK_NO_TO_CHARS_INT

#include "mpt/base/algorithm.hpp"
#include "mpt/base/namespace.hpp"
#include "mpt/format/helpers.hpp"
#include "mpt/format/simple_spec.hpp"
#include "mpt/string/types.hpp"

#if !MPT_FORMAT_FORMAT_SIMPLE_INT_CXX17
#include <algorithm>
#endif // !MPT_FORMAT_FORMAT_SIMPLE_INT_CXX17
#if MPT_FORMAT_FORMAT_SIMPLE_INT_CXX17
#include <charconv>
#endif // MPT_FORMAT_FORMAT_SIMPLE_INT_CXX17
#if !MPT_FORMAT_FORMAT_SIMPLE_INT_CXX17
#include <ios>
#include <locale>
#endif // !MPT_FORMAT_FORMAT_SIMPLE_INT_CXX17
#if MPT_FORMAT_FORMAT_SIMPLE_INT_CXX17
#include <string>
#endif // MPT_FORMAT_FORMAT_SIMPLE_INT_CXX17
#if !MPT_FORMAT_FORMAT_SIMPLE_INT_CXX17
#include <sstream>
#endif // !MPT_FORMAT_FORMAT_SIMPLE_INT_CXX17
#if MPT_FORMAT_FORMAT_SIMPLE_INT_CXX17
#include <system_error>
#endif // MPT_FORMAT_FORMAT_SIMPLE_INT_CXX17
#include <type_traits>

#include <cstddef>



namespace mpt {
inline namespace MPT_INLINE_NS {


#if MPT_FORMAT_FORMAT_SIMPLE_INT_CXX17

template <typename Tstring, typename T, std::enable_if_t<std::is_integral<T>::value, bool> = true>
inline Tstring format_simple_integer_to_chars(const T & x, int base) {
	std::string str(1, '\0');
	bool done = false;
	while (!done) {
		if constexpr (std::is_same<T, bool>::value) {
			std::to_chars_result result = std::to_chars(str.data(), str.data() + str.size(), static_cast<int>(x), base);
			if (result.ec != std::errc{}) {
				str.resize(mpt::exponential_grow(str.size()), '\0');
			} else {
				str.resize(result.ptr - str.data());
				done = true;
			}
		} else {
			std::to_chars_result result = std::to_chars(str.data(), str.data() + str.size(), x, base);
			if (result.ec != std::errc{}) {
				str.resize(mpt::exponential_grow(str.size()), '\0');
			} else {
				str.resize(result.ptr - str.data());
				done = true;
			}
		}
	}
	return mpt::convert_formatted_simple<Tstring>(str);
}

#else // !MPT_FORMAT_FORMAT_SIMPLE_INT_CXX17

template <typename Tstring, typename T, std::enable_if_t<std::is_integral<T>::value, bool> = true>
inline Tstring format_simple_integer_to_stream(const T & x, int base) {
	using stream_char_type = typename mpt::select_format_char_type<typename Tstring::value_type>::type;
	if ((base == 10) || ((base == 16) && std::is_unsigned<T>::value) || ((base == 8) && std::is_unsigned<T>::value)) {
		// fast path
		std::basic_ostringstream<stream_char_type> s;
		s.imbue(std::locale::classic());
		if (base == 16) {
			s << std::hex;
		} else if (base == 8) {
			s << std::oct;
		}
		if constexpr (std::is_same<T, bool>::value) {
			s << static_cast<int>(x);
		} else if constexpr (mpt::is_character<T>::value) {
			s << (x + 0); // force integral promotion
		} else {
			s << x;
		}
		return mpt::convert_formatted_simple<Tstring>(s.str());
	} else {
		if constexpr (std::is_same<T, bool>::value) {
			return x ? Tstring(1, mpt::char_constants<typename Tstring::value_type>::number1) : Tstring(1, mpt::char_constants<typename Tstring::value_type>::number0);
		} else if constexpr (std::is_unsigned<T>::value) {
			Tstring result;
			T val = x;
			if (val == 0) {
				result += Tstring(1, mpt::char_constants<typename Tstring::value_type>::number0);
			} else {
				using Tunsigned = typename std::make_unsigned<T>::type;
				while (val > 0) {
					Tunsigned digit = static_cast<Tunsigned>(val % static_cast<unsigned int>(base));
					val = static_cast<Tunsigned>(val / static_cast<unsigned int>(base));
					if (digit >= 10) {
						result += Tstring(1, static_cast<typename Tstring::value_type>(mpt::char_constants<typename Tstring::value_type>::a - 10 + digit));
					} else {
						result += Tstring(1, static_cast<typename Tstring::value_type>(mpt::char_constants<typename Tstring::value_type>::number0 + digit));
					}
				}
				std::reverse(result.begin(), result.end());
			}
			return result;
		} else {
			Tstring result;
			if (x == 0) {
				result += Tstring(1, mpt::char_constants<typename Tstring::value_type>::number0);
			} else {
				using Tunsigned = typename std::make_unsigned<T>::type;
				Tunsigned val = (x != -x) ? ((x >= 0) ? x : -x) : (static_cast<Tunsigned>(-(x + 1)) + 1);
				while (val > 0) {
					Tunsigned digit = static_cast<Tunsigned>(val % static_cast<unsigned int>(base));
					val = static_cast<Tunsigned>(val / static_cast<unsigned int>(base));
					if (digit >= 10) {
						result += Tstring(1, static_cast<typename Tstring::value_type>(mpt::char_constants<typename Tstring::value_type>::a - 10 + digit));
					} else {
						result += Tstring(1, static_cast<typename Tstring::value_type>(mpt::char_constants<typename Tstring::value_type>::number0 + digit));
					}
				}
				if (x < 0) {
					result += Tstring(1, mpt::char_constants<typename Tstring::value_type>::minus);
				}
				std::reverse(result.begin(), result.end());
			}
			return result;
		}
	}
}

#endif // MPT_FORMAT_FORMAT_SIMPLE_INT_CXX17


template <typename Tstring>
inline Tstring format_simple_integer_postprocess_case(Tstring str, const format_simple_spec<Tstring> & format) {
	format_simple_flags f = format.GetFlags();
	if (f & format_simple_base::CaseUpp) {
		for (auto & c : str) {
			if (mpt::char_constants<typename Tstring::value_type>::a <= c && c <= mpt::char_constants<typename Tstring::value_type>::z) {
				c -= mpt::char_constants<typename Tstring::value_type>::a - mpt::char_constants<typename Tstring::value_type>::A;
			}
		}
	}
	return str;
}


template <typename Tstring>
inline Tstring format_simple_integer_postprocess_digits(Tstring str, const format_simple_spec<Tstring> & format) {
	format_simple_flags f = format.GetFlags();
	std::size_t width = format.GetWidth();
	if (f & format_simple_base::FillNul) {
		auto pos = str.begin();
		if (str.length() > 0) {
			if (str[0] == mpt::char_constants<typename Tstring::value_type>::plus) {
				pos++;
				width++;
			} else if (str[0] == mpt::char_constants<typename Tstring::value_type>::minus) {
				pos++;
				width++;
			}
		}
		if (str.length() < width) {
			str.insert(pos, width - str.length(), mpt::char_constants<typename Tstring::value_type>::number0);
		}
	}
	return str;
}


#if MPT_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable : 4723) // potential divide by 0
#endif                          // MPT_COMPILER_MSVC
template <typename Tstring>
inline Tstring format_simple_integer_postprocess_group(Tstring str, const format_simple_spec<Tstring> & format) {
	if (format.GetGroup() > 0) {
		const unsigned int groupSize = format.GetGroup();
		const Tstring groupSep = format.GetGroupSep();
		std::size_t len = str.length();
		for (std::size_t n = 0; n < len; ++n) {
			if (n > 0 && (n % groupSize) == 0) {
				if (!(n == (len - 1) && (str[0] == mpt::char_constants<typename Tstring::value_type>::plus || str[0] == mpt::char_constants<typename Tstring::value_type>::minus))) {
					str.insert(len - n, groupSep);
				}
			}
		}
	}
	return str;
}
#if MPT_COMPILER_MSVC
#pragma warning(pop)
#endif // MPT_COMPILER_MSVC


#if MPT_FORMAT_FORMAT_SIMPLE_INT_CXX17

template <typename Tstring, typename T, std::enable_if_t<std::is_integral<T>::value, bool> = true>
inline Tstring format_simple(const T & x, const format_simple_spec<Tstring> & format) {
	int base = 10;
	if (format.GetFlags() & format_simple_base::BaseDec) {
		base = 10;
	}
	if (format.GetFlags() & format_simple_base::BaseHex) {
		base = 16;
	}
	using format_string_type = typename mpt::select_format_string_type<Tstring>::type;
	const format_simple_spec<format_string_type> f = mpt::transcode_format_simple_spec<format_string_type>(format);
	return mpt::transcode<Tstring>(mpt::format_simple_integer_postprocess_group(mpt::format_simple_integer_postprocess_digits(mpt::format_simple_integer_postprocess_case(mpt::format_simple_integer_to_chars<format_string_type>(x, base), f), f), f));
}

#else // !MPT_FORMAT_FORMAT_SIMPLE_INT_CXX17

template <typename Tstring, typename T, std::enable_if_t<std::is_integral<T>::value, bool> = true>
inline Tstring format_simple(const T & x, const format_simple_spec<Tstring> & format) {
	int base = 10;
	if (format.GetFlags() & format_simple_base::BaseDec) {
		base = 10;
	}
	if (format.GetFlags() & format_simple_base::BaseHex) {
		base = 16;
	}
	using format_string_type = typename mpt::select_format_string_type<Tstring>::type;
	const format_simple_spec<format_string_type> f = mpt::transcode_format_simple_spec<format_string_type>(format);
	return mpt::transcode<Tstring>(mpt::format_simple_integer_postprocess_group(mpt::format_simple_integer_postprocess_digits(mpt::format_simple_integer_postprocess_case(mpt::format_simple_integer_to_stream<format_string_type>(x, base), f), f), f));
}

#endif // MPT_FORMAT_FORMAT_SIMPLE_INT_CXX17


} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_FORMAT_SIMPLE_INTEGER_HPP
