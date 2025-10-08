/*
 * mptStringFormat.h
 * -----------------
 * Purpose: Convert other types to strings.
 * Notes  : Currently none.
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#pragma once

#include "openmpt/all/BuildSettings.hpp"

#include "mpt/base/detect.hpp"
#include "mpt/endian/integer.hpp"
#include "mpt/format/default_formatter.hpp"
#include "mpt/format/message.hpp"
#include "mpt/format/message_macros.hpp"
#include "mpt/format/simple.hpp"
#include "mpt/format/simple_spec.hpp"
#include "mpt/string/types.hpp"
#include "mpt/string_transcode/transcode.hpp"

#include "openmpt/base/FlagSet.hpp"

#include "mptString.h"

#include <cstddef>



// The following section demands a rationale.
//  1. mpt::afmt::val(), mpt::wfmt::val() and mpt::ufmt::val() mimic the semantics of c++11 std::to_string() and std::to_wstring().
//     There is an important difference though. The c++11 versions are specified in terms of sprintf formatting which in turn
//     depends on the current C locale. This renders these functions unusable in a library context because the current
//     C locale is set by the library-using application and could be anything. There is no way a library can get reliable semantics
//     out of these functions. It is thus better to just avoid them.
//     ToAString() and ToWString() are based on iostream internally, but the the locale of the stream is forced to std::locale::classic(),
//     which results in "C" ASCII locale behavior.
//  2. The full suite of printf-like or iostream like number formatting is generally not required. Instead, a sane subset functionality
//     is provided here.
//     When formatting integers, it is recommended to use mpt::afmt::dec or mpt::afmt::hex. Appending a template argument '<n>' sets the width,
//     the same way as '%nd' would do. Appending a '0' to the function name causes zero-filling as print-like '%0nd' would do. Spelling 'HEX'
//     in upper-case generates upper-case hex digits. If these are not known at compile-time, a more verbose FormatValA(int, format) can be
//     used.
//  3. mpt::format(format)(...) provides simplified and type-safe message and localization string formatting.
//     The only specifier allowed is '{}' enclosing a number n. It references to n-th parameter after the format string (1-based).
//     This mimics the behaviour of QString::arg() in QT4/5 or MFC AfxFormatString2(). C printf-like functions offer similar functionality
//     with a '%n$TYPE' syntax. In .NET, the syntax is '{n}'. This is useful to support localization strings that can change the parameter
//     ordering.
//  4. Every function is available for std::string, std::wstring and mpt::ustring. std::string makes no assumption about the encoding, which
//     basically means, it should work for any 7-bit or 8-bit encoding, including for example ASCII, UTF8 or the current locale encoding.
//     std::string         std::wstring          mpt::ustring                    mpt::tsrtring                        CString
//     mpt::afmt           mpt::wfmt             mpt::ufmt                       mpt::tfmt                            mpt::cfmt
//     MPT_AFORMAT("{}")   MPT_WFORMAT("{}")     MPT_UFORMAT("{}")               MPT_TFORMAT("{}")                    MPT_CFORMAT("{}")
//  5. All functionality here delegates real work outside of the header file so that <sstream> and <locale> do not need to be included when
//     using this functionality.
//     Advantages:
//      - Avoids binary code bloat when too much of iostream operator << gets inlined at every usage site.
//      - Faster compile times because <sstream> and <locale> (2 very complex headers) are not included everywhere.
//     Disadvantages:
//      - Slightly more c++ code is required for delegating work.
//      - As the header does not use iostreams, custom types need to overload mpt::UString instead of iostream operator << to allow for custom type
//        formatting.
//      - std::string, std::wstring and mpt::ustring are returned from somewhat deep cascades of helper functions. Where possible, code is
//        written in such a way that return-value-optimization (RVO) or named-return-value-optimization (NRVO) should be able to eliminate
//        almost all these copies. This should not be a problem for any decent modern compiler (and even less so for a c++11 compiler where
//        move-semantics will kick in if RVO/NRVO fails).



namespace mpt {
inline namespace MPT_INLINE_NS {
template <typename Tstring, typename Tint, mpt::endian endian>
inline auto format_value_default(const mpt::packed<Tint, endian> & x) -> decltype(mpt::default_formatter::format<Tstring, Tint>(x)) {
	return mpt::default_formatter::format<Tstring, Tint>(x);
}
} // namespace MPT_INLINE_NS
} // namespace mpt


OPENMPT_NAMESPACE_BEGIN


template <typename Tstring, typename T>
inline auto format_value_default(const T & x) -> decltype(mpt::transcode<Tstring>(x.ToUString())) {
	return mpt::transcode<Tstring>(x.ToUString());
}


template <typename Tstring, typename T>
inline auto format_value_default(const T & x) -> decltype(mpt::transcode<Tstring>(ToUString(x))) {
	return mpt::transcode<Tstring>(ToUString(x));
}


namespace mpt
{


template <typename Tstring, typename T>
inline auto format_value_default(const T & x) -> decltype(mpt::transcode<Tstring>(x.ToUString())) {
	return mpt::transcode<Tstring>(x.ToUString());
}


template <typename Tstring, typename T>
inline auto format_value_default(const T & x) -> decltype(mpt::transcode<Tstring>(ToUString(x))) {
	return mpt::transcode<Tstring>(ToUString(x));
}


template <typename Tstring>
using fmtT = mpt::format<Tstring>;

using afmt = fmtT<std::string>;
#if !defined(MPT_COMPILER_QUIRK_NO_WCHAR)
using wfmt = fmtT<std::wstring>;
#endif
using ufmt = fmtT<mpt::ustring>;
#if defined(MPT_ENABLE_CHARSET_LOCALE)
using lfmt = fmtT<mpt::lstring>;
#endif // MPT_ENABLE_CHARSET_LOCALE
#if MPT_OS_WINDOWS
using tfmt = fmtT<mpt::tstring>;
#endif
#if defined(MPT_WITH_MFC)
using cfmt = fmtT<CString>;
#endif // MPT_WITH_MFC


#define MPT_AFORMAT(f) MPT_AFORMAT_MESSAGE(f)

#if !defined(MPT_COMPILER_QUIRK_NO_WCHAR)
#define MPT_WFORMAT(f) MPT_WFORMAT_MESSAGE(f)
#endif

#define MPT_UFORMAT(f) MPT_UFORMAT_MESSAGE(f)

#if defined(MPT_ENABLE_CHARSET_LOCALE)
#define MPT_LFORMAT(f) MPT_LFORMAT_MESSAGE(f)
#endif // MPT_ENABLE_CHARSET_LOCALE

#if MPT_OS_WINDOWS
#define MPT_TFORMAT(f) MPT_TFORMAT_MESSAGE(f)
#endif // MPT_OS_WINDOWS

#if defined(MPT_WITH_MFC)
#define MPT_CFORMAT(f) MPT_CFORMAT_MESSAGE(f)
#endif // MPT_WITH_MFC


} // namespace mpt



template <typename enum_t, typename store_t>
mpt::ustring ToUString(FlagSet<enum_t, store_t> flagset)
{
	mpt::ustring str(flagset.size_bits(), UC_('0'));

	for(std::size_t x = 0; x < flagset.size_bits(); ++x)
	{
		str[flagset.size_bits() - x - 1] = (flagset.value().as_bits() & (static_cast<typename FlagSet<enum_t>::store_type>(1) << x) ? UC_('1') : UC_('0'));
	}

	return str;
}



OPENMPT_NAMESPACE_END
