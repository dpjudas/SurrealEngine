/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_FORMAT_SIMPLE_HPP
#define MPT_FORMAT_SIMPLE_HPP



#include "mpt/base/namespace.hpp"
#include "mpt/base/pointer.hpp"
#include "mpt/format/default_formatter.hpp"
#include "mpt/format/simple_floatingpoint.hpp"
#include "mpt/format/simple_integer.hpp"
#include "mpt/format/simple_spec.hpp"
#include "mpt/string/utility.hpp"

#include <type_traits>

#include <cstddef>



namespace mpt {
inline namespace MPT_INLINE_NS {



template <typename Tstring>
struct format : format_simple_base {

	template <typename T>
	static inline Tstring val(const T & x) {
		return mpt::default_formatter::format<Tstring>(x);
	}

	template <typename T>
	static inline Tstring fmt(const T & x, const format_simple_spec<Tstring> & f) {
		return mpt::format_simple<Tstring>(x, f);
	}

	template <typename T>
	static inline Tstring dec(const T & x) {
		static_assert(std::numeric_limits<T>::is_integer);
		return mpt::format_simple<Tstring>(x, format_simple_spec<Tstring>().BaseDec().FillOff());
	}
	template <int width, typename T>
	static inline Tstring dec0(const T & x) {
		static_assert(std::numeric_limits<T>::is_integer);
		return mpt::format_simple<Tstring>(x, format_simple_spec<Tstring>().BaseDec().FillNul().Width(width));
	}

	template <typename T>
	static inline Tstring dec(unsigned int g, Tstring s, const T & x) {
		static_assert(std::numeric_limits<T>::is_integer);
		return mpt::format_simple<Tstring>(x, format_simple_spec<Tstring>().BaseDec().FillOff().Group(g).GroupSep(s));
	}
	template <int width, typename T>
	static inline Tstring dec0(unsigned int g, Tstring s, const T & x) {
		static_assert(std::numeric_limits<T>::is_integer);
		return mpt::format_simple<Tstring>(x, format_simple_spec<Tstring>().BaseDec().FillNul().Width(width).Group(g).GroupSep(s));
	}

	template <typename T>
	static inline Tstring hex(const T & x) {
		static_assert(std::numeric_limits<T>::is_integer);
		return mpt::format_simple<Tstring>(x, format_simple_spec<Tstring>().BaseHex().CaseLow().FillOff());
	}
	template <typename T>
	static inline Tstring HEX(const T & x) {
		static_assert(std::numeric_limits<T>::is_integer);
		return mpt::format_simple<Tstring>(x, format_simple_spec<Tstring>().BaseHex().CaseUpp().FillOff());
	}
	template <int width, typename T>
	static inline Tstring hex0(const T & x) {
		static_assert(std::numeric_limits<T>::is_integer);
		return mpt::format_simple<Tstring>(x, format_simple_spec<Tstring>().BaseHex().CaseLow().FillNul().Width(width));
	}
	template <int width, typename T>
	static inline Tstring HEX0(const T & x) {
		static_assert(std::numeric_limits<T>::is_integer);
		return mpt::format_simple<Tstring>(x, format_simple_spec<Tstring>().BaseHex().CaseUpp().FillNul().Width(width));
	}

	template <typename T>
	static inline Tstring hex(unsigned int g, Tstring s, const T & x) {
		static_assert(std::numeric_limits<T>::is_integer);
		return mpt::format_simple<Tstring>(x, format_simple_spec<Tstring>().BaseHex().CaseLow().FillOff().Group(g).GroupSep(s));
	}
	template <typename T>
	static inline Tstring HEX(unsigned int g, Tstring s, const T & x) {
		static_assert(std::numeric_limits<T>::is_integer);
		return mpt::format_simple<Tstring>(x, format_simple_spec<Tstring>().BaseHex().CaseUpp().FillOff().Group(g).GroupSep(s));
	}
	template <int width, typename T>
	static inline Tstring hex0(unsigned int g, Tstring s, const T & x) {
		static_assert(std::numeric_limits<T>::is_integer);
		return mpt::format_simple<Tstring>(x, format_simple_spec<Tstring>().BaseHex().CaseLow().FillNul().Width(width).Group(g).GroupSep(s));
	}
	template <int width, typename T>
	static inline Tstring HEX0(unsigned int g, Tstring s, const T & x) {
		static_assert(std::numeric_limits<T>::is_integer);
		return mpt::format_simple<Tstring>(x, format_simple_spec<Tstring>().BaseHex().CaseUpp().FillNul().Width(width).Group(g).GroupSep(s));
	}

	template <typename T>
	static inline Tstring flt(const T & x, int precision = -1) {
		static_assert(std::is_floating_point<T>::value);
		return mpt::format_simple<Tstring>(x, format_simple_spec<Tstring>().NotaNrm().FillOff().Precision(precision));
	}
	template <typename T>
	static inline Tstring fix(const T & x, int precision = -1) {
		static_assert(std::is_floating_point<T>::value);
		return mpt::format_simple<Tstring>(x, format_simple_spec<Tstring>().NotaFix().FillOff().Precision(precision));
	}
	template <typename T>
	static inline Tstring sci(const T & x, int precision = -1) {
		static_assert(std::is_floating_point<T>::value);
		return mpt::format_simple<Tstring>(x, format_simple_spec<Tstring>().NotaSci().FillOff().Precision(precision));
	}

	template <typename T>
	static inline Tstring ptr(const T & x) {
		static_assert(std::is_pointer<T>::value || std::is_same<T, std::uintptr_t>::value || std::is_same<T, std::intptr_t>::value, "");
		return hex0<mpt::pointer_size * 2>(mpt::pointer_cast<const std::uintptr_t>(x));
	}
	template <typename T>
	static inline Tstring PTR(const T & x) {
		static_assert(std::is_pointer<T>::value || std::is_same<T, std::uintptr_t>::value || std::is_same<T, std::intptr_t>::value, "");
		return HEX0<mpt::pointer_size * 2>(mpt::pointer_cast<const std::uintptr_t>(x));
	}

	static inline Tstring pad_left(std::size_t width_, const Tstring & str) {
		typedef mpt::string_traits<Tstring> traits;
		typename traits::size_type width = static_cast<typename traits::size_type>(width_);
		return traits::pad(str, width, 0);
	}
	static inline Tstring pad_right(std::size_t width_, const Tstring & str) {
		typedef mpt::string_traits<Tstring> traits;
		typename traits::size_type width = static_cast<typename traits::size_type>(width_);
		return traits::pad(str, 0, width);
	}
	static inline Tstring left(std::size_t width_, const Tstring & str) {
		typedef mpt::string_traits<Tstring> traits;
		typename traits::size_type width = static_cast<typename traits::size_type>(width_);
		return (traits::length(str) < width) ? traits::pad(str, 0, width - traits::length(str)) : str;
	}
	static inline Tstring right(std::size_t width_, const Tstring & str) {
		typedef mpt::string_traits<Tstring> traits;
		typename traits::size_type width = static_cast<typename traits::size_type>(width_);
		return (traits::length(str) < width) ? traits::pad(str, width - traits::length(str), 0) : str;
	}
	static inline Tstring center(std::size_t width_, const Tstring & str) {
		typedef mpt::string_traits<Tstring> traits;
		typename traits::size_type width = static_cast<typename traits::size_type>(width_);
		return (traits::length(str) < width) ? traits::pad(str, (width - traits::length(str)) / 2, (width - traits::length(str) + 1) / 2) : str;
	}

}; // struct format



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_FORMAT_SIMPLE_HPP
