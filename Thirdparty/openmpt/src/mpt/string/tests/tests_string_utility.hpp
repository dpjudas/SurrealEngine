/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_STRING_TESTS_STRING_UTILITY_HPP
#define MPT_STRING_TESTS_STRING_UTILITY_HPP



#include "mpt/base/detect.hpp"
#include "mpt/base/namespace.hpp"
#include "mpt/string/utility.hpp"
#include "mpt/test/test.hpp"
#include "mpt/test/test_macros.hpp"

#include <string>

#include <cstddef>



namespace mpt {
inline namespace MPT_INLINE_NS {



namespace tests {
namespace string {
namespace utility {

#if MPT_COMPILER_CLANG
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wglobal-constructors"
#endif
MPT_TEST_GROUP_INLINE("mpt/string/utility")
#if MPT_COMPILER_CLANG
#pragma clang diagnostic pop
#endif
{
	MPT_TEST_EXPECT_EQUAL(mpt::trim_left(std::string(" ")), "");
	MPT_TEST_EXPECT_EQUAL(mpt::trim_right(std::string(" ")), "");
	MPT_TEST_EXPECT_EQUAL(mpt::trim(std::string(" ")), "");

	// weird things with std::string containing \0 in the middle and trimming \0
	MPT_TEST_EXPECT_EQUAL(std::string("\0\ta\0b ", 6).length(), (std::size_t)6);
	MPT_TEST_EXPECT_EQUAL(mpt::trim_right(std::string("\0\ta\0b ", 6)), std::string("\0\ta\0b", 5));
	MPT_TEST_EXPECT_EQUAL(mpt::trim(std::string("\0\ta\0b\0", 6), std::string("\0", 1)), std::string("\ta\0b", 4));
}

} // namespace utility
} // namespace string
} // namespace tests



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_STRING_TESTS_STRING_UTILITY_HPP
