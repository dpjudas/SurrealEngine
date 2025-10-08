/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_FORMAT_TESTS_FORMAT_MESSAGE_HPP
#define MPT_FORMAT_TESTS_FORMAT_MESSAGE_HPP



#include "mpt/base/detect.hpp"
#include "mpt/base/namespace.hpp"
#include "mpt/format/message.hpp"
#include "mpt/format/message_macros.hpp"
#include "mpt/string/types.hpp"
#include "mpt/test/test.hpp"
#include "mpt/test/test_macros.hpp"



namespace mpt {
inline namespace MPT_INLINE_NS {



namespace tests {
namespace format {
namespace message {

#if MPT_COMPILER_CLANG
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wglobal-constructors"
#endif
MPT_TEST_GROUP_INLINE("mpt/format/message")
#if MPT_COMPILER_CLANG
#pragma clang diagnostic pop
#endif
{
	static_assert(mpt::parse_format_string_argument_count("") == 0);
	static_assert(mpt::parse_format_string_argument_count("{{") == 0);
	static_assert(mpt::parse_format_string_argument_count("}}") == 0);
	static_assert(mpt::parse_format_string_argument_count("{}") == 1);
	static_assert(mpt::parse_format_string_argument_count("{}{}") == 2);
	static_assert(mpt::parse_format_string_argument_count("{0}{1}") == 2);

	// basic
	MPT_TEST_EXPECT_EQUAL(MPT_AFORMAT_MESSAGE("{}{}{}")(1, 2, 3), "123");
	MPT_TEST_EXPECT_EQUAL(MPT_AFORMAT_MESSAGE("{2}{1}{0}")(1, 2, 3), "321");

	MPT_TEST_EXPECT_EQUAL(MPT_AFORMAT_MESSAGE("{2}{1}{0}{4}{3}{6}{5}{7}{10}{9}{8}")(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, "a"), "21043657a98");

#if !defined(MPT_COMPILER_QUIRK_NO_WCHAR)
	MPT_TEST_EXPECT_EQUAL(MPT_AFORMAT_MESSAGE(L"{}{}{}")(1, 2, 3), L"123");
#endif // !MPT_COMPILER_QUIRK_NO_WCHAR

	// escaping behviour
	MPT_TEST_EXPECT_EQUAL(MPT_AFORMAT_MESSAGE("%")(), "%");
	MPT_TEST_EXPECT_EQUAL(MPT_AFORMAT_MESSAGE("%")(), "%");
	MPT_TEST_EXPECT_EQUAL(MPT_AFORMAT_MESSAGE("%%")(), "%%");
	MPT_TEST_EXPECT_EQUAL(MPT_AFORMAT_MESSAGE("{}")("a"), "a");
	MPT_TEST_EXPECT_EQUAL(MPT_AFORMAT_MESSAGE("{}%")("a"), "a%");
	MPT_TEST_EXPECT_EQUAL(MPT_AFORMAT_MESSAGE("{}%")("a"), "a%");
	MPT_TEST_EXPECT_EQUAL(MPT_AFORMAT_MESSAGE("{}%%")("a"), "a%%");
	MPT_TEST_EXPECT_EQUAL(MPT_AFORMAT_MESSAGE("%1")(), "%1");
	MPT_TEST_EXPECT_EQUAL(MPT_AFORMAT_MESSAGE("%{}")("a"), "%a");
	MPT_TEST_EXPECT_EQUAL(MPT_AFORMAT_MESSAGE("%b")(), "%b");
	MPT_TEST_EXPECT_EQUAL(MPT_AFORMAT_MESSAGE("{{}}")(), "{}");
	MPT_TEST_EXPECT_EQUAL(MPT_AFORMAT_MESSAGE("{{{}}}")("a"), "{a}");

	// formatting string_view
	MPT_TEST_EXPECT_EQUAL(MPT_UFORMAT_MESSAGE("{}")(mpt::ustring(MPT_ULITERAL("foo"))), MPT_USTRING("foo"));
	MPT_TEST_EXPECT_EQUAL(MPT_UFORMAT_MESSAGE("{}")(mpt::ustring_view(MPT_ULITERAL("foo"))), MPT_USTRING("foo"));
}

} // namespace message
} // namespace format
} // namespace tests



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_FORMAT_TESTS_FORMAT_MESSAGE_HPP
