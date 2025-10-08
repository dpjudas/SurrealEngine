/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_FORMAT_TESTS_FORMAT_SIMPLE_HPP
#define MPT_FORMAT_TESTS_FORMAT_SIMPLE_HPP



#include "mpt/base/detect.hpp"
#include "mpt/base/namespace.hpp"
#include "mpt/detect/mfc.hpp"
#include "mpt/format/simple.hpp"
#include "mpt/format/simple_integer.hpp"
#include "mpt/string/types.hpp"
#include "mpt/test/test.hpp"
#include "mpt/test/test_macros.hpp"

#include <limits>
#include <string>
#include <string_view>



namespace mpt {
inline namespace MPT_INLINE_NS {



namespace tests {
namespace format {
namespace simple {

#if MPT_COMPILER_CLANG
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wglobal-constructors"
#endif
MPT_TEST_GROUP_INLINE("mpt/format/simple")
#if MPT_COMPILER_CLANG
#pragma clang diagnostic pop
#endif
{
	MPT_TEST_EXPECT_EQUAL(mpt::format<std::string>::val(1.5f), "1.5");
	MPT_TEST_EXPECT_EQUAL(mpt::format<std::string>::val(true), "1");
	MPT_TEST_EXPECT_EQUAL(mpt::format<std::string>::val(false), "0");

	MPT_TEST_EXPECT_EQUAL(mpt::format<std::string>::val(0), "0");
	MPT_TEST_EXPECT_EQUAL(mpt::format<std::string>::val(-23), "-23");
	MPT_TEST_EXPECT_EQUAL(mpt::format<std::string>::val(42), "42");

	MPT_TEST_EXPECT_EQUAL(mpt::format<std::string>::hex0<3>((int32)-1), "-001");
	MPT_TEST_EXPECT_EQUAL(mpt::format<std::string>::hex((int32)-1), "-1");
	MPT_TEST_EXPECT_EQUAL(mpt::format<std::string>::hex(-0xabcde), "-abcde");
	MPT_TEST_EXPECT_EQUAL(mpt::format<std::string>::hex(std::numeric_limits<int32>::min()), "-80000000");
	MPT_TEST_EXPECT_EQUAL(mpt::format<std::string>::hex(std::numeric_limits<int32>::min() + 1), "-7fffffff");
	MPT_TEST_EXPECT_EQUAL(mpt::format<std::string>::hex(0x123e), "123e");
	MPT_TEST_EXPECT_EQUAL(mpt::format<std::string>::hex0<6>(0x123e), "00123e");
	MPT_TEST_EXPECT_EQUAL(mpt::format<std::string>::hex0<2>(0x123e), "123e");

	MPT_TEST_EXPECT_EQUAL(mpt::format<std::string>::dec0<0>(1), "1");
	MPT_TEST_EXPECT_EQUAL(mpt::format<std::string>::dec0<1>(1), "1");
	MPT_TEST_EXPECT_EQUAL(mpt::format<std::string>::dec0<2>(1), "01");
	MPT_TEST_EXPECT_EQUAL(mpt::format<std::string>::dec0<3>(1), "001");
	MPT_TEST_EXPECT_EQUAL(mpt::format<std::string>::dec0<0>(11), "11");
	MPT_TEST_EXPECT_EQUAL(mpt::format<std::string>::dec0<1>(11), "11");
	MPT_TEST_EXPECT_EQUAL(mpt::format<std::string>::dec0<2>(11), "11");
	MPT_TEST_EXPECT_EQUAL(mpt::format<std::string>::dec0<3>(11), "011");
	MPT_TEST_EXPECT_EQUAL(mpt::format<std::string>::dec0<0>(-1), "-1");
	MPT_TEST_EXPECT_EQUAL(mpt::format<std::string>::dec0<1>(-1), "-1");
	MPT_TEST_EXPECT_EQUAL(mpt::format<std::string>::dec0<2>(-1), "-01");
	MPT_TEST_EXPECT_EQUAL(mpt::format<std::string>::dec0<3>(-1), "-001");

	MPT_TEST_EXPECT_EQUAL(mpt::format<mpt::ustring>::HEX0<7>(0xa2345678), MPT_USTRING("A2345678"));
	MPT_TEST_EXPECT_EQUAL(mpt::format<mpt::ustring>::HEX0<8>(0xa2345678), MPT_USTRING("A2345678"));
	MPT_TEST_EXPECT_EQUAL(mpt::format<mpt::ustring>::HEX0<9>(0xa2345678), MPT_USTRING("0A2345678"));
	MPT_TEST_EXPECT_EQUAL(mpt::format<mpt::ustring>::HEX0<10>(0xa2345678), MPT_USTRING("00A2345678"));

#if MPT_FORMAT_FORMAT_SIMPLE_INT_CXX17

	MPT_TEST_EXPECT_EQUAL(mpt::format_simple_integer_to_chars<std::string>(std::numeric_limits<int16>::min(), 10), "-32768");
	MPT_TEST_EXPECT_EQUAL(mpt::format_simple_integer_to_chars<std::string>(std::numeric_limits<int16>::max(), 10), "32767");

	MPT_TEST_EXPECT_EQUAL(mpt::format_simple_integer_to_chars<std::string>(std::numeric_limits<int16>::min(), 7), "-164351");
	MPT_TEST_EXPECT_EQUAL(mpt::format_simple_integer_to_chars<std::string>(std::numeric_limits<int16>::min() + 1, 7), "-164350");
	MPT_TEST_EXPECT_EQUAL(mpt::format_simple_integer_to_chars<std::string>(std::numeric_limits<int16>::max(), 7), "164350");

#else // !MPT_FORMAT_FORMAT_SIMPLE_INT_CXX17

	MPT_TEST_EXPECT_EQUAL(mpt::format_simple_integer_to_stream<std::string>(std::numeric_limits<int16>::min(), 10), "-32768");
	MPT_TEST_EXPECT_EQUAL(mpt::format_simple_integer_to_stream<std::string>(std::numeric_limits<int16>::max(), 10), "32767");

	MPT_TEST_EXPECT_EQUAL(mpt::format_simple_integer_to_stream<std::string>(std::numeric_limits<int16>::min(), 7), "-164351");
	MPT_TEST_EXPECT_EQUAL(mpt::format_simple_integer_to_stream<std::string>(std::numeric_limits<int16>::min() + 1, 7), "-164350");
	MPT_TEST_EXPECT_EQUAL(mpt::format_simple_integer_to_stream<std::string>(std::numeric_limits<int16>::max(), 7), "164350");

#endif // MPT_FORMAT_FORMAT_SIMPLE_INT_CXX17

#if !defined(MPT_COMPILER_QUIRK_NO_WCHAR)
	MPT_TEST_EXPECT_EQUAL(mpt::format<std::wstring>::hex(0x123e), L"123e");
	MPT_TEST_EXPECT_EQUAL(mpt::format<std::wstring>::hex0<6>(0x123e), L"00123e");
	MPT_TEST_EXPECT_EQUAL(mpt::format<std::wstring>::hex0<2>(0x123e), L"123e");
#endif // !MPT_COMPILER_QUIRK_NO_WCHAR

	MPT_TEST_EXPECT_EQUAL(mpt::format<std::string>::val(-87.0f), "-87");
	if (mpt::format<std::string>::val(-0.5e-6) != "-5e-007"
		&& mpt::format<std::string>::val(-0.5e-6) != "-5e-07"
		&& mpt::format<std::string>::val(-0.5e-6) != "-5e-7"
		&& mpt::format<std::string>::val(-0.5e-6) != "-4.9999999999999998e-7"
		&& mpt::format<std::string>::val(-0.5e-6) != "-4.9999999999999998e-07"
		&& mpt::format<std::string>::val(-0.5e-6) != "-4.9999999999999998e-007")
	{
		MPT_TEST_EXPECT_EQUAL(true, false);
	}
	if (mpt::format<std::string>::val(-1.0 / 65536.0) != "-1.52587890625e-005"
		&& mpt::format<std::string>::val(-1.0 / 65536.0) != "-1.52587890625e-05"
		&& mpt::format<std::string>::val(-1.0 / 65536.0) != "-1.52587890625e-5")
	{
		MPT_TEST_EXPECT_EQUAL(true, false);
	}
	if (mpt::format<std::string>::val(-1.0f / 65536.0f) != "-1.52587891e-005"
		&& mpt::format<std::string>::val(-1.0f / 65536.0f) != "-1.52587891e-05"
		&& mpt::format<std::string>::val(-1.0f / 65536.0f) != "-1.52587891e-5"
		&& mpt::format<std::string>::val(-1.0f / 65536.0f) != "-1.5258789e-005"
		&& mpt::format<std::string>::val(-1.0f / 65536.0f) != "-1.5258789e-05"
		&& mpt::format<std::string>::val(-1.0f / 65536.0f) != "-1.5258789e-5")
	{
		MPT_TEST_EXPECT_EQUAL(true, false);
	}
	if (mpt::format<std::string>::val(58.65403492763) != "58.654034927630001"
		&& mpt::format<std::string>::val(58.65403492763) != "58.65403492763")
	{
		MPT_TEST_EXPECT_EQUAL(true, false);
	}
	MPT_TEST_EXPECT_EQUAL(mpt::format<std::string>::flt(58.65403492763, 6), "58.654");
	MPT_TEST_EXPECT_EQUAL(mpt::format<std::string>::fix(23.42, 1), "23.4");
	MPT_TEST_EXPECT_EQUAL(mpt::format<std::string>::fix(234.2, 1), "234.2");
	MPT_TEST_EXPECT_EQUAL(mpt::format<std::string>::fix(2342.0, 1), "2342.0");

	MPT_TEST_EXPECT_EQUAL(mpt::format<std::string>::dec(2, ";", 2345678), std::string("2;34;56;78"));
	MPT_TEST_EXPECT_EQUAL(mpt::format<std::string>::dec(2, ";", 12345678), std::string("12;34;56;78"));
	MPT_TEST_EXPECT_EQUAL(mpt::format<std::string>::hex(3, ":", 0xa2345678), std::string("a2:345:678"));

	MPT_TEST_EXPECT_EQUAL(mpt::format<mpt::ustring>::dec(2, MPT_USTRING(";"), 12345678), MPT_USTRING("12;34;56;78"));
	MPT_TEST_EXPECT_EQUAL(mpt::format<mpt::ustring>::hex(3, MPT_USTRING(":"), 0xa2345678), MPT_USTRING("a2:345:678"));

	MPT_TEST_EXPECT_EQUAL(mpt::format<mpt::ustring>::HEX0<7>(3, MPT_USTRING(":"), 0xa2345678), MPT_USTRING("A2:345:678"));
	MPT_TEST_EXPECT_EQUAL(mpt::format<mpt::ustring>::HEX0<8>(3, MPT_USTRING(":"), 0xa2345678), MPT_USTRING("A2:345:678"));
	MPT_TEST_EXPECT_EQUAL(mpt::format<mpt::ustring>::HEX0<9>(3, MPT_USTRING(":"), 0xa2345678), MPT_USTRING("0A2:345:678"));
	MPT_TEST_EXPECT_EQUAL(mpt::format<mpt::ustring>::HEX0<10>(3, MPT_USTRING(":"), 0xa2345678), MPT_USTRING("0:0A2:345:678"));
	MPT_TEST_EXPECT_EQUAL(mpt::format<mpt::ustring>::HEX0<11>(3, MPT_USTRING(":"), 0xa2345678), MPT_USTRING("00:0A2:345:678"));
	MPT_TEST_EXPECT_EQUAL(mpt::format<mpt::ustring>::HEX0<12>(3, MPT_USTRING(":"), 0xa2345678), MPT_USTRING("000:0A2:345:678"));
	MPT_TEST_EXPECT_EQUAL(mpt::format<mpt::ustring>::HEX0<7>(3, MPT_USTRING(":"), -0x12345678), MPT_USTRING("-12:345:678"));
	MPT_TEST_EXPECT_EQUAL(mpt::format<mpt::ustring>::HEX0<8>(3, MPT_USTRING(":"), -0x12345678), MPT_USTRING("-12:345:678"));
	MPT_TEST_EXPECT_EQUAL(mpt::format<mpt::ustring>::HEX0<9>(3, MPT_USTRING(":"), -0x12345678), MPT_USTRING("-012:345:678"));
	MPT_TEST_EXPECT_EQUAL(mpt::format<mpt::ustring>::HEX0<10>(3, MPT_USTRING(":"), -0x12345678), MPT_USTRING("-0:012:345:678"));
	MPT_TEST_EXPECT_EQUAL(mpt::format<mpt::ustring>::HEX0<11>(3, MPT_USTRING(":"), -0x12345678), MPT_USTRING("-00:012:345:678"));
	MPT_TEST_EXPECT_EQUAL(mpt::format<mpt::ustring>::HEX0<12>(3, MPT_USTRING(":"), -0x12345678), MPT_USTRING("-000:012:345:678"));

	MPT_TEST_EXPECT_EQUAL(mpt::format<mpt::ustring>::HEX0<5>(3, MPT_USTRING(":"), 0x345678), MPT_USTRING("345:678"));
	MPT_TEST_EXPECT_EQUAL(mpt::format<mpt::ustring>::HEX0<6>(3, MPT_USTRING(":"), 0x345678), MPT_USTRING("345:678"));
	MPT_TEST_EXPECT_EQUAL(mpt::format<mpt::ustring>::HEX0<7>(3, MPT_USTRING(":"), 0x345678), MPT_USTRING("0:345:678"));
	MPT_TEST_EXPECT_EQUAL(mpt::format<mpt::ustring>::HEX0<5>(3, MPT_USTRING(":"), -0x345678), MPT_USTRING("-345:678"));
	MPT_TEST_EXPECT_EQUAL(mpt::format<mpt::ustring>::HEX0<6>(3, MPT_USTRING(":"), -0x345678), MPT_USTRING("-345:678"));
	MPT_TEST_EXPECT_EQUAL(mpt::format<mpt::ustring>::HEX0<7>(3, MPT_USTRING(":"), -0x345678), MPT_USTRING("-0:345:678"));

	MPT_TEST_EXPECT_EQUAL(mpt::format<std::string>::left(3, "a"), "a  ");
	MPT_TEST_EXPECT_EQUAL(mpt::format<std::string>::right(3, "a"), "  a");
	MPT_TEST_EXPECT_EQUAL(mpt::format<std::string>::center(3, "a"), " a ");
	MPT_TEST_EXPECT_EQUAL(mpt::format<std::string>::center(4, "a"), " a  ");

	MPT_TEST_EXPECT_EQUAL(mpt::format<std::string>::flt(6.12345, 3), "6.12");
	MPT_TEST_EXPECT_EQUAL(mpt::format<std::string>::fix(6.12345, 3), "6.123");
	MPT_TEST_EXPECT_EQUAL(mpt::format<std::string>::flt(6.12345, 4), "6.123");
	MPT_TEST_EXPECT_EQUAL(mpt::format<std::string>::fix(6.12345, 4), "6.1235");

	MPT_TEST_EXPECT_EQUAL(mpt::format<std::u32string>::val(std::u32string(U"foo")), U"foo");
	MPT_TEST_EXPECT_EQUAL(mpt::format<std::u32string>::val(std::u32string_view(U"foo")), U"foo");

#if MPT_DETECTED_MFC

	MPT_TEST_EXPECT_EQUAL(mpt::format<CString>::flt(58.65403492763, 6), TEXT("58.654"));
	MPT_TEST_EXPECT_EQUAL(mpt::format<CString>::fix(23.42, 1), TEXT("23.4"));
	MPT_TEST_EXPECT_EQUAL(mpt::format<CString>::fix(234.2, 1), TEXT("234.2"));
	MPT_TEST_EXPECT_EQUAL(mpt::format<CString>::fix(2342.0, 1), TEXT("2342.0"));

	MPT_TEST_EXPECT_EQUAL(mpt::format<CString>::dec(2, TEXT(";"), 2345678), CString(TEXT("2;34;56;78")));
	MPT_TEST_EXPECT_EQUAL(mpt::format<CString>::dec(2, TEXT(";"), 12345678), CString(TEXT("12;34;56;78")));
	MPT_TEST_EXPECT_EQUAL(mpt::format<CString>::hex(3, TEXT(":"), 0xa2345678), CString(TEXT("a2:345:678")));

	MPT_TEST_EXPECT_EQUAL(mpt::format<CString>::dec(2, TEXT(";"), 12345678), CString(TEXT("12;34;56;78")));
	MPT_TEST_EXPECT_EQUAL(mpt::format<CString>::hex(3, TEXT(":"), 0xa2345678), CString(TEXT("a2:345:678")));

	MPT_TEST_EXPECT_EQUAL(mpt::format<CString>::HEX0<7>(3, TEXT(":"), 0xa2345678), CString(TEXT("A2:345:678")));
	MPT_TEST_EXPECT_EQUAL(mpt::format<CString>::HEX0<8>(3, TEXT(":"), 0xa2345678), CString(TEXT("A2:345:678")));
	MPT_TEST_EXPECT_EQUAL(mpt::format<CString>::HEX0<9>(3, TEXT(":"), 0xa2345678), CString(TEXT("0A2:345:678")));
	MPT_TEST_EXPECT_EQUAL(mpt::format<CString>::HEX0<10>(3, TEXT(":"), 0xa2345678), CString(TEXT("0:0A2:345:678")));
	MPT_TEST_EXPECT_EQUAL(mpt::format<CString>::HEX0<11>(3, TEXT(":"), 0xa2345678), CString(TEXT("00:0A2:345:678")));
	MPT_TEST_EXPECT_EQUAL(mpt::format<CString>::HEX0<12>(3, TEXT(":"), 0xa2345678), CString(TEXT("000:0A2:345:678")));
	MPT_TEST_EXPECT_EQUAL(mpt::format<CString>::HEX0<7>(3, TEXT(":"), -0x12345678), CString(TEXT("-12:345:678")));
	MPT_TEST_EXPECT_EQUAL(mpt::format<CString>::HEX0<8>(3, TEXT(":"), -0x12345678), CString(TEXT("-12:345:678")));
	MPT_TEST_EXPECT_EQUAL(mpt::format<CString>::HEX0<9>(3, TEXT(":"), -0x12345678), CString(TEXT("-012:345:678")));
	MPT_TEST_EXPECT_EQUAL(mpt::format<CString>::HEX0<10>(3, TEXT(":"), -0x12345678), CString(TEXT("-0:012:345:678")));
	MPT_TEST_EXPECT_EQUAL(mpt::format<CString>::HEX0<11>(3, TEXT(":"), -0x12345678), CString(TEXT("-00:012:345:678")));
	MPT_TEST_EXPECT_EQUAL(mpt::format<CString>::HEX0<12>(3, TEXT(":"), -0x12345678), CString(TEXT("-000:012:345:678")));

	MPT_TEST_EXPECT_EQUAL(mpt::format<CString>::HEX0<5>(3, TEXT(":"), 0x345678), CString(TEXT("345:678")));
	MPT_TEST_EXPECT_EQUAL(mpt::format<CString>::HEX0<6>(3, TEXT(":"), 0x345678), CString(TEXT("345:678")));
	MPT_TEST_EXPECT_EQUAL(mpt::format<CString>::HEX0<7>(3, TEXT(":"), 0x345678), CString(TEXT("0:345:678")));
	MPT_TEST_EXPECT_EQUAL(mpt::format<CString>::HEX0<5>(3, TEXT(":"), -0x345678), CString(TEXT("-345:678")));
	MPT_TEST_EXPECT_EQUAL(mpt::format<CString>::HEX0<6>(3, TEXT(":"), -0x345678), CString(TEXT("-345:678")));
	MPT_TEST_EXPECT_EQUAL(mpt::format<CString>::HEX0<7>(3, TEXT(":"), -0x345678), CString(TEXT("-0:345:678")));

	MPT_TEST_EXPECT_EQUAL(mpt::format<CString>::left(3, TEXT("a")), TEXT("a  "));
	MPT_TEST_EXPECT_EQUAL(mpt::format<CString>::right(3, TEXT("a")), TEXT("  a"));
	MPT_TEST_EXPECT_EQUAL(mpt::format<CString>::center(3, TEXT("a")), TEXT(" a "));
	MPT_TEST_EXPECT_EQUAL(mpt::format<CString>::center(4, TEXT("a")), TEXT(" a  "));

	MPT_TEST_EXPECT_EQUAL(mpt::format<CString>::flt(6.12345, 3), TEXT("6.12"));
	MPT_TEST_EXPECT_EQUAL(mpt::format<CString>::fix(6.12345, 3), TEXT("6.123"));
	MPT_TEST_EXPECT_EQUAL(mpt::format<CString>::flt(6.12345, 4), TEXT("6.123"));
	MPT_TEST_EXPECT_EQUAL(mpt::format<CString>::fix(6.12345, 4), TEXT("6.1235"));

#endif

#if !defined(MPT_COMPILER_QUIRK_NO_WCHAR)
	MPT_TEST_EXPECT_EQUAL(mpt::format<std::wstring>::flt(6.12345, 3), L"6.12");
	MPT_TEST_EXPECT_EQUAL(mpt::format<std::wstring>::fix(6.12345, 3), L"6.123");
	MPT_TEST_EXPECT_EQUAL(mpt::format<std::wstring>::flt(6.12345, 4), L"6.123");
#endif // !MPT_COMPILER_QUIRK_NO_WCHAR
}

} // namespace simple
} // namespace format
} // namespace tests



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_FORMAT_TESTS_FORMAT_SIMPLE_HPP
