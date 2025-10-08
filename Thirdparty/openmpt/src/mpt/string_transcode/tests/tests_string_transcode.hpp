/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_STRING_TRANSCODE_TESTS_STRING_TRANSCODE_HPP
#define MPT_STRING_TRANSCODE_TESTS_STRING_TRANSCODE_HPP



#include "mpt/base/detect.hpp"
#include "mpt/base/namespace.hpp"
#include "mpt/string/types.hpp"
#include "mpt/string_transcode/macros.hpp"
#include "mpt/string_transcode/transcode.hpp"
#include "mpt/test/test.hpp"
#include "mpt/test/test_macros.hpp"

#include <string>
#include <string_view>



namespace mpt {
inline namespace MPT_INLINE_NS {



namespace tests {
namespace string_transcode {

#if MPT_COMPILER_CLANG
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wglobal-constructors"
#endif
MPT_TEST_GROUP_INLINE("mpt/string_transcode")
#if MPT_COMPILER_CLANG
#pragma clang diagnostic pop
#endif
{
	// MPT_UTF8_STRING version

	// Charset conversions (basic sanity checks)
	MPT_TEST_EXPECT_EQUAL(mpt::transcode<std::string>(mpt::common_encoding::utf8, MPT_USTRING("a")), "a");
	MPT_TEST_EXPECT_EQUAL(mpt::transcode<std::string>(mpt::common_encoding::iso8859_1, MPT_USTRING("a")), "a");
	MPT_TEST_EXPECT_EQUAL(mpt::transcode<std::string>(mpt::common_encoding::ascii, MPT_USTRING("a")), "a");
	MPT_TEST_EXPECT_EQUAL(mpt::transcode<mpt::ustring>(mpt::common_encoding::utf8, "a"), MPT_USTRING("a"));
	MPT_TEST_EXPECT_EQUAL(mpt::transcode<mpt::ustring>(mpt::common_encoding::iso8859_1, "a"), MPT_USTRING("a"));
	MPT_TEST_EXPECT_EQUAL(mpt::transcode<mpt::ustring>(mpt::common_encoding::ascii, "a"), MPT_USTRING("a"));

	MPT_TEST_EXPECT_EQUAL(mpt::transcode<std::string>(mpt::logical_encoding::locale, MPT_USTRING("a")), "a");
	MPT_TEST_EXPECT_EQUAL(mpt::transcode<mpt::ustring>(mpt::logical_encoding::locale, "a"), MPT_USTRING("a"));
	MPT_TEST_EXPECT_EQUAL(mpt::transcode<std::string>(mpt::logical_encoding::active_locale, MPT_USTRING("a")), "a");
	MPT_TEST_EXPECT_EQUAL(mpt::transcode<mpt::ustring>(mpt::logical_encoding::active_locale, "a"), MPT_USTRING("a"));

	MPT_TEST_EXPECT_EQUAL(mpt::transcode<std::string>(mpt::common_encoding::utf8, MPT_UTF8_STRING("a")), "a");
	MPT_TEST_EXPECT_EQUAL(mpt::transcode<std::string>(mpt::common_encoding::iso8859_1, MPT_UTF8_STRING("a")), "a");
	MPT_TEST_EXPECT_EQUAL(mpt::transcode<std::string>(mpt::common_encoding::ascii, MPT_UTF8_STRING("a")), "a");
	MPT_TEST_EXPECT_EQUAL(mpt::transcode<mpt::ustring>(mpt::common_encoding::utf8, "a"), MPT_UTF8_STRING("a"));
	MPT_TEST_EXPECT_EQUAL(mpt::transcode<mpt::ustring>(mpt::common_encoding::iso8859_1, "a"), MPT_UTF8_STRING("a"));
	MPT_TEST_EXPECT_EQUAL(mpt::transcode<mpt::ustring>(mpt::common_encoding::ascii, "a"), MPT_UTF8_STRING("a"));

	MPT_TEST_EXPECT_EQUAL(mpt::transcode<std::string>(mpt::logical_encoding::locale, MPT_UTF8_STRING("a")), "a");
	MPT_TEST_EXPECT_EQUAL(mpt::transcode<mpt::ustring>(mpt::logical_encoding::locale, "a"), MPT_UTF8_STRING("a"));
	MPT_TEST_EXPECT_EQUAL(mpt::transcode<std::string>(mpt::logical_encoding::active_locale, MPT_UTF8_STRING("a")), "a");
	MPT_TEST_EXPECT_EQUAL(mpt::transcode<mpt::ustring>(mpt::logical_encoding::active_locale, "a"), MPT_UTF8_STRING("a"));

#if MPT_OS_EMSCRIPTEN
	MPT_TEST_EXPECT_EQUAL(mpt::transcode<std::string>(mpt::logical_encoding::locale, MPT_UTF8_STRING("\xe2\x8c\x82")), "\xe2\x8c\x82");
#endif // MPT_OS_EMSCRIPTEN

	// Check that some character replacement is done (and not just empty strings or truncated strings are returned)
	// We test german umlaut-a (U+00E4) (\xC3\xA4) and CJK U+5BB6 (\xE5\xAE\xB6)

	MPT_TEST_EXPECT_EXPR(mpt::ends_with(mpt::transcode<std::string>(mpt::common_encoding::ascii, MPT_UTF8_STRING("abc\xC3\xA4xyz")), "xyz"));
	MPT_TEST_EXPECT_EXPR(mpt::ends_with(mpt::transcode<std::string>(mpt::common_encoding::iso8859_1, MPT_UTF8_STRING("abc\xC3\xA4xyz")), "xyz"));
	MPT_TEST_EXPECT_EXPR(mpt::ends_with(mpt::transcode<std::string>(mpt::common_encoding::cp437, MPT_UTF8_STRING("abc\xC3\xA4xyz")), "xyz"));
	MPT_TEST_EXPECT_EXPR(mpt::ends_with(mpt::transcode<std::string>(mpt::common_encoding::utf8, MPT_UTF8_STRING("abc\xC3\xA4xyz")), "xyz"));
	MPT_TEST_EXPECT_EXPR(mpt::starts_with(mpt::transcode<std::string>(mpt::common_encoding::ascii, MPT_UTF8_STRING("abc\xC3\xA4xyz")), "abc"));
	MPT_TEST_EXPECT_EXPR(mpt::starts_with(mpt::transcode<std::string>(mpt::common_encoding::iso8859_1, MPT_UTF8_STRING("abc\xC3\xA4xyz")), "abc"));
	MPT_TEST_EXPECT_EXPR(mpt::starts_with(mpt::transcode<std::string>(mpt::common_encoding::cp437, MPT_UTF8_STRING("abc\xC3\xA4xyz")), "abc"));
	MPT_TEST_EXPECT_EXPR(mpt::starts_with(mpt::transcode<std::string>(mpt::common_encoding::utf8, MPT_UTF8_STRING("abc\xC3\xA4xyz")), "abc"));

	MPT_TEST_EXPECT_EXPR(mpt::ends_with(mpt::transcode<std::string>(mpt::logical_encoding::locale, MPT_UTF8_STRING("abc\xC3\xA4xyz")), "xyz"));
	MPT_TEST_EXPECT_EXPR(mpt::starts_with(mpt::transcode<std::string>(mpt::logical_encoding::locale, MPT_UTF8_STRING("abc\xC3\xA4xyz")), "abc"));
	MPT_TEST_EXPECT_EXPR(mpt::ends_with(mpt::transcode<std::string>(mpt::logical_encoding::active_locale, MPT_UTF8_STRING("abc\xC3\xA4xyz")), "xyz"));
	MPT_TEST_EXPECT_EXPR(mpt::starts_with(mpt::transcode<std::string>(mpt::logical_encoding::active_locale, MPT_UTF8_STRING("abc\xC3\xA4xyz")), "abc"));

	MPT_TEST_EXPECT_EXPR(mpt::ends_with(mpt::transcode<std::string>(mpt::common_encoding::ascii, MPT_UTF8_STRING("abc\xE5\xAE\xB6xyz")), "xyz"));
	MPT_TEST_EXPECT_EXPR(mpt::ends_with(mpt::transcode<std::string>(mpt::common_encoding::iso8859_1, MPT_UTF8_STRING("abc\xE5\xAE\xB6xyz")), "xyz"));
	MPT_TEST_EXPECT_EXPR(mpt::ends_with(mpt::transcode<std::string>(mpt::common_encoding::cp437, MPT_UTF8_STRING("abc\xE5\xAE\xB6xyz")), "xyz"));
	MPT_TEST_EXPECT_EXPR(mpt::ends_with(mpt::transcode<std::string>(mpt::common_encoding::utf8, MPT_UTF8_STRING("abc\xE5\xAE\xB6xyz")), "xyz"));
	MPT_TEST_EXPECT_EXPR(mpt::starts_with(mpt::transcode<std::string>(mpt::common_encoding::ascii, MPT_UTF8_STRING("abc\xE5\xAE\xB6xyz")), "abc"));
	MPT_TEST_EXPECT_EXPR(mpt::starts_with(mpt::transcode<std::string>(mpt::common_encoding::iso8859_1, MPT_UTF8_STRING("abc\xE5\xAE\xB6xyz")), "abc"));
	MPT_TEST_EXPECT_EXPR(mpt::starts_with(mpt::transcode<std::string>(mpt::common_encoding::cp437, MPT_UTF8_STRING("abc\xE5\xAE\xB6xyz")), "abc"));
	MPT_TEST_EXPECT_EXPR(mpt::starts_with(mpt::transcode<std::string>(mpt::common_encoding::utf8, MPT_UTF8_STRING("abc\xE5\xAE\xB6xyz")), "abc"));

	MPT_TEST_EXPECT_EXPR(mpt::ends_with(mpt::transcode<std::string>(mpt::logical_encoding::locale, MPT_UTF8_STRING("abc\xE5\xAE\xB6xyz")), "xyz"));
	MPT_TEST_EXPECT_EXPR(mpt::starts_with(mpt::transcode<std::string>(mpt::logical_encoding::locale, MPT_UTF8_STRING("abc\xE5\xAE\xB6xyz")), "abc"));
	MPT_TEST_EXPECT_EXPR(mpt::ends_with(mpt::transcode<std::string>(mpt::logical_encoding::active_locale, MPT_UTF8_STRING("abc\xE5\xAE\xB6xyz")), "xyz"));
	MPT_TEST_EXPECT_EXPR(mpt::starts_with(mpt::transcode<std::string>(mpt::logical_encoding::active_locale, MPT_UTF8_STRING("abc\xE5\xAE\xB6xyz")), "abc"));

	MPT_TEST_EXPECT_EXPR(mpt::ends_with(mpt::transcode<mpt::ustring>(mpt::common_encoding::ascii, "abc\xC3\xA4xyz"), MPT_USTRING("xyz")));
	MPT_TEST_EXPECT_EXPR(mpt::ends_with(mpt::transcode<mpt::ustring>(mpt::common_encoding::iso8859_1, "abc\xC3\xA4xyz"), MPT_USTRING("xyz")));
	MPT_TEST_EXPECT_EXPR(mpt::ends_with(mpt::transcode<mpt::ustring>(mpt::common_encoding::cp437, "abc\xC3\xA4xyz"), MPT_USTRING("xyz")));
	MPT_TEST_EXPECT_EXPR(mpt::ends_with(mpt::transcode<mpt::ustring>(mpt::common_encoding::utf8, "abc\xC3\xA4xyz"), MPT_USTRING("xyz")));
	MPT_TEST_EXPECT_EXPR(mpt::starts_with(mpt::transcode<mpt::ustring>(mpt::common_encoding::ascii, "abc\xC3\xA4xyz"), MPT_USTRING("abc")));
	MPT_TEST_EXPECT_EXPR(mpt::starts_with(mpt::transcode<mpt::ustring>(mpt::common_encoding::iso8859_1, "abc\xC3\xA4xyz"), MPT_USTRING("abc")));
	MPT_TEST_EXPECT_EXPR(mpt::starts_with(mpt::transcode<mpt::ustring>(mpt::common_encoding::cp437, "abc\xC3\xA4xyz"), MPT_USTRING("abc")));
	MPT_TEST_EXPECT_EXPR(mpt::starts_with(mpt::transcode<mpt::ustring>(mpt::common_encoding::utf8, "abc\xC3\xA4xyz"), MPT_USTRING("abc")));

	MPT_TEST_EXPECT_EXPR(mpt::ends_with(mpt::transcode<mpt::ustring>(mpt::logical_encoding::locale, "abc\xC3\xA4xyz"), MPT_USTRING("xyz")));
	MPT_TEST_EXPECT_EXPR(mpt::starts_with(mpt::transcode<mpt::ustring>(mpt::logical_encoding::locale, "abc\xC3\xA4xyz"), MPT_USTRING("abc")));
	MPT_TEST_EXPECT_EXPR(mpt::ends_with(mpt::transcode<mpt::ustring>(mpt::logical_encoding::active_locale, "abc\xC3\xA4xyz"), MPT_USTRING("xyz")));
	MPT_TEST_EXPECT_EXPR(mpt::starts_with(mpt::transcode<mpt::ustring>(mpt::logical_encoding::active_locale, "abc\xC3\xA4xyz"), MPT_USTRING("abc")));

	MPT_TEST_EXPECT_EXPR(mpt::ends_with(mpt::transcode<mpt::ustring>(mpt::common_encoding::ascii, "abc\xE5\xAE\xB6xyz"), MPT_USTRING("xyz")));
	MPT_TEST_EXPECT_EXPR(mpt::ends_with(mpt::transcode<mpt::ustring>(mpt::common_encoding::iso8859_1, "abc\xE5\xAE\xB6xyz"), MPT_USTRING("xyz")));
	MPT_TEST_EXPECT_EXPR(mpt::ends_with(mpt::transcode<mpt::ustring>(mpt::common_encoding::cp437, "abc\xE5\xAE\xB6xyz"), MPT_USTRING("xyz")));
	MPT_TEST_EXPECT_EXPR(mpt::ends_with(mpt::transcode<mpt::ustring>(mpt::common_encoding::utf8, "abc\xE5\xAE\xB6xyz"), MPT_USTRING("xyz")));
	MPT_TEST_EXPECT_EXPR(mpt::starts_with(mpt::transcode<mpt::ustring>(mpt::common_encoding::ascii, "abc\xE5\xAE\xB6xyz"), MPT_USTRING("abc")));
	MPT_TEST_EXPECT_EXPR(mpt::starts_with(mpt::transcode<mpt::ustring>(mpt::common_encoding::iso8859_1, "abc\xE5\xAE\xB6xyz"), MPT_USTRING("abc")));
	MPT_TEST_EXPECT_EXPR(mpt::starts_with(mpt::transcode<mpt::ustring>(mpt::common_encoding::cp437, "abc\xE5\xAE\xB6xyz"), MPT_USTRING("abc")));
	MPT_TEST_EXPECT_EXPR(mpt::starts_with(mpt::transcode<mpt::ustring>(mpt::common_encoding::utf8, "abc\xE5\xAE\xB6xyz"), MPT_USTRING("abc")));

	MPT_TEST_EXPECT_EXPR(mpt::ends_with(mpt::transcode<mpt::ustring>(mpt::logical_encoding::locale, "abc\xE5\xAE\xB6xyz"), MPT_USTRING("xyz")));
	MPT_TEST_EXPECT_EXPR(mpt::starts_with(mpt::transcode<mpt::ustring>(mpt::logical_encoding::locale, "abc\xE5\xAE\xB6xyz"), MPT_USTRING("abc")));
	MPT_TEST_EXPECT_EXPR(mpt::ends_with(mpt::transcode<mpt::ustring>(mpt::logical_encoding::active_locale, "abc\xE5\xAE\xB6xyz"), MPT_USTRING("xyz")));
	MPT_TEST_EXPECT_EXPR(mpt::starts_with(mpt::transcode<mpt::ustring>(mpt::logical_encoding::active_locale, "abc\xE5\xAE\xB6xyz"), MPT_USTRING("abc")));

	// Check that characters are correctly converted
	// We test german umlaut-a (U+00E4) and CJK U+5BB6

	// cp437
	MPT_TEST_EXPECT_EQUAL(mpt::transcode<std::string>(mpt::common_encoding::cp437, MPT_UTF8_STRING("abc\xC3\xA4xyz")), "abc\x84xyz");
	MPT_TEST_EXPECT_EQUAL(MPT_UTF8_STRING("abc\xC3\xA4xyz"), mpt::transcode<mpt::ustring>(mpt::common_encoding::cp437, "abc\x84xyz"));

	// iso8859
	MPT_TEST_EXPECT_EQUAL(mpt::transcode<std::string>(mpt::common_encoding::iso8859_1, MPT_UTF8_STRING("abc\xC3\xA4xyz")), "abc\xE4xyz");
	MPT_TEST_EXPECT_EQUAL(MPT_UTF8_STRING("abc\xC3\xA4xyz"), mpt::transcode<mpt::ustring>(mpt::common_encoding::iso8859_1, "abc\xE4xyz"));

	// utf8
	MPT_TEST_EXPECT_EQUAL(mpt::transcode<std::string>(mpt::common_encoding::utf8, MPT_UTF8_STRING("abc\xC3\xA4xyz")), "abc\xC3\xA4xyz");
	MPT_TEST_EXPECT_EQUAL(MPT_UTF8_STRING("abc\xC3\xA4xyz"), mpt::transcode<mpt::ustring>(mpt::common_encoding::utf8, "abc\xC3\xA4xyz"));
	MPT_TEST_EXPECT_EQUAL(mpt::transcode<std::string>(mpt::common_encoding::utf8, MPT_UTF8_STRING("abc\xE5\xAE\xB6xyz")), "abc\xE5\xAE\xB6xyz");
	MPT_TEST_EXPECT_EQUAL(MPT_UTF8_STRING("abc\xE5\xAE\xB6xyz"), mpt::transcode<mpt::ustring>(mpt::common_encoding::utf8, "abc\xE5\xAE\xB6xyz"));

	// utf16
	MPT_TEST_EXPECT_EQUAL(mpt::transcode<std::string>(mpt::common_encoding::utf8, std::u16string(1, char16_t{0xe4})), "\xC3\xA4");
	MPT_TEST_EXPECT_EQUAL(std::u16string(1, char16_t{0xe4}), mpt::transcode<std::u16string>(mpt::common_encoding::utf8, "\xC3\xA4"));
	MPT_TEST_EXPECT_EQUAL(mpt::transcode<std::string>(mpt::common_encoding::utf8, std::u16string(1, char16_t{0x5BB6})), "\xE5\xAE\xB6");
	MPT_TEST_EXPECT_EQUAL(std::u16string(1, char16_t{0x5BB6}), mpt::transcode<std::u16string>(mpt::common_encoding::utf8, "\xE5\xAE\xB6"));

	// utf32
	MPT_TEST_EXPECT_EQUAL(mpt::transcode<std::string>(mpt::common_encoding::utf8, std::u32string(1, char32_t{0xe4})), "\xC3\xA4");
	MPT_TEST_EXPECT_EQUAL(std::u32string(1, char32_t{0xe4}), mpt::transcode<std::u32string>(mpt::common_encoding::utf8, "\xC3\xA4"));
	MPT_TEST_EXPECT_EQUAL(mpt::transcode<std::string>(mpt::common_encoding::utf8, std::u32string(1, char32_t{0x5BB6})), "\xE5\xAE\xB6");
	MPT_TEST_EXPECT_EQUAL(std::u32string(1, char32_t{0x5BB6}), mpt::transcode<std::u32string>(mpt::common_encoding::utf8, "\xE5\xAE\xB6"));

#if !defined(MPT_COMPILER_QUIRK_NO_WCHAR)

	// wide L"" version

	// Charset conversions (basic sanity checks)
	MPT_TEST_EXPECT_EQUAL(mpt::transcode<std::string>(mpt::common_encoding::utf8, L"a"), "a");
	MPT_TEST_EXPECT_EQUAL(mpt::transcode<std::string>(mpt::common_encoding::iso8859_1, L"a"), "a");
	MPT_TEST_EXPECT_EQUAL(mpt::transcode<std::string>(mpt::common_encoding::ascii, L"a"), "a");
	MPT_TEST_EXPECT_EQUAL(mpt::transcode<std::wstring>(mpt::common_encoding::utf8, "a"), L"a");
	MPT_TEST_EXPECT_EQUAL(mpt::transcode<std::wstring>(mpt::common_encoding::iso8859_1, "a"), L"a");
	MPT_TEST_EXPECT_EQUAL(mpt::transcode<std::wstring>(mpt::common_encoding::ascii, "a"), L"a");

	MPT_TEST_EXPECT_EQUAL(mpt::transcode<std::string>(mpt::logical_encoding::locale, L"a"), "a");
	MPT_TEST_EXPECT_EQUAL(mpt::transcode<std::wstring>(mpt::logical_encoding::locale, "a"), L"a");
	MPT_TEST_EXPECT_EQUAL(mpt::transcode<std::string>(mpt::logical_encoding::active_locale, L"a"), "a");
	MPT_TEST_EXPECT_EQUAL(mpt::transcode<std::wstring>(mpt::logical_encoding::active_locale, "a"), L"a");

	// Check that some character replacement is done (and not just empty strings or truncated strings are returned)
	// We test german umlaut-a (U+00E4) and CJK U+5BB6

#if MPT_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable : 4428) // universal-character-name encountered in source
#endif

	MPT_TEST_EXPECT_EXPR(mpt::ends_with(mpt::transcode<std::string>(mpt::common_encoding::ascii, L"abc\u00E4xyz"), "xyz"));
	MPT_TEST_EXPECT_EXPR(mpt::ends_with(mpt::transcode<std::string>(mpt::common_encoding::iso8859_1, L"abc\u00E4xyz"), "xyz"));
	MPT_TEST_EXPECT_EXPR(mpt::ends_with(mpt::transcode<std::string>(mpt::common_encoding::cp437, L"abc\u00E4xyz"), "xyz"));
	MPT_TEST_EXPECT_EXPR(mpt::ends_with(mpt::transcode<std::string>(mpt::common_encoding::utf8, L"abc\u00E4xyz"), "xyz"));
	MPT_TEST_EXPECT_EXPR(mpt::starts_with(mpt::transcode<std::string>(mpt::common_encoding::ascii, L"abc\u00E4xyz"), "abc"));
	MPT_TEST_EXPECT_EXPR(mpt::starts_with(mpt::transcode<std::string>(mpt::common_encoding::iso8859_1, L"abc\u00E4xyz"), "abc"));
	MPT_TEST_EXPECT_EXPR(mpt::starts_with(mpt::transcode<std::string>(mpt::common_encoding::cp437, L"abc\u00E4xyz"), "abc"));
	MPT_TEST_EXPECT_EXPR(mpt::starts_with(mpt::transcode<std::string>(mpt::common_encoding::utf8, L"abc\u00E4xyz"), "abc"));

	MPT_TEST_EXPECT_EXPR(mpt::ends_with(mpt::transcode<std::string>(mpt::logical_encoding::locale, L"abc\u00E4xyz"), "xyz"));
	MPT_TEST_EXPECT_EXPR(mpt::starts_with(mpt::transcode<std::string>(mpt::logical_encoding::locale, L"abc\u00E4xyz"), "abc"));
	MPT_TEST_EXPECT_EXPR(mpt::ends_with(mpt::transcode<std::string>(mpt::logical_encoding::active_locale, L"abc\u00E4xyz"), "xyz"));
	MPT_TEST_EXPECT_EXPR(mpt::starts_with(mpt::transcode<std::string>(mpt::logical_encoding::active_locale, L"abc\u00E4xyz"), "abc"));

	MPT_TEST_EXPECT_EXPR(mpt::ends_with(mpt::transcode<std::string>(mpt::common_encoding::ascii, L"abc\u5BB6xyz"), "xyz"));
	MPT_TEST_EXPECT_EXPR(mpt::ends_with(mpt::transcode<std::string>(mpt::common_encoding::iso8859_1, L"abc\u5BB6xyz"), "xyz"));
	MPT_TEST_EXPECT_EXPR(mpt::ends_with(mpt::transcode<std::string>(mpt::common_encoding::cp437, L"abc\u5BB6xyz"), "xyz"));
	MPT_TEST_EXPECT_EXPR(mpt::ends_with(mpt::transcode<std::string>(mpt::common_encoding::utf8, L"abc\u5BB6xyz"), "xyz"));
	MPT_TEST_EXPECT_EXPR(mpt::starts_with(mpt::transcode<std::string>(mpt::common_encoding::ascii, L"abc\u5BB6xyz"), "abc"));
	MPT_TEST_EXPECT_EXPR(mpt::starts_with(mpt::transcode<std::string>(mpt::common_encoding::iso8859_1, L"abc\u5BB6xyz"), "abc"));
	MPT_TEST_EXPECT_EXPR(mpt::starts_with(mpt::transcode<std::string>(mpt::common_encoding::cp437, L"abc\u5BB6xyz"), "abc"));
	MPT_TEST_EXPECT_EXPR(mpt::starts_with(mpt::transcode<std::string>(mpt::common_encoding::utf8, L"abc\u5BB6xyz"), "abc"));

	MPT_TEST_EXPECT_EXPR(mpt::ends_with(mpt::transcode<std::string>(mpt::logical_encoding::locale, L"abc\u5BB6xyz"), "xyz"));
	MPT_TEST_EXPECT_EXPR(mpt::starts_with(mpt::transcode<std::string>(mpt::logical_encoding::locale, L"abc\u5BB6xyz"), "abc"));
	MPT_TEST_EXPECT_EXPR(mpt::ends_with(mpt::transcode<std::string>(mpt::logical_encoding::active_locale, L"abc\u5BB6xyz"), "xyz"));
	MPT_TEST_EXPECT_EXPR(mpt::starts_with(mpt::transcode<std::string>(mpt::logical_encoding::active_locale, L"abc\u5BB6xyz"), "abc"));

	MPT_TEST_EXPECT_EXPR(mpt::ends_with(mpt::transcode<std::wstring>(mpt::common_encoding::ascii, "abc\xC3\xA4xyz"), L"xyz"));
	MPT_TEST_EXPECT_EXPR(mpt::ends_with(mpt::transcode<std::wstring>(mpt::common_encoding::iso8859_1, "abc\xC3\xA4xyz"), L"xyz"));
	MPT_TEST_EXPECT_EXPR(mpt::ends_with(mpt::transcode<std::wstring>(mpt::common_encoding::cp437, "abc\xC3\xA4xyz"), L"xyz"));
	MPT_TEST_EXPECT_EXPR(mpt::ends_with(mpt::transcode<std::wstring>(mpt::common_encoding::utf8, "abc\xC3\xA4xyz"), L"xyz"));
	MPT_TEST_EXPECT_EXPR(mpt::starts_with(mpt::transcode<std::wstring>(mpt::common_encoding::ascii, "abc\xC3\xA4xyz"), L"abc"));
	MPT_TEST_EXPECT_EXPR(mpt::starts_with(mpt::transcode<std::wstring>(mpt::common_encoding::iso8859_1, "abc\xC3\xA4xyz"), L"abc"));
	MPT_TEST_EXPECT_EXPR(mpt::starts_with(mpt::transcode<std::wstring>(mpt::common_encoding::cp437, "abc\xC3\xA4xyz"), L"abc"));
	MPT_TEST_EXPECT_EXPR(mpt::starts_with(mpt::transcode<std::wstring>(mpt::common_encoding::utf8, "abc\xC3\xA4xyz"), L"abc"));

	MPT_TEST_EXPECT_EXPR(mpt::ends_with(mpt::transcode<std::wstring>(mpt::logical_encoding::locale, "abc\xC3\xA4xyz"), L"xyz"));
	MPT_TEST_EXPECT_EXPR(mpt::starts_with(mpt::transcode<std::wstring>(mpt::logical_encoding::locale, "abc\xC3\xA4xyz"), L"abc"));
	MPT_TEST_EXPECT_EXPR(mpt::ends_with(mpt::transcode<std::wstring>(mpt::logical_encoding::active_locale, "abc\xC3\xA4xyz"), L"xyz"));
	MPT_TEST_EXPECT_EXPR(mpt::starts_with(mpt::transcode<std::wstring>(mpt::logical_encoding::active_locale, "abc\xC3\xA4xyz"), L"abc"));

	MPT_TEST_EXPECT_EXPR(mpt::ends_with(mpt::transcode<std::wstring>(mpt::common_encoding::ascii, "abc\xE5\xAE\xB6xyz"), L"xyz"));
	MPT_TEST_EXPECT_EXPR(mpt::ends_with(mpt::transcode<std::wstring>(mpt::common_encoding::iso8859_1, "abc\xE5\xAE\xB6xyz"), L"xyz"));
	MPT_TEST_EXPECT_EXPR(mpt::ends_with(mpt::transcode<std::wstring>(mpt::common_encoding::cp437, "abc\xE5\xAE\xB6xyz"), L"xyz"));
	MPT_TEST_EXPECT_EXPR(mpt::ends_with(mpt::transcode<std::wstring>(mpt::common_encoding::utf8, "abc\xE5\xAE\xB6xyz"), L"xyz"));
	MPT_TEST_EXPECT_EXPR(mpt::starts_with(mpt::transcode<std::wstring>(mpt::common_encoding::ascii, "abc\xE5\xAE\xB6xyz"), L"abc"));
	MPT_TEST_EXPECT_EXPR(mpt::starts_with(mpt::transcode<std::wstring>(mpt::common_encoding::iso8859_1, "abc\xE5\xAE\xB6xyz"), L"abc"));
	MPT_TEST_EXPECT_EXPR(mpt::starts_with(mpt::transcode<std::wstring>(mpt::common_encoding::cp437, "abc\xE5\xAE\xB6xyz"), L"abc"));
	MPT_TEST_EXPECT_EXPR(mpt::starts_with(mpt::transcode<std::wstring>(mpt::common_encoding::utf8, "abc\xE5\xAE\xB6xyz"), L"abc"));

	MPT_TEST_EXPECT_EXPR(mpt::ends_with(mpt::transcode<std::wstring>(mpt::logical_encoding::locale, "abc\xE5\xAE\xB6xyz"), L"xyz"));
	MPT_TEST_EXPECT_EXPR(mpt::starts_with(mpt::transcode<std::wstring>(mpt::logical_encoding::locale, "abc\xE5\xAE\xB6xyz"), L"abc"));
	MPT_TEST_EXPECT_EXPR(mpt::ends_with(mpt::transcode<std::wstring>(mpt::logical_encoding::active_locale, "abc\xE5\xAE\xB6xyz"), L"xyz"));
	MPT_TEST_EXPECT_EXPR(mpt::starts_with(mpt::transcode<std::wstring>(mpt::logical_encoding::active_locale, "abc\xE5\xAE\xB6xyz"), L"abc"));

	// Check that characters are correctly converted
	// We test german umlaut-a (U+00E4) and CJK U+5BB6

	// cp437
	MPT_TEST_EXPECT_EQUAL(mpt::transcode<std::string>(mpt::common_encoding::cp437, L"abc\u00E4xyz"), "abc\x84xyz");
	MPT_TEST_EXPECT_EQUAL(L"abc\u00E4xyz", mpt::transcode<std::wstring>(mpt::common_encoding::cp437, "abc\x84xyz"));

	// iso8859
	MPT_TEST_EXPECT_EQUAL(mpt::transcode<std::string>(mpt::common_encoding::iso8859_1, L"abc\u00E4xyz"), "abc\xE4xyz");
	MPT_TEST_EXPECT_EQUAL(L"abc\u00E4xyz", mpt::transcode<std::wstring>(mpt::common_encoding::iso8859_1, "abc\xE4xyz"));

	// utf8
	MPT_TEST_EXPECT_EQUAL(mpt::transcode<std::string>(mpt::common_encoding::utf8, L"abc\u00E4xyz"), "abc\xC3\xA4xyz");
	MPT_TEST_EXPECT_EQUAL(L"abc\u00E4xyz", mpt::transcode<std::wstring>(mpt::common_encoding::utf8, "abc\xC3\xA4xyz"));
	MPT_TEST_EXPECT_EQUAL(mpt::transcode<std::string>(mpt::common_encoding::utf8, L"abc\u5BB6xyz"), "abc\xE5\xAE\xB6xyz");
	MPT_TEST_EXPECT_EQUAL(L"abc\u5BB6xyz", mpt::transcode<std::wstring>(mpt::common_encoding::utf8, "abc\xE5\xAE\xB6xyz"));

#if MPT_COMPILER_MSVC
#pragma warning(pop)
#endif

#endif // !MPT_COMPILER_QUIRK_NO_WCHAR

	// string_view
	MPT_TEST_EXPECT_EQUAL(mpt::transcode<mpt::ustring>(U"foo"), MPT_USTRING("foo"));
	MPT_TEST_EXPECT_EQUAL(mpt::transcode<mpt::ustring>(std::u32string_view(U"foo")), MPT_USTRING("foo"));
	MPT_TEST_EXPECT_EQUAL(mpt::transcode<mpt::ustring>(std::u32string(U"foo")), MPT_USTRING("foo"));

	// bogus unknown -> unknown transcode
	MPT_TEST_EXPECT_EQUAL(mpt::transcode<std::string>(std::string("foo")), std::string("foo"));
}

} // namespace string_transcode
} // namespace tests



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_STRING_TRANSCODE_TESTS_STRING_TRANSCODE_HPP
