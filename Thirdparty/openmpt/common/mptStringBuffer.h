/*
 * mptStringBuffer.h
 * -----------------
 * Purpose: Various functions for "fixing" char array strings for writing to or
 *          reading from module files, or for securing char arrays in general.
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#pragma once

#include "openmpt/all/BuildSettings.hpp"

#include "mpt/string/buffer.hpp"

#include "mptString.h"

#include <algorithm>
#include <string>
#include <vector>



OPENMPT_NAMESPACE_BEGIN



namespace mpt
{


namespace String
{


	enum ReadWriteMode : uint8
	{
		// Reading / Writing: Standard null-terminated string handling.
		nullTerminated = 1,
		// Reading: Source string is not guaranteed to be null-terminated (if it fills the whole char array).
		// Writing: Destination string is not guaranteed to be null-terminated (if it fills the whole char array).
		maybeNullTerminated = 2,
		// Reading: String may contain null characters anywhere. They should be treated as spaces.
		// Writing: A space-padded string is written.
		spacePadded = 3,
		// Reading: String may contain null characters anywhere. The last character is ignored (it is supposed to be 0).
		// Writing: A space-padded string with a trailing null is written.
		spacePaddedNull = 4,
	};
	
	namespace detail
	{

	std::string ReadStringBuffer(String::ReadWriteMode mode, const char *srcBuffer, std::size_t srcSize);

	void WriteStringBuffer(String::ReadWriteMode mode, char *destBuffer, const std::size_t destSize, const char *srcBuffer, const std::size_t srcSize);

	} // namespace detail


} // namespace String



namespace String {
using mpt::ReadTypedBuf;
using mpt::WriteTypedBuf;
} // namespace String

namespace String {
using mpt::ReadAutoBuf;
using mpt::WriteAutoBuf;
} // namespace String



template <typename Tchar>
class StringModeBufRefImpl
{
private:
	Tchar * buf;
	std::size_t size;
	String::ReadWriteMode mode;
public:
	// cppcheck false-positive
	// cppcheck-suppress uninitMemberVar
	StringModeBufRefImpl(Tchar * buf_, std::size_t size_, String::ReadWriteMode mode_)
		: buf(buf_)
		, size(size_)
		, mode(mode_)
	{
		static_assert(sizeof(Tchar) == 1);
	}
	StringModeBufRefImpl(const StringModeBufRefImpl &) = delete;
	StringModeBufRefImpl(StringModeBufRefImpl &&) = default;
	StringModeBufRefImpl & operator = (const StringModeBufRefImpl &) = delete;
	StringModeBufRefImpl & operator = (StringModeBufRefImpl &&) = delete;
	operator std::string () const
	{
		return String::detail::ReadStringBuffer(mode, buf, size);
	}
	bool empty() const
	{
		return String::detail::ReadStringBuffer(mode, buf, size).empty();
	}
	StringModeBufRefImpl & operator = (const std::string & str)
	{
		String::detail::WriteStringBuffer(mode, buf, size, str.data(), str.size());
		return *this;
	}
};

template <typename Tchar>
class StringModeBufRefImpl<const Tchar>
{
private:
	const Tchar * buf;
	std::size_t size;
	String::ReadWriteMode mode;
public:
	// cppcheck false-positive
	// cppcheck-suppress uninitMemberVar
	StringModeBufRefImpl(const Tchar * buf_, std::size_t size_, String::ReadWriteMode mode_)
		: buf(buf_)
		, size(size_)
		, mode(mode_)
	{
		static_assert(sizeof(Tchar) == 1);
	}
	StringModeBufRefImpl(const StringModeBufRefImpl &) = delete;
	StringModeBufRefImpl(StringModeBufRefImpl &&) = default;
	StringModeBufRefImpl & operator = (const StringModeBufRefImpl &) = delete;
	StringModeBufRefImpl & operator = (StringModeBufRefImpl &&) = delete;
	operator std::string () const
	{
		return String::detail::ReadStringBuffer(mode, buf, size);
	}
	bool empty() const
	{
		return String::detail::ReadStringBuffer(mode, buf, size).empty();
	}
};

namespace String {
template <typename Tchar, std::size_t size>
inline StringModeBufRefImpl<typename std::add_const<Tchar>::type> ReadBuf(String::ReadWriteMode mode, const std::array<Tchar, size> &buf)
{
	return StringModeBufRefImpl<typename std::add_const<Tchar>::type>(buf.data(), size, mode);
}
template <typename Tchar, std::size_t size>
inline StringModeBufRefImpl<typename std::add_const<Tchar>::type> ReadBuf(String::ReadWriteMode mode, const Tchar (&buf)[size])
{
	return StringModeBufRefImpl<typename std::add_const<Tchar>::type>(buf, size, mode);
}
template <typename Tchar>
inline StringModeBufRefImpl<typename std::add_const<Tchar>::type> ReadBuf(String::ReadWriteMode mode, const Tchar * buf, std::size_t size)
{
	return StringModeBufRefImpl<typename std::add_const<Tchar>::type>(buf, size, mode);
}
template <typename Tchar, std::size_t size>
inline StringModeBufRefImpl<Tchar> WriteBuf(String::ReadWriteMode mode, std::array<Tchar, size> &buf)
{
	return StringModeBufRefImpl<Tchar>(buf.data(), size, mode);
}
template <typename Tchar, std::size_t size>
inline StringModeBufRefImpl<Tchar> WriteBuf(String::ReadWriteMode mode, Tchar (&buf)[size])
{
	return StringModeBufRefImpl<Tchar>(buf, size, mode);
}
template <typename Tchar>
inline StringModeBufRefImpl<Tchar> WriteBuf(String::ReadWriteMode mode, Tchar * buf, std::size_t size)
{
	return StringModeBufRefImpl<Tchar>(buf, size, mode);
}
} // namespace String

template <std::size_t len, mpt::String::ReadWriteMode mode>
struct modecharbuf
{
public:
	using Tchar = char;
	using char_type = Tchar;
	using string_type = std::basic_string<Tchar>;
public:
	Tchar buf[len];
public:
	modecharbuf() = default;
	modecharbuf(const modecharbuf &) = default;
	modecharbuf(modecharbuf &&) = default;
	modecharbuf & operator = (const modecharbuf &) = default;
	modecharbuf & operator = (modecharbuf &&) = default;
	operator string_type () const
	{
		return mpt::String::ReadBuf(mode, buf);
	}
	bool empty() const
	{
		return mpt::String::ReadBuf(mode, buf).empty();
	}
	modecharbuf & operator = (const string_type & str)
	{
		mpt::String::WriteBuf(mode, buf) = str;
		return *this;
	}
};


// see MPT_BINARY_STRUCT
template <std::size_t len, mpt::String::ReadWriteMode mode>
constexpr bool declare_binary_safe(const typename mpt::modecharbuf<len, mode> &) { return true; }
//struct is_binary_safe<typename mpt::modecharbuf<len, mode>> : public std::true_type { };
static_assert(sizeof(mpt::modecharbuf<7, mpt::String::ReadWriteMode::nullTerminated>) == 7);
static_assert(alignof(mpt::modecharbuf<7, mpt::String::ReadWriteMode::nullTerminated>) == 1);
static_assert(std::is_standard_layout<mpt::modecharbuf<7, mpt::String::ReadWriteMode::nullTerminated>>::value);


#ifdef MODPLUG_TRACKER

#if MPT_OS_WINDOWS

namespace String {
using mpt::ReadWinBuf;
using mpt::WriteWinBuf;
} // namespace String

#if defined(MPT_WITH_MFC)

namespace String {
using mpt::ReadCStringBuf;
using mpt::WriteCStringBuf;
} // namespace String

#endif // MPT_WITH_MFC

#endif // MPT_OS_WINDOWS

#endif // MODPLUG_TRACKER





namespace String
{


#if MPT_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable:4127) // conditional expression is constant
#endif // MPT_COMPILER_MSVC


	// Sets last character to null in given char array.
	// Size of the array must be known at compile time.
	template <size_t size>
	void SetNullTerminator(char (&buffer)[size])
	{
		static_assert(size > 0);
		buffer[size - 1] = 0;
	}

	inline void SetNullTerminator(char *buffer, size_t size)
	{
		MPT_ASSERT(size > 0);
		buffer[size - 1] = 0;
	}

#if !defined(MPT_COMPILER_QUIRK_NO_WCHAR)

	template <size_t size>
	void SetNullTerminator(wchar_t (&buffer)[size])
	{
		static_assert(size > 0);
		buffer[size - 1] = 0;
	}

	inline void SetNullTerminator(wchar_t *buffer, size_t size)
	{
		MPT_ASSERT(size > 0);
		buffer[size - 1] = 0;
	}

#endif // !MPT_COMPILER_QUIRK_NO_WCHAR

#if MPT_COMPILER_MSVC
#pragma warning(pop)
#endif // MPT_COMPILER_MSVC


} // namespace String


} // namespace mpt



OPENMPT_NAMESPACE_END



template <typename Tchar>
struct mpt::make_string_type<OPENMPT_NAMESPACE::mpt::StringModeBufRefImpl<Tchar>> {
	using type = std::basic_string<typename std::remove_const<Tchar>::type>;
};

template <typename Tchar>
struct mpt::make_string_view_type<OPENMPT_NAMESPACE::mpt::StringModeBufRefImpl<Tchar>> {
	using type = std::basic_string_view<typename std::remove_const<Tchar>::type>;
};


template <std::size_t len, OPENMPT_NAMESPACE::mpt::String::ReadWriteMode mode>
struct mpt::make_string_type<OPENMPT_NAMESPACE::mpt::modecharbuf<len, mode>> {
	using type = std::string;
};

template <std::size_t len, OPENMPT_NAMESPACE::mpt::String::ReadWriteMode mode>
struct mpt::make_string_view_type<OPENMPT_NAMESPACE::mpt::modecharbuf<len, mode>> {
	using type = std::string_view;
};
