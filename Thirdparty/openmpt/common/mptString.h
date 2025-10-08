/*
 * mptString.h
 * ----------
 * Purpose: Small string-related utilities, number and message formatting.
 * Notes  : Currently none.
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#pragma once

#include "openmpt/all/BuildSettings.hpp"

#include "mpt/string/types.hpp"
#include "mpt/string/utility.hpp"
#include "mpt/string_transcode/transcode.hpp"

#include <algorithm>
#include <string>
#include <string_view>
#include <utility>

#include <cstddef>
#if defined(MODPLUG_TRACKER)
#include <cwctype>
#endif // MODPLUG_TRACKER



OPENMPT_NAMESPACE_BEGIN


namespace mpt
{



enum class CharsetEnum {

	UTF8,

	ASCII, // strictly 7-bit ASCII

	ISO8859_1,
	ISO8859_15,

	CP437,
	CP737,
	CP775,
	CP850,
	CP852,
	CP855,
	CP857,
	CP860,
	CP861,
	CP862,
	CP863,
	CP864,
	CP865,
	CP866,
	CP869,
	CP874,

	CP437AMS,
	CP437AMS2,

	Windows1252,

	Amiga,
	RISC_OS,
	AtariST,

	ISO8859_1_no_C1,
	ISO8859_15_no_C1,
	Amiga_no_C1,

#if defined(MPT_ENABLE_CHARSET_LOCALE)
	Locale, // CP_ACP on windows, current C locale otherwise
#endif // MPT_ENABLE_CHARSET_LOCALE

};

namespace CharsetTable
{

#define C(x) (mpt::char_value((x)))

// AMS1 actually only supports ASCII plus the modified control characters and no high chars at all.
// Just default to CP437 for those to keep things simple.
inline constexpr char32_t CP437AMS[256] = {
	C(' '),0x0001,0x0002,0x0003,0x00e4,0x0005,0x00e5,0x0007,0x0008,0x0009,0x000a,0x000b,0x000c,0x000d,0x00c4,0x00c5, // differs from CP437
	0x0010,0x0011,0x0012,0x0013,0x00f6,0x0015,0x0016,0x0017,0x0018,0x00d6,0x001a,0x001b,0x001c,0x001d,0x001e,0x001f, // differs from CP437
	0x0020,0x0021,0x0022,0x0023,0x0024,0x0025,0x0026,0x0027,0x0028,0x0029,0x002a,0x002b,0x002c,0x002d,0x002e,0x002f,
	0x0030,0x0031,0x0032,0x0033,0x0034,0x0035,0x0036,0x0037,0x0038,0x0039,0x003a,0x003b,0x003c,0x003d,0x003e,0x003f,
	0x0040,0x0041,0x0042,0x0043,0x0044,0x0045,0x0046,0x0047,0x0048,0x0049,0x004a,0x004b,0x004c,0x004d,0x004e,0x004f,
	0x0050,0x0051,0x0052,0x0053,0x0054,0x0055,0x0056,0x0057,0x0058,0x0059,0x005a,0x005b,0x005c,0x005d,0x005e,0x005f,
	0x0060,0x0061,0x0062,0x0063,0x0064,0x0065,0x0066,0x0067,0x0068,0x0069,0x006a,0x006b,0x006c,0x006d,0x006e,0x006f,
	0x0070,0x0071,0x0072,0x0073,0x0074,0x0075,0x0076,0x0077,0x0078,0x0079,0x007a,0x007b,0x007c,0x007d,0x007e,0x2302,
	0x00c7,0x00fc,0x00e9,0x00e2,0x00e4,0x00e0,0x00e5,0x00e7,0x00ea,0x00eb,0x00e8,0x00ef,0x00ee,0x00ec,0x00c4,0x00c5,
	0x00c9,0x00e6,0x00c6,0x00f4,0x00f6,0x00f2,0x00fb,0x00f9,0x00ff,0x00d6,0x00dc,0x00a2,0x00a3,0x00a5,0x20a7,0x0192,
	0x00e1,0x00ed,0x00f3,0x00fa,0x00f1,0x00d1,0x00aa,0x00ba,0x00bf,0x2310,0x00ac,0x00bd,0x00bc,0x00a1,0x00ab,0x00bb,
	0x2591,0x2592,0x2593,0x2502,0x2524,0x2561,0x2562,0x2556,0x2555,0x2563,0x2551,0x2557,0x255d,0x255c,0x255b,0x2510,
	0x2514,0x2534,0x252c,0x251c,0x2500,0x253c,0x255e,0x255f,0x255a,0x2554,0x2569,0x2566,0x2560,0x2550,0x256c,0x2567,
	0x2568,0x2564,0x2565,0x2559,0x2558,0x2552,0x2553,0x256b,0x256a,0x2518,0x250c,0x2588,0x2584,0x258c,0x2590,0x2580,
	0x03b1,0x00df,0x0393,0x03c0,0x03a3,0x03c3,0x00b5,0x03c4,0x03a6,0x0398,0x03a9,0x03b4,0x221e,0x03c6,0x03b5,0x2229,
	0x2261,0x00b1,0x2265,0x2264,0x2320,0x2321,0x00f7,0x2248,0x00b0,0x2219,0x00b7,0x221a,0x207f,0x00b2,0x25a0,0x00a0
};

// AMS2: Looking at Velvet Studio's bitmap font (TPIC32.PCX), these appear to be the only supported non-ASCII chars.
inline constexpr char32_t CP437AMS2[256] = {
	C(' '),0x00a9,0x221a,0x00b7,C('0'),C('1'),C('2'),C('3'),C('4'),C('5'),C('6'),C('7'),C('8'),C('9'),C('A'),C('B'), // differs from CP437
	C('C'),C('D'),C('E'),C('F'),C(' '),0x00a7,C(' '),C(' '),C(' '),C(' '),C(' '),C(' '),C(' '),C(' '),C(' '),C(' '), // differs from CP437
	0x0020,0x0021,0x0022,0x0023,0x0024,0x0025,0x0026,0x0027,0x0028,0x0029,0x002a,0x002b,0x002c,0x002d,0x002e,0x002f,
	0x0030,0x0031,0x0032,0x0033,0x0034,0x0035,0x0036,0x0037,0x0038,0x0039,0x003a,0x003b,0x003c,0x003d,0x003e,0x003f,
	0x0040,0x0041,0x0042,0x0043,0x0044,0x0045,0x0046,0x0047,0x0048,0x0049,0x004a,0x004b,0x004c,0x004d,0x004e,0x004f,
	0x0050,0x0051,0x0052,0x0053,0x0054,0x0055,0x0056,0x0057,0x0058,0x0059,0x005a,0x005b,0x005c,0x005d,0x005e,0x005f,
	0x0060,0x0061,0x0062,0x0063,0x0064,0x0065,0x0066,0x0067,0x0068,0x0069,0x006a,0x006b,0x006c,0x006d,0x006e,0x006f,
	0x0070,0x0071,0x0072,0x0073,0x0074,0x0075,0x0076,0x0077,0x0078,0x0079,0x007a,0x007b,0x007c,0x007d,0x007e,0x2302,
	0x00c7,0x00fc,0x00e9,0x00e2,0x00e4,0x00e0,0x00e5,0x00e7,0x00ea,0x00eb,0x00e8,0x00ef,0x00ee,0x00ec,0x00c4,0x00c5,
	0x00c9,0x00e6,0x00c6,0x00f4,0x00f6,0x00f2,0x00fb,0x00f9,0x00ff,0x00d6,0x00dc,0x00a2,0x00a3,0x00a5,0x20a7,0x0192,
	0x00e1,0x00ed,0x00f3,0x00fa,0x00f1,0x00d1,0x00aa,0x00ba,0x00bf,0x2310,0x00ac,0x00bd,0x00bc,0x00a1,0x00ab,0x00bb,
	0x2591,0x2592,0x2593,0x2502,0x2524,0x2561,0x2562,0x2556,0x2555,0x2563,0x2551,0x2557,0x255d,0x255c,0x255b,0x2510,
	0x2514,0x2534,0x252c,0x251c,0x2500,0x253c,0x255e,0x255f,0x255a,0x2554,0x2569,0x2566,0x2560,0x2550,0x256c,0x2567,
	0x2568,0x2564,0x2565,0x2559,0x2558,0x2552,0x2553,0x256b,0x256a,0x2518,0x250c,0x2588,0x2584,0x258c,0x2590,0x2580,
	0x03b1,0x00df,0x0393,0x03c0,0x03a3,0x03c3,0x00b5,0x03c4,0x03a6,0x0398,0x03a9,0x03b4,0x221e,0x03c6,0x03b5,0x2229,
	0x2261,0x00b1,0x2265,0x2264,0x2320,0x2321,0x00f7,0x2248,0x00b0,0x2219,0x00b7,0x221a,0x207f,0x00b2,0x25a0,0x00a0
};

#undef C

} // namespace CharsetTable



struct CharsetTranscoder
{

private:

	CharsetEnum m_Charset;

public:

	constexpr CharsetEnum GetCharset() const noexcept
	{
		return m_Charset;
	}

	constexpr CharsetTranscoder(CharsetEnum charset) noexcept
		: m_Charset(charset)
	{
		return;
	}

	constexpr operator CharsetEnum() const noexcept
	{
		return m_Charset;
	}

	// templated on 8bit strings because of type-safe variants
	template <typename Tdststring>
	inline Tdststring encode(const mpt::widestring &src) const
	{
		static_assert(sizeof(typename Tdststring::value_type) == sizeof(char));
		static_assert(mpt::is_character<typename Tdststring::value_type>::value);
		switch(m_Charset)
		{
		#if defined(MPT_ENABLE_CHARSET_LOCALE)
			case CharsetEnum::Locale:           return mpt::encode<Tdststring>(mpt::logical_encoding::locale, src); break;
		#endif
			case CharsetEnum::UTF8:             return mpt::encode<Tdststring>(mpt::common_encoding::utf8, src); break;
			case CharsetEnum::ASCII:            return mpt::encode<Tdststring>(mpt::common_encoding::ascii, src); break;
			case CharsetEnum::ISO8859_1:        return mpt::encode<Tdststring>(mpt::common_encoding::iso8859_1, src); break;
			case CharsetEnum::ISO8859_15:       return mpt::encode<Tdststring>(mpt::common_encoding::iso8859_15, src); break;
			case CharsetEnum::CP437:            return mpt::encode<Tdststring>(mpt::common_encoding::cp437, src); break;
			case CharsetEnum::CP737:            return mpt::encode<Tdststring>(mpt::common_encoding::cp737, src); break;
			case CharsetEnum::CP775:            return mpt::encode<Tdststring>(mpt::common_encoding::cp775, src); break;
			case CharsetEnum::CP850:            return mpt::encode<Tdststring>(mpt::common_encoding::cp850, src); break;
			case CharsetEnum::CP852:            return mpt::encode<Tdststring>(mpt::common_encoding::cp852, src); break;
			case CharsetEnum::CP855:            return mpt::encode<Tdststring>(mpt::common_encoding::cp855, src); break;
			case CharsetEnum::CP857:            return mpt::encode<Tdststring>(mpt::common_encoding::cp857, src); break;
			case CharsetEnum::CP860:            return mpt::encode<Tdststring>(mpt::common_encoding::cp860, src); break;
			case CharsetEnum::CP861:            return mpt::encode<Tdststring>(mpt::common_encoding::cp861, src); break;
			case CharsetEnum::CP862:            return mpt::encode<Tdststring>(mpt::common_encoding::cp862, src); break;
			case CharsetEnum::CP863:            return mpt::encode<Tdststring>(mpt::common_encoding::cp863, src); break;
			case CharsetEnum::CP864:            return mpt::encode<Tdststring>(mpt::common_encoding::cp864, src); break;
			case CharsetEnum::CP865:            return mpt::encode<Tdststring>(mpt::common_encoding::cp865, src); break;
			case CharsetEnum::CP866:            return mpt::encode<Tdststring>(mpt::common_encoding::cp866, src); break;
			case CharsetEnum::CP869:            return mpt::encode<Tdststring>(mpt::common_encoding::cp869, src); break;
			case CharsetEnum::CP874:            return mpt::encode<Tdststring>(mpt::common_encoding::cp874, src); break;
			case CharsetEnum::CP437AMS:         return mpt::encode<Tdststring>(CharsetTable::CP437AMS, src); break;
			case CharsetEnum::CP437AMS2:        return mpt::encode<Tdststring>(CharsetTable::CP437AMS2, src); break;
			case CharsetEnum::Windows1252:      return mpt::encode<Tdststring>(mpt::common_encoding::windows1252, src); break;
			case CharsetEnum::Amiga:            return mpt::encode<Tdststring>(mpt::common_encoding::amiga, src); break;
			case CharsetEnum::RISC_OS:          return mpt::encode<Tdststring>(mpt::common_encoding::riscos, src); break;
			case CharsetEnum::AtariST:          return mpt::encode<Tdststring>(mpt::common_encoding::atarist, src); break;
			case CharsetEnum::ISO8859_1_no_C1:  return mpt::encode<Tdststring>(mpt::common_encoding::iso8859_1_no_c1, src); break;
			case CharsetEnum::ISO8859_15_no_C1: return mpt::encode<Tdststring>(mpt::common_encoding::iso8859_15_no_c1, src); break;
			case CharsetEnum::Amiga_no_C1:      return mpt::encode<Tdststring>(mpt::common_encoding::amiga_no_c1, src); break;
		}
		return Tdststring();
	}

	// templated on 8bit strings because of type-safe variants
	template <typename Tsrcstring>
	inline mpt::widestring decode(const Tsrcstring &src) const
	{
		static_assert(sizeof(typename Tsrcstring::value_type) == sizeof(char));
		static_assert(mpt::is_character<typename Tsrcstring::value_type>::value);
		switch(m_Charset)
		{
		#if defined(MPT_ENABLE_CHARSET_LOCALE)
			case CharsetEnum::Locale:           return mpt::decode<Tsrcstring>(mpt::logical_encoding::locale, src); break;
		#endif
			case CharsetEnum::UTF8:             return mpt::decode<Tsrcstring>(mpt::common_encoding::utf8, src); break;
			case CharsetEnum::ASCII:            return mpt::decode<Tsrcstring>(mpt::common_encoding::ascii, src); break;
			case CharsetEnum::ISO8859_1:        return mpt::decode<Tsrcstring>(mpt::common_encoding::iso8859_1, src); break;
			case CharsetEnum::ISO8859_15:       return mpt::decode<Tsrcstring>(mpt::common_encoding::iso8859_15, src); break;
			case CharsetEnum::CP437:            return mpt::decode<Tsrcstring>(mpt::common_encoding::cp437, src); break;
			case CharsetEnum::CP737:            return mpt::decode<Tsrcstring>(mpt::common_encoding::cp737, src); break;
			case CharsetEnum::CP775:            return mpt::decode<Tsrcstring>(mpt::common_encoding::cp775, src); break;
			case CharsetEnum::CP850:            return mpt::decode<Tsrcstring>(mpt::common_encoding::cp850, src); break;
			case CharsetEnum::CP852:            return mpt::decode<Tsrcstring>(mpt::common_encoding::cp852, src); break;
			case CharsetEnum::CP855:            return mpt::decode<Tsrcstring>(mpt::common_encoding::cp855, src); break;
			case CharsetEnum::CP857:            return mpt::decode<Tsrcstring>(mpt::common_encoding::cp857, src); break;
			case CharsetEnum::CP860:            return mpt::decode<Tsrcstring>(mpt::common_encoding::cp860, src); break;
			case CharsetEnum::CP861:            return mpt::decode<Tsrcstring>(mpt::common_encoding::cp861, src); break;
			case CharsetEnum::CP862:            return mpt::decode<Tsrcstring>(mpt::common_encoding::cp862, src); break;
			case CharsetEnum::CP863:            return mpt::decode<Tsrcstring>(mpt::common_encoding::cp863, src); break;
			case CharsetEnum::CP864:            return mpt::decode<Tsrcstring>(mpt::common_encoding::cp864, src); break;
			case CharsetEnum::CP865:            return mpt::decode<Tsrcstring>(mpt::common_encoding::cp865, src); break;
			case CharsetEnum::CP866:            return mpt::decode<Tsrcstring>(mpt::common_encoding::cp866, src); break;
			case CharsetEnum::CP869:            return mpt::decode<Tsrcstring>(mpt::common_encoding::cp869, src); break;
			case CharsetEnum::CP874:            return mpt::decode<Tsrcstring>(mpt::common_encoding::cp874, src); break;
			case CharsetEnum::CP437AMS:         return mpt::decode<Tsrcstring>(CharsetTable::CP437AMS, src); break;
			case CharsetEnum::CP437AMS2:        return mpt::decode<Tsrcstring>(CharsetTable::CP437AMS2, src); break;
			case CharsetEnum::Windows1252:      return mpt::decode<Tsrcstring>(mpt::common_encoding::windows1252, src); break;
			case CharsetEnum::Amiga:            return mpt::decode<Tsrcstring>(mpt::common_encoding::amiga, src); break;
			case CharsetEnum::RISC_OS:          return mpt::decode<Tsrcstring>(mpt::common_encoding::riscos, src); break;
			case CharsetEnum::AtariST:          return mpt::decode<Tsrcstring>(mpt::common_encoding::atarist, src); break;
			case CharsetEnum::ISO8859_1_no_C1:  return mpt::decode<Tsrcstring>(mpt::common_encoding::iso8859_1_no_c1, src); break;
			case CharsetEnum::ISO8859_15_no_C1: return mpt::decode<Tsrcstring>(mpt::common_encoding::iso8859_15_no_c1, src); break;
			case CharsetEnum::Amiga_no_C1:      return mpt::decode<Tsrcstring>(mpt::common_encoding::amiga_no_c1, src); break;
		}
		return mpt::widestring();
	}

};



struct Charset
	: public CharsetTranscoder
{

	constexpr Charset(mpt::CharsetEnum charset) noexcept
		: CharsetTranscoder(charset)
	{
		return;
	}

	constexpr Charset(mpt::CharsetTranscoder charset) noexcept
		: CharsetTranscoder(charset.GetCharset())
	{
		return;
	}

	static inline constexpr auto UTF8 = mpt::CharsetTranscoder{ mpt::CharsetEnum::UTF8 };
	static inline constexpr auto ASCII = mpt::CharsetTranscoder{ mpt::CharsetEnum::ASCII };
	static inline constexpr auto ISO8859_1 = mpt::CharsetTranscoder{ mpt::CharsetEnum::ISO8859_1 };
	static inline constexpr auto ISO8859_15 = mpt::CharsetTranscoder{ mpt::CharsetEnum::ISO8859_15 };
	static inline constexpr auto CP437 = mpt::CharsetTranscoder{ mpt::CharsetEnum::CP437 };
	static inline constexpr auto CP737 = mpt::CharsetTranscoder{ mpt::CharsetEnum::CP737 };
	static inline constexpr auto CP775 = mpt::CharsetTranscoder{ mpt::CharsetEnum::CP775 };
	static inline constexpr auto CP850 = mpt::CharsetTranscoder{ mpt::CharsetEnum::CP850 };
	static inline constexpr auto CP852 = mpt::CharsetTranscoder{ mpt::CharsetEnum::CP852 };
	static inline constexpr auto CP855 = mpt::CharsetTranscoder{ mpt::CharsetEnum::CP855 };
	static inline constexpr auto CP857 = mpt::CharsetTranscoder{ mpt::CharsetEnum::CP857 };
	static inline constexpr auto CP860 = mpt::CharsetTranscoder{ mpt::CharsetEnum::CP860 };
	static inline constexpr auto CP861 = mpt::CharsetTranscoder{ mpt::CharsetEnum::CP861 };
	static inline constexpr auto CP862 = mpt::CharsetTranscoder{ mpt::CharsetEnum::CP862 };
	static inline constexpr auto CP863 = mpt::CharsetTranscoder{ mpt::CharsetEnum::CP863 };
	static inline constexpr auto CP864 = mpt::CharsetTranscoder{ mpt::CharsetEnum::CP864 };
	static inline constexpr auto CP865 = mpt::CharsetTranscoder{ mpt::CharsetEnum::CP865 };
	static inline constexpr auto CP866 = mpt::CharsetTranscoder{ mpt::CharsetEnum::CP866 };
	static inline constexpr auto CP869 = mpt::CharsetTranscoder{ mpt::CharsetEnum::CP869 };
	static inline constexpr auto CP874 = mpt::CharsetTranscoder{ mpt::CharsetEnum::CP874 };
	static inline constexpr auto CP437AMS = mpt::CharsetTranscoder{ mpt::CharsetEnum::CP437AMS };
	static inline constexpr auto CP437AMS2 = mpt::CharsetTranscoder{ mpt::CharsetEnum::CP437AMS2 };
	static inline constexpr auto Windows1252 = mpt::CharsetTranscoder{ mpt::CharsetEnum::Windows1252 };
	static inline constexpr auto Amiga = mpt::CharsetTranscoder{ mpt::CharsetEnum::Amiga };
	static inline constexpr auto RISC_OS = mpt::CharsetTranscoder{ mpt::CharsetEnum::RISC_OS };
	static inline constexpr auto AtariST = mpt::CharsetTranscoder{ mpt::CharsetEnum::AtariST };
	static inline constexpr auto ISO8859_1_no_C1 = mpt::CharsetTranscoder{ mpt::CharsetEnum::ISO8859_1_no_C1 };
	static inline constexpr auto ISO8859_15_no_C1 = mpt::CharsetTranscoder{ mpt::CharsetEnum::ISO8859_15_no_C1 };
	static inline constexpr auto Amiga_no_C1 = mpt::CharsetTranscoder{ mpt::CharsetEnum::Amiga_no_C1 };

#if defined(MPT_ENABLE_CHARSET_LOCALE)
	static inline constexpr auto Locale = mpt::CharsetTranscoder{ mpt::CharsetEnum::Locale };
#endif // MPT_ENABLE_CHARSET_LOCALE

};





// Checks if the std::string represents an UTF8 string.
// This is currently implemented as converting to std::wstring and back assuming UTF8 both ways,
// and comparing the result to the original string.
// Caveats:
//  - can give false negatives because of possible unicode normalization during conversion
//  - can give false positives if the 8bit encoding contains high-ascii only in valid utf8 groups
//  - slow because of double conversion
inline bool IsUTF8(const std::string &str)
{
	return mpt::is_utf8(str);
}



template <typename Tsrc>
inline mpt::ustring ToUnicode(Tsrc &&str)
{
	return mpt::transcode<mpt::ustring>(std::forward<Tsrc>(str));
}
template <typename Tsrc, typename Tencoding>
inline mpt::ustring ToUnicode(Tencoding &&from, Tsrc &&str)
{
	return mpt::transcode<mpt::ustring>(std::forward<Tencoding>(from), std::forward<Tsrc>(str));
}

#if !defined(MPT_COMPILER_QUIRK_NO_WCHAR)
// Convert to a wide character string.
// The wide encoding is UTF-16 or UTF-32, based on sizeof(wchar_t).
// If str does not contain any invalid characters, this conversion is lossless.
// Invalid source bytes will be replaced by some replacement character or string.
template <typename Tsrc>
inline std::wstring ToWide(Tsrc &&str)
{
	return mpt::transcode<std::wstring>(std::forward<Tsrc>(str));
}
template <typename Tsrc, typename Tencoding>
inline std::wstring ToWide(Tencoding &&from, Tsrc &&str)
{
	return mpt::transcode<std::wstring>(std::forward<Tencoding>(from), std::forward<Tsrc>(str));
}
#endif

// Convert to a string encoded in the 'to'-specified character set.
// If str does not contain any invalid characters,
// this conversion will be lossless iff, and only iff,
// 'to' is UTF8.
// Invalid source bytes or characters that are not representable in the
// destination charset will be replaced by some replacement character or string.
template <typename Tsrc, typename Tencoding>
inline std::string ToCharset(Tencoding &&to, Tsrc &&str)
{
	return mpt::transcode<std::string>(std::forward<Tencoding>(to), std::forward<Tsrc>(str));
}
template <typename Tsrc, typename Tto, typename Tfrom>
inline std::string ToCharset(Tto &&to, Tfrom &&from, Tsrc &&str)
{
	return mpt::transcode<std::string>(std::forward<Tto>(to), std::forward<Tfrom>(from), std::forward<Tsrc>(str));
}

#if defined(MPT_ENABLE_CHARSET_LOCALE)
template <typename Tsrc>
inline mpt::lstring ToLocale(Tsrc &&str)
{
	return mpt::transcode<mpt::lstring>(std::forward<Tsrc>(str));
}
template <typename Tsrc, typename Tencoding>
inline mpt::lstring ToLocale(Tencoding &&from, Tsrc &&str)
{
	return mpt::transcode<mpt::lstring>(std::forward<Tencoding>(from), std::forward<Tsrc>(str));
}
#endif // MPT_ENABLE_CHARSET_LOCALE

#if MPT_OS_WINDOWS
template <typename Tsrc>
inline mpt::winstring ToWin(Tsrc &&str)
{
	return mpt::transcode<mpt::winstring>(std::forward<Tsrc>(str));
}
template <typename Tsrc, typename Tencoding>
inline mpt::winstring ToWin(Tencoding &&from, Tsrc &&str)
{
	return mpt::transcode<mpt::winstring>(std::forward<Tencoding>(from), std::forward<Tsrc>(str));
}
#endif // MPT_OS_WINDOWS

#if defined(MPT_WITH_MFC)
template <typename Tsrc>
inline CString ToCString(Tsrc &&str)
{
	return mpt::transcode<CString>(std::forward<Tsrc>(str));
}
template <typename Tsrc, typename Tencoding>
inline CString ToCString(Tencoding &&from, Tsrc &&str)
{
	return mpt::transcode<CString>(std::forward<Tencoding>(from), std::forward<Tsrc>(str));
}
#endif // MPT_WITH_MFC



#define UC_(x)           MPT_UCHAR(x)
#define UL_(x)           MPT_ULITERAL(x)
#define UV_(x)           MPT_USTRINGVIEW(x)
#define U_(x)            MPT_USTRING(x)



// The MPT_UTF8 allows specifying UTF8 char arrays.
// The resulting type is mpt::ustring and the construction might require runtime translation,
// i.e. it is NOT generally available at compile time.
// Use explicit UTF8 encoding,
// i.e. U+00FC (LATIN SMALL LETTER U WITH DIAERESIS) would be written as "\xC3\xBC".
#define MPT_UTF8(x) mpt::transcode<mpt::ustring>(mpt::common_encoding::utf8, x)



template <typename Tsrc, typename Tencoding>
inline mpt::ustring ToUnicode(uint16 codepage, Tencoding &&fallback, Tsrc &&str)
{
	#if MPT_OS_WINDOWS && !defined(MPT_COMPILER_QUIRK_NO_WCHAR)
		mpt::ustring result;
		std::optional<mpt::common_encoding> charset = mpt::optional_encoding_from_codepage(codepage);
		if(charset.has_value())
		{
#if defined(MPT_LIBCXX_QUIRK_NO_OPTIONAL_VALUE)
			result = mpt::transcode<mpt::ustring>(*charset, std::forward<Tsrc>(str));
#else
			result = mpt::transcode<mpt::ustring>(charset.value(), std::forward<Tsrc>(str));
#endif
		} else if(mpt::has_codepage(static_cast<UINT>(codepage)))
		{
			result = mpt::transcode<mpt::ustring>(static_cast<UINT>(codepage), std::forward<Tsrc>(str));
		} else
		{
			result = mpt::transcode<mpt::ustring>(std::forward<Tencoding>(fallback), std::forward<Tsrc>(str));
		}
		return result;
	#else // !MPT_OS_WINDOWS
		std::optional<mpt::common_encoding> charset = mpt::optional_encoding_from_codepage(codepage);
#if defined(MPT_LIBCXX_QUIRK_NO_OPTIONAL_VALUE)
		return charset.has_value() ? mpt::transcode<mpt::ustring>(charset.value(), std::forward<Tsrc>(str)) : mpt::transcode<mpt::ustring>(std::forward<Tencoding>(fallback), std::forward<Tsrc>(str));
#else
		return charset.has_value() ? mpt::transcode<mpt::ustring>(*charset, std::forward<Tsrc>(str)) : mpt::transcode<mpt::ustring>(std::forward<Tencoding>(fallback), std::forward<Tsrc>(str));
#endif
	#endif // MPT_OS_WINDOWS
}





inline char ToLowerCaseAscii(char c)
{
	return mpt::to_lower_ascii(c);
}

inline char ToUpperCaseAscii(char c)
{
	return mpt::to_upper_ascii(c);
}

inline std::string ToLowerCaseAscii(std::string s)
{
	std::transform(s.begin(), s.end(), s.begin(), static_cast<char(*)(char)>(&mpt::ToLowerCaseAscii));
	return s;
}

inline std::string ToUpperCaseAscii(std::string s)
{
	std::transform(s.begin(), s.end(), s.begin(), static_cast<char(*)(char)>(&mpt::ToUpperCaseAscii));
	return s;
}

inline int CompareNoCaseAscii(const char *a, const char *b, std::size_t n)
{
	while(n--)
	{
		unsigned char ac = mpt::char_value(mpt::ToLowerCaseAscii(*a));
		unsigned char bc = mpt::char_value(mpt::ToLowerCaseAscii(*b));
		if(ac != bc)
		{
			return ac < bc ? -1 : 1;
		} else if(!ac && !bc)
		{
			return 0;
		}
		++a;
		++b;
	}
	return 0;
}

inline int CompareNoCaseAscii(std::string_view a, std::string_view b)
{
	for(std::size_t i = 0; i < std::min(a.length(), b.length()); ++i)
	{
		unsigned char ac = mpt::char_value(mpt::ToLowerCaseAscii(a[i]));
		unsigned char bc = mpt::char_value(mpt::ToLowerCaseAscii(b[i]));
		if(ac != bc)
		{
			return ac < bc ? -1 : 1;
		} else if(!ac && !bc)
		{
			return 0;
		}
	}
	if(a.length() == b.length())
	{
		return 0;
	}
	return a.length() < b.length() ? -1 : 1;
}

inline int CompareNoCaseAscii(const std::string &a, const std::string &b)
{
	return CompareNoCaseAscii(std::string_view(a), std::string_view(b));
}


#if defined(MODPLUG_TRACKER)

inline mpt::ustring ToLowerCase(const mpt::ustring &s)
{
	#if defined(MPT_WITH_MFC)
		#if defined(UNICODE)
			return mpt::transcode<mpt::ustring>(mpt::transcode<CString>(s).MakeLower());
		#else // !UNICODE
			return mpt::transcode<mpt::ustring>(mpt::transcode<CStringW>(s).MakeLower());
		#endif // UNICODE
	#else // !MPT_WITH_MFC
		std::wstring ws = mpt::transcode<std::wstring>(s);
		std::transform(ws.begin(), ws.end(), ws.begin(), &std::towlower);
		return mpt::transcode<mpt::ustring>(ws);
	#endif // MPT_WITH_MFC
}

inline mpt::ustring ToUpperCase(const mpt::ustring &s)
{
	#if defined(MPT_WITH_MFC)
		#if defined(UNICODE)
			return mpt::transcode<mpt::ustring>(mpt::transcode<CString>(s).MakeUpper());
		#else // !UNICODE
			return mpt::transcode<mpt::ustring>(mpt::transcode<CStringW>(s).MakeUpper());
		#endif // UNICODE
	#else // !MPT_WITH_MFC
		std::wstring ws = mpt::transcode<std::wstring>(s);
		std::transform(ws.begin(), ws.end(), ws.begin(), &std::towupper);
		return mpt::transcode<mpt::ustring>(ws);
	#endif // MPT_WITH_MFC
}

#endif // MODPLUG_TRACKER





} // namespace mpt





// The AnyString types are meant to be used as function argument types only,
// and only during the transition phase to all-unicode strings in the whole codebase.
// Using an AnyString type as function argument avoids the need to overload a function for all the
// different string types that we currently have.
// Warning: These types will silently do charset conversions. Only use them when this can be tolerated.

// BasicAnyString is convertable to mpt::ustring and constructable from any string at all.
template <mpt::CharsetEnum charset = mpt::Charset::UTF8, bool tryUTF8 = true>
class BasicAnyString : public mpt::ustring
{

private:
	
	static mpt::ustring From8bit(std::string str)
	{
		if constexpr(charset == mpt::CharsetEnum::UTF8)
		{
			return mpt::transcode<mpt::ustring>(mpt::common_encoding::utf8, std::move(str));
		} else
		{
			// auto utf8 detection
			if constexpr(tryUTF8)
			{
				if(mpt::is_utf8(str))
				{
					return mpt::transcode<mpt::ustring>(mpt::common_encoding::utf8, std::move(str));
				} else
				{
					return mpt::transcode<mpt::ustring>(mpt::CharsetTranscoder(charset), std::move(str));
				}
			} else
			{
				return mpt::transcode<mpt::ustring>(mpt::CharsetTranscoder(charset), std::move(str));
			}
		}
	}

public:

	// 8 bit
	BasicAnyString(const char *str)
		: mpt::ustring(From8bit(str ? str : std::string()))
	{
		return;
	}
	BasicAnyString(std::string str)
		: mpt::ustring(From8bit(std::move(str)))
	{
		return;
	}

	template <typename Tstring>
	BasicAnyString(Tstring &&str)
		: mpt::ustring(mpt::transcode<mpt::ustring>(std::forward<Tstring>(str)))
	{
		return;
	}

};

// AnyUnicodeString is convertable to mpt::ustring and constructable from any known encoding
class AnyUnicodeString : public mpt::ustring
{

public:

	template <typename Tstring>
	AnyUnicodeString(Tstring &&str)
		: mpt::ustring(mpt::transcode<mpt::ustring>(std::forward<Tstring>(str)))
	{
		return;
	}

};

// AnyString
// Try to do the smartest auto-magic we can do.
#if defined(MPT_ENABLE_CHARSET_LOCALE)
using AnyString = BasicAnyString<mpt::CharsetEnum::Locale, true>;
#elif MPT_OS_WINDOWS
using AnyString = BasicAnyString<mpt::CharsetEnum::Windows1252, true>;
#else
using AnyString = BasicAnyString<mpt::CharsetEnum::ISO8859_1, true>;
#endif

// AnyStringLocale
// char-based strings are assumed to be in locale encoding.
#if defined(MPT_ENABLE_CHARSET_LOCALE)
using AnyStringLocale = BasicAnyString<mpt::CharsetEnum::Locale, false>;
#else
using AnyStringLocale = BasicAnyString<mpt::CharsetEnum::UTF8, false>;
#endif

// AnyStringUTF8orLocale
// char-based strings are tried in UTF8 first, if this fails, locale is used.
#if defined(MPT_ENABLE_CHARSET_LOCALE)
using AnyStringUTF8orLocale = BasicAnyString<mpt::CharsetEnum::Locale, true>;
#else
using AnyStringUTF8orLocale = BasicAnyString<mpt::CharsetEnum::UTF8, false>;
#endif

// AnyStringUTF8
// char-based strings are assumed to be in UTF8.
using AnyStringUTF8 = BasicAnyString<mpt::CharsetEnum::UTF8, false>;



OPENMPT_NAMESPACE_END



template <OPENMPT_NAMESPACE::mpt::CharsetEnum charset, bool tryUTF8>
struct mpt::make_string_type<OPENMPT_NAMESPACE::BasicAnyString<charset, tryUTF8>> {
	using type = mpt::ustring;
};

template <>
struct mpt::make_string_type<OPENMPT_NAMESPACE::AnyUnicodeString> {
	using type = mpt::ustring;
};
