/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_STRING_TESTS_STRING_BUFFER_HPP
#define MPT_STRING_TESTS_STRING_BUFFER_HPP



#include "mpt/base/detect_compiler.hpp"
#include "mpt/base/namespace.hpp"
#include "mpt/string/buffer.hpp"
#include "mpt/test/test.hpp"
#include "mpt/test/test_macros.hpp"

#include <string>



namespace mpt {
inline namespace MPT_INLINE_NS {



namespace tests {
namespace string {
namespace buffer {

#if MPT_COMPILER_CLANG
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wglobal-constructors"
#endif
MPT_TEST_GROUP_INLINE("mpt/string/buffer")
#if MPT_COMPILER_CLANG
#pragma clang diagnostic pop
#endif
{
	{
		char buf[4] = {'x', 'x', 'x', 'x'};
		mpt::WriteAutoBuf(buf) = std::string("foobar");
		MPT_TEST_EXPECT_EQUAL(buf[0], 'f');
		MPT_TEST_EXPECT_EQUAL(buf[1], 'o');
		MPT_TEST_EXPECT_EQUAL(buf[2], 'o');
		MPT_TEST_EXPECT_EQUAL(buf[3], '\0');
	}
	{
		char buf[4] = {'x', 'x', 'x', 'x'};
		char foobar[] = {'f', 'o', 'o', 'b', 'a', 'r', '\0'};
		mpt::WriteTypedBuf<std::string>(buf) = (char *)foobar;
		MPT_TEST_EXPECT_EQUAL(buf[0], 'f');
		MPT_TEST_EXPECT_EQUAL(buf[1], 'o');
		MPT_TEST_EXPECT_EQUAL(buf[2], 'o');
		MPT_TEST_EXPECT_EQUAL(buf[3], '\0');
	}
	{
		char buf[4] = {'x', 'x', 'x', 'x'};
		mpt::WriteTypedBuf<std::string>(buf) = (const char *)"foobar";
		MPT_TEST_EXPECT_EQUAL(buf[0], 'f');
		MPT_TEST_EXPECT_EQUAL(buf[1], 'o');
		MPT_TEST_EXPECT_EQUAL(buf[2], 'o');
		MPT_TEST_EXPECT_EQUAL(buf[3], '\0');
	}
	{
		char buf[4] = {'x', 'x', 'x', 'x'};
		mpt::WriteTypedBuf<std::string>(buf) = "foobar";
		MPT_TEST_EXPECT_EQUAL(buf[0], 'f');
		MPT_TEST_EXPECT_EQUAL(buf[1], 'o');
		MPT_TEST_EXPECT_EQUAL(buf[2], 'o');
		MPT_TEST_EXPECT_EQUAL(buf[3], '\0');
	}
	{
		const char buf[4] = {'f', 'o', 'o', 'b'};
		std::string foo = mpt::ReadAutoBuf(buf);
		MPT_TEST_EXPECT_EQUAL(foo, std::string("foob"));
	}
}

} // namespace buffer
} // namespace string
} // namespace tests



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_STRING_TESTS_STRING_BUFFER_HPP
