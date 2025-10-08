/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_FORMAT_MESSAGE_HPP
#define MPT_FORMAT_MESSAGE_HPP



#include "mpt/base/macros.hpp"
#include "mpt/base/namespace.hpp"
#include "mpt/base/span.hpp"
#include "mpt/string/types.hpp"
#include "mpt/string/utility.hpp"

#include <array>
#include <stdexcept>
#include <utility>

#include <cstddef>



namespace mpt {
inline namespace MPT_INLINE_NS {



class format_message_syntax_error
	: public std::domain_error {
public:
	format_message_syntax_error()
		: std::domain_error("format string syntax error") {
		return;
	}
};

template <typename Tformatter, typename Tstring>
class message_formatter {

private:
	using Tstringview = typename mpt::make_string_view_type<Tstring>::type;

private:
	Tstring format;

private:
	MPT_NOINLINE Tstring do_format(const mpt::span<const Tstring> vals) const {
		using traits = typename mpt::string_traits<Tstring>;
		using char_type = typename traits::char_type;
		using size_type = typename traits::size_type;
		Tstring result;
		const size_type len = traits::length(format);
		traits::reserve(result, len);
		std::size_t max_arg = 0;
		//std::size_t args = 0;
		bool success = true;
		enum class state : int {
			error = -1,
			text = 0,
			open_seen = 1,
			number_seen = 2,
			close_seen = 3,
		};
		state state = state::text;
		bool numbered_args = false;
		bool unnumbered_args = false;
		std::size_t last_arg = 0;
		std::size_t this_arg = 0;
		std::size_t current_arg = 0;
		for (size_type pos = 0; pos != len; ++pos) {
			char_type c = format[pos];
			switch (state) {
				case state::text:
					if (c == char_type('{')) {
						state = state::open_seen;
					} else if (c == char_type('}')) {
						state = state::close_seen;
					} else {
						state = state::text;
						traits::append(result, c); // output c here
					}
					break;
				case state::open_seen:
					if (c == char_type('{')) {
						state = state::text;
						traits::append(result, char_type('{')); // output { here
					} else if (c == char_type('}')) {
						state = state::text;
						unnumbered_args = true;
						last_arg++;
						this_arg = last_arg;
						{ // output this_arg here
							const std::size_t n = this_arg - 1;
							if (n < std::size(vals)) {
								traits::append(result, vals[n]);
							}
						}
						if (this_arg > max_arg) {
							max_arg = this_arg;
						}
						//args += 1;
					} else if (char_type('0') <= c && c <= char_type('9')) {
						state = state::number_seen;
						numbered_args = true;
						current_arg = c - char_type('0');
					} else {
						state = state::error;
					}
					break;
				case state::number_seen:
					if (c == char_type('{')) {
						state = state::error;
					} else if (c == char_type('}')) {
						state = state::text;
						this_arg = current_arg + 1;
						{ // output this_arg here
							const std::size_t n = this_arg - 1;
							if (n < std::size(vals)) {
								traits::append(result, vals[n]);
							}
						}
						if (this_arg > max_arg) {
							max_arg = this_arg;
						}
						//args += 1;
					} else if (char_type('0') <= c && c <= char_type('9')) {
						state = state::number_seen;
						numbered_args = true;
						current_arg = (current_arg * 10) + (c - char_type('0'));
					} else {
						state = state::error;
					}
					break;
				case state::close_seen:
					if (c == char_type('{')) {
						state = state::error;
					} else if (c == char_type('}')) {
						state = state::text;
						traits::append(result, char_type('}')); // output } here
					} else {
						state = state::error;
					}
					break;
				case state::error:
					state = state::error;
					break;
			}
		}
		if (state == state::error) {
			success = false;
		}
		if (state != state::text) {
			success = false;
		}
		if (numbered_args && unnumbered_args) {
			success = false;
		}
		if (!success) {
			throw format_message_syntax_error();
		}
		return result;
	}

public:
	MPT_FORCEINLINE message_formatter(Tstring format_)
		: format(std::move(format_)) {
	}

public:
	template <typename... Ts>
	MPT_NOINLINE Tstring operator()(Ts &&... xs) const {
		const std::array<Tstring, sizeof...(xs)> vals{{Tformatter::template format<Tstring>(std::forward<Ts>(xs))...}};
		return do_format(mpt::as_span(vals));
	}

}; // class message_formatter


template <typename Tformatter, std::ptrdiff_t N, typename Tchar, typename Tstring>
class message_formatter_counted {

private:
	using Tstringview = typename mpt::make_string_view_type<Tstring>::type;

private:
	message_formatter<Tformatter, Tstring> formatter;

public:
	template <std::size_t literal_length>
	inline message_formatter_counted(const Tchar (&format)[literal_length])
		: formatter(Tstring(format)) {
		return;
	}

public:
	template <typename... Ts>
	inline Tstring operator()(Ts &&... xs) const {
		static_assert(static_cast<std::ptrdiff_t>(sizeof...(xs)) == N);
		return formatter(std::forward<Ts>(xs)...);
	}

}; // struct message_formatter_counted


template <typename Tchar>
MPT_CONSTEXPRINLINE std::ptrdiff_t parse_format_string_argument_count_impl(const Tchar * const format, const std::size_t len) {
	std::size_t max_arg = 0;
	std::size_t args = 0;
	bool success = true;
	enum class state : int {
		error = -1,
		text = 0,
		open_seen = 1,
		number_seen = 2,
		close_seen = 3,
	};
	state state = state::text;
	bool numbered_args = false;
	bool unnumbered_args = false;
	std::size_t last_arg = 0;
	std::size_t this_arg = 0;
	std::size_t current_arg = 0;
	for (std::size_t pos = 0; pos != len; ++pos) {
		Tchar c = format[pos];
		switch (state) {
			case state::text:
				if (c == Tchar('{')) {
					state = state::open_seen;
				} else if (c == Tchar('}')) {
					state = state::close_seen;
				} else {
					state = state::text;
					// output c here
				}
				break;
			case state::open_seen:
				if (c == Tchar('{')) {
					state = state::text;
					// output { here
				} else if (c == Tchar('}')) {
					state = state::text;
					unnumbered_args = true;
					last_arg++;
					this_arg = last_arg;
					// output this_arg here
					if (this_arg > max_arg)
					{
						max_arg = this_arg;
					}
					args += 1;
				} else if (Tchar('0') <= c && c <= Tchar('9')) {
					state = state::number_seen;
					numbered_args = true;
					current_arg = c - Tchar('0');
				} else {
					state = state::error;
				}
				break;
			case state::number_seen:
				if (c == Tchar('{')) {
					state = state::error;
				} else if (c == Tchar('}')) {
					state = state::text;
					this_arg = current_arg + 1;
					// output this_arg here
					if (this_arg > max_arg) {
						max_arg = this_arg;
					}
					args += 1;
				} else if (Tchar('0') <= c && c <= Tchar('9')) {
					state = state::number_seen;
					numbered_args = true;
					current_arg = (current_arg * 10) + (c - Tchar('0'));
				} else {
					state = state::error;
				}
				break;
			case state::close_seen:
				if (c == Tchar('{')) {
					state = state::error;
				} else if (c == Tchar('}')) {
					state = state::text;
					// output } here
				} else {
					state = state::error;
				}
				break;
			case state::error:
				state = state::error;
				break;
		}
	}
	if (state == state::error) {
		success = false;
	}
	if (state != state::text) {
		success = false;
	}
	if (numbered_args && unnumbered_args) {
		success = false;
	}
	if (!success) {
		throw format_message_syntax_error();
	}
	if (max_arg != args) {
		throw format_message_syntax_error();
	}
	return max_arg;
}


template <typename Tchar, std::size_t literal_length>
MPT_CONSTEXPRINLINE std::ptrdiff_t parse_format_string_argument_count(const Tchar (&format)[literal_length]) {
	return parse_format_string_argument_count_impl(format, literal_length - 1);
}


template <typename Tformatter, std::size_t args, typename Tchar, std::size_t N>
inline auto format_message(const Tchar (&format)[N]) {
	using Tstring = typename mpt::make_string_type<const Tchar *>::type;
	return message_formatter_counted<Tformatter, args, Tchar, Tstring>(format);
}

template <typename Tformatter, std::size_t args, typename Tstring, typename Tchar, std::size_t N>
inline auto format_message_typed(const Tchar (&format)[N]) {
	return message_formatter_counted<Tformatter, args, Tchar, Tstring>(format);
}



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_FORMAT_MESSAGE_HPP
