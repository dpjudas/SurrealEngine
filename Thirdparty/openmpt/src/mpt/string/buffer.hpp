/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_STRING_BUFFER_HPP
#define MPT_STRING_BUFFER_HPP



#include "mpt/base/detect.hpp"
#include "mpt/base/namespace.hpp"
#include "mpt/base/saturate_cast.hpp"
#include "mpt/detect/mfc.hpp"
#include "mpt/string/types.hpp"
#include "mpt/string/utility.hpp"

#include <algorithm>
#include <array>
#include <iterator>
#include <string>
#include <string_view>
#include <type_traits>

#include <cassert>
#include <cstddef>

#if MPT_DETECTED_MFC
// cppcheck-suppress missingInclude
#include <afx.h>
#endif // MPT_DETECTED_MFC



namespace mpt {
inline namespace MPT_INLINE_NS {




template <typename Tstring, typename Tchar>
class StringBufRefImpl {
private:
	Tchar * buf;
	std::size_t size;

public:
	// cppcheck false-positive
	// cppcheck-suppress uninitMemberVar
	explicit StringBufRefImpl(Tchar * buf_, std::size_t size_)
		: buf(buf_)
		, size(size_) {
		static_assert(sizeof(Tchar) == sizeof(typename Tstring::value_type));
		assert(size > 0);
	}
	StringBufRefImpl(const StringBufRefImpl &) = delete;
	StringBufRefImpl(StringBufRefImpl &&) = default;
	StringBufRefImpl & operator=(const StringBufRefImpl &) = delete;
	StringBufRefImpl & operator=(StringBufRefImpl &&) = delete;
	operator Tstring() const {
		std::size_t len = std::find(buf, buf + size, char_constants<Tchar>::null) - buf; // terminate at \0
		return Tstring(buf, buf + len);
	}
	explicit operator std::basic_string_view<Tchar>() const {
		std::size_t len = std::find(buf, buf + size, char_constants<Tchar>::null) - buf; // terminate at \0
		return std::basic_string_view<Tchar>(buf, buf + len);
	}
	bool empty() const {
		return buf[0] == char_constants<Tchar>::null;
	}
	StringBufRefImpl & operator=(const Tstring & str) {
		std::copy(str.data(), str.data() + std::min(str.length(), size - 1), buf);
		std::fill(buf + std::min(str.length(), size - 1), buf + size, char_constants<Tchar>::null);
		return *this;
	}
};

template <typename Tstring, typename Tchar>
class StringBufRefImpl<Tstring, const Tchar> {
private:
	const Tchar * buf;
	std::size_t size;

public:
	// cppcheck false-positive
	// cppcheck-suppress uninitMemberVar
	explicit StringBufRefImpl(const Tchar * buf_, std::size_t size_)
		: buf(buf_)
		, size(size_) {
		static_assert(sizeof(Tchar) == sizeof(typename Tstring::value_type));
		assert(size > 0);
	}
	StringBufRefImpl(const StringBufRefImpl &) = delete;
	StringBufRefImpl(StringBufRefImpl &&) = default;
	StringBufRefImpl & operator=(const StringBufRefImpl &) = delete;
	StringBufRefImpl & operator=(StringBufRefImpl &&) = delete;
	operator Tstring() const {
		std::size_t len = std::find(buf, buf + size, char_constants<Tchar>::null) - buf; // terminate at \0
		return Tstring(buf, buf + len);
	}
	explicit operator std::basic_string_view<Tchar>() const {
		std::size_t len = std::find(buf, buf + size, char_constants<Tchar>::null) - buf; // terminate at \0
		return std::basic_string_view<Tchar>(buf, len);
	}
	bool empty() const {
		return buf[0] == char_constants<Tchar>::null;
	}
};

template <typename Tstring, typename Tchar>
struct make_string_type<StringBufRefImpl<Tstring, Tchar>> {
	using type = Tstring;
};

template <typename Tstring, typename Tchar>
struct make_string_view_type<StringBufRefImpl<Tstring, Tchar>> {
	using type = typename mpt::make_string_view_type<Tstring>::type;
};



template <typename Tstring, typename Tchar, std::size_t size>
inline StringBufRefImpl<Tstring, typename std::add_const<Tchar>::type> ReadTypedBuf(const std::array<Tchar, size> & buf) {
	return StringBufRefImpl<Tstring, typename std::add_const<Tchar>::type>(buf.data(), size);
}
template <typename Tstring, typename Tchar, std::size_t size>
inline StringBufRefImpl<Tstring, typename std::add_const<Tchar>::type> ReadTypedBuf(const Tchar (&buf)[size]) {
	return StringBufRefImpl<Tstring, typename std::add_const<Tchar>::type>(buf, size);
}
template <typename Tstring, typename Tchar>
inline StringBufRefImpl<Tstring, typename std::add_const<Tchar>::type> ReadTypedBuf(const Tchar * buf, std::size_t size) {
	return StringBufRefImpl<Tstring, typename std::add_const<Tchar>::type>(buf, size);
}
template <typename Tstring, typename Tchar, std::size_t size>
inline StringBufRefImpl<Tstring, Tchar> WriteTypedBuf(std::array<Tchar, size> & buf) {
	return StringBufRefImpl<Tstring, Tchar>(buf.data(), size);
}
template <typename Tstring, typename Tchar, std::size_t size>
inline StringBufRefImpl<Tstring, Tchar> WriteTypedBuf(Tchar (&buf)[size]) {
	return StringBufRefImpl<Tstring, Tchar>(buf, size);
}
template <typename Tstring, typename Tchar>
inline StringBufRefImpl<Tstring, Tchar> WriteTypedBuf(Tchar * buf, std::size_t size) {
	return StringBufRefImpl<Tstring, Tchar>(buf, size);
}



template <typename Tchar, std::size_t size>
inline StringBufRefImpl<typename std::basic_string<typename std::remove_const<Tchar>::type>, typename std::add_const<Tchar>::type> ReadAutoBuf(const std::array<Tchar, size> & buf) {
	return StringBufRefImpl<typename std::basic_string<typename std::remove_const<Tchar>::type>, typename std::add_const<Tchar>::type>(buf.data(), size);
}
template <typename Tchar, std::size_t size>
inline StringBufRefImpl<typename std::basic_string<typename std::remove_const<Tchar>::type>, typename std::add_const<Tchar>::type> ReadAutoBuf(const Tchar (&buf)[size]) {
	return StringBufRefImpl<typename std::basic_string<typename std::remove_const<Tchar>::type>, typename std::add_const<Tchar>::type>(buf, size);
}
template <typename Tchar>
inline StringBufRefImpl<typename std::basic_string<typename std::remove_const<Tchar>::type>, typename std::add_const<Tchar>::type> ReadAutoBuf(const Tchar * buf, std::size_t size) {
	return StringBufRefImpl<typename std::basic_string<typename std::remove_const<Tchar>::type>, typename std::add_const<Tchar>::type>(buf, size);
}
template <typename Tchar, std::size_t size>
inline StringBufRefImpl<typename std::basic_string<typename std::remove_const<Tchar>::type>, Tchar> WriteAutoBuf(std::array<Tchar, size> & buf) {
	return StringBufRefImpl<typename std::basic_string<typename std::remove_const<Tchar>::type>, Tchar>(buf.data(), size);
}
template <typename Tchar, std::size_t size>
inline StringBufRefImpl<typename std::basic_string<typename std::remove_const<Tchar>::type>, Tchar> WriteAutoBuf(Tchar (&buf)[size]) {
	return StringBufRefImpl<typename std::basic_string<typename std::remove_const<Tchar>::type>, Tchar>(buf, size);
}
template <typename Tchar>
inline StringBufRefImpl<typename std::basic_string<typename std::remove_const<Tchar>::type>, Tchar> WriteAutoBuf(Tchar * buf, std::size_t size) {
	return StringBufRefImpl<typename std::basic_string<typename std::remove_const<Tchar>::type>, Tchar>(buf, size);
}



#if MPT_OS_WINDOWS

template <typename Tchar, std::size_t size>
inline StringBufRefImpl<typename mpt::windows_char_traits<typename std::remove_const<Tchar>::type>::string_type, typename std::add_const<Tchar>::type> ReadWinBuf(const std::array<Tchar, size> & buf) {
	return StringBufRefImpl<typename mpt::windows_char_traits<typename std::remove_const<Tchar>::type>::string_type, typename std::add_const<Tchar>::type>(buf.data(), size);
}
template <typename Tchar, std::size_t size>
inline StringBufRefImpl<typename mpt::windows_char_traits<typename std::remove_const<Tchar>::type>::string_type, typename std::add_const<Tchar>::type> ReadWinBuf(const Tchar (&buf)[size]) {
	return StringBufRefImpl<typename mpt::windows_char_traits<typename std::remove_const<Tchar>::type>::string_type, typename std::add_const<Tchar>::type>(buf, size);
}
template <typename Tchar>
inline StringBufRefImpl<typename mpt::windows_char_traits<typename std::remove_const<Tchar>::type>::string_type, typename std::add_const<Tchar>::type> ReadWinBuf(const Tchar * buf, std::size_t size) {
	return StringBufRefImpl<typename mpt::windows_char_traits<typename std::remove_const<Tchar>::type>::string_type, typename std::add_const<Tchar>::type>(buf, size);
}
template <typename Tchar, std::size_t size>
inline StringBufRefImpl<typename mpt::windows_char_traits<typename std::remove_const<Tchar>::type>::string_type, Tchar> WriteWinBuf(std::array<Tchar, size> & buf) {
	return StringBufRefImpl<typename mpt::windows_char_traits<typename std::remove_const<Tchar>::type>::string_type, Tchar>(buf.data(), size);
}
template <typename Tchar, std::size_t size>
inline StringBufRefImpl<typename mpt::windows_char_traits<typename std::remove_const<Tchar>::type>::string_type, Tchar> WriteWinBuf(Tchar (&buf)[size]) {
	return StringBufRefImpl<typename mpt::windows_char_traits<typename std::remove_const<Tchar>::type>::string_type, Tchar>(buf, size);
}
template <typename Tchar>
inline StringBufRefImpl<typename mpt::windows_char_traits<typename std::remove_const<Tchar>::type>::string_type, Tchar> WriteWinBuf(Tchar * buf, std::size_t size) {
	return StringBufRefImpl<typename mpt::windows_char_traits<typename std::remove_const<Tchar>::type>::string_type, Tchar>(buf, size);
}

#endif // MPT_OS_WINDOWS



#if MPT_DETECTED_MFC

template <typename Tchar>
class CStringBufRefImpl {
private:
	Tchar * buf;
	std::size_t size;

public:
	// cppcheck false-positive
	// cppcheck-suppress uninitMemberVar
	explicit CStringBufRefImpl(Tchar * buf_, std::size_t size_)
		: buf(buf_)
		, size(size_) {
		assert(size > 0);
	}
	CStringBufRefImpl(const CStringBufRefImpl &) = delete;
	CStringBufRefImpl(CStringBufRefImpl &&) = default;
	CStringBufRefImpl & operator=(const CStringBufRefImpl &) = delete;
	CStringBufRefImpl & operator=(CStringBufRefImpl &&) = delete;
	operator CString() const {
		std::size_t len = std::find(buf, buf + size, char_constants<Tchar>::null) - buf; // terminate at \0
		return CString(buf, mpt::saturate_cast<int>(len));
	}
	CStringBufRefImpl & operator=(const CString & str) {
		std::copy(str.GetString(), str.GetString() + std::min(static_cast<std::size_t>(str.GetLength()), size - 1), buf);
		std::fill(buf + std::min(static_cast<std::size_t>(str.GetLength()), size - 1), buf + size, char_constants<Tchar>::null);
		return *this;
	}
};

template <typename Tchar>
class CStringBufRefImpl<const Tchar> {
private:
	const Tchar * buf;
	std::size_t size;

public:
	// cppcheck false-positive
	// cppcheck-suppress uninitMemberVar
	explicit CStringBufRefImpl(const Tchar * buf_, std::size_t size_)
		: buf(buf_)
		, size(size_) {
		assert(size > 0);
	}
	CStringBufRefImpl(const CStringBufRefImpl &) = delete;
	CStringBufRefImpl(CStringBufRefImpl &&) = default;
	CStringBufRefImpl & operator=(const CStringBufRefImpl &) = delete;
	CStringBufRefImpl & operator=(CStringBufRefImpl &&) = delete;
	operator CString() const {
		std::size_t len = std::find(buf, buf + size, char_constants<Tchar>::null) - buf; // terminate at \0
		return CString(buf, mpt::saturate_cast<int>(len));
	}
};

template <typename Tchar>
struct make_string_type<CStringBufRefImpl<Tchar>> {
	using type = CString;
};

template <typename Tchar>
struct make_string_view_type<CStringBufRefImpl<Tchar>> {
	using type = CString;
};

template <typename Tchar, std::size_t size>
inline CStringBufRefImpl<typename std::add_const<Tchar>::type> ReadCStringBuf(const std::array<Tchar, size> & buf) {
	return CStringBufRefImpl<typename std::add_const<Tchar>::type>(buf.data(), size);
}
template <typename Tchar, std::size_t size>
inline CStringBufRefImpl<typename std::add_const<Tchar>::type> ReadCStringBuf(const Tchar (&buf)[size]) {
	return CStringBufRefImpl<typename std::add_const<Tchar>::type>(buf, size);
}
template <typename Tchar>
inline CStringBufRefImpl<typename std::add_const<Tchar>::type> ReadCStringBuf(const Tchar * buf, std::size_t size) {
	return CStringBufRefImpl<typename std::add_const<Tchar>::type>(buf, size);
}
template <typename Tchar, std::size_t size>
inline CStringBufRefImpl<Tchar> WriteCStringBuf(std::array<Tchar, size> & buf) {
	return CStringBufRefImpl<Tchar>(buf.data(), size);
}
template <typename Tchar, std::size_t size>
inline CStringBufRefImpl<Tchar> WriteCStringBuf(Tchar (&buf)[size]) {
	return CStringBufRefImpl<Tchar>(buf, size);
}
template <typename Tchar>
inline CStringBufRefImpl<Tchar> WriteCStringBuf(Tchar * buf, std::size_t size) {
	return CStringBufRefImpl<Tchar>(buf, size);
}

#endif // MPT_DETECTED_MFC



template <std::size_t len>
struct charbuf {
public:
	using Tchar = char;
	using char_type = Tchar;
	using string_type = std::basic_string<Tchar>;
	using string_view_type = std::basic_string_view<Tchar>;
	constexpr std::size_t static_length() const {
		return len;
	}

public:
	Tchar buf[len]{};

public:
	constexpr charbuf() {
		for (std::size_t i = 0; i < len; ++i) {
			buf[i] = char_constants<Tchar>::null;
		}
	}
	constexpr charbuf(const charbuf &) = default;
	constexpr charbuf(charbuf &&) = default;
	constexpr charbuf & operator=(const charbuf &) = default;
	constexpr charbuf & operator=(charbuf &&) = default;
	const Tchar & operator[](std::size_t i) const {
		return buf[i];
	}
	std::string str() const {
		return static_cast<std::string>(*this);
	}
	operator string_type() const {
		return mpt::ReadAutoBuf(buf);
	}
	explicit operator string_view_type() const {
		return static_cast<string_view_type>(mpt::ReadAutoBuf(buf));
	}
	bool empty() const {
		return mpt::ReadAutoBuf(buf).empty();
	}
	charbuf & operator=(const string_type & str) {
		mpt::WriteAutoBuf(buf) = str;
		return *this;
	}

public:
	friend bool operator!=(const charbuf & a, const charbuf & b) {
		return static_cast<string_view_type>(a) != static_cast<string_view_type>(b);
	}
	friend bool operator!=(const std::string & a, const charbuf & b) {
		return a != static_cast<string_view_type>(b);
	}
	friend bool operator!=(const charbuf & a, const std::string & b) {
		return static_cast<string_view_type>(a) != b;
	}
	friend bool operator==(const charbuf & a, const charbuf & b) {
		return static_cast<string_view_type>(a) == static_cast<string_view_type>(b);
	}
	friend bool operator==(const std::string & a, const charbuf & b) {
		return a == static_cast<string_view_type>(b);
	}
	friend bool operator==(const charbuf & a, const std::string & b) {
		return static_cast<string_view_type>(a) == b;
	}
};

template <std::size_t len>
struct make_string_type<charbuf<len>> {
	using type = std::string;
};

template <std::size_t len>
struct make_string_view_type<charbuf<len>> {
	using type = std::string_view;
};


} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_STRING_BUFFER_HPP
