/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_ENDIAN_TESTS_ENDIAN_INT24_HPP
#define MPT_ENDIAN_TESTS_ENDIAN_INT24_HPP



#include "mpt/base/detect_compiler.hpp"
#include "mpt/base/integer.hpp"
#include "mpt/base/macros.hpp"
#include "mpt/base/namespace.hpp"
#include "mpt/endian/int24.hpp"
#include "mpt/endian/integer.hpp"
#include "mpt/test/test.hpp"
#include "mpt/test/test_macros.hpp"

#include <limits>

#include <cstring>



namespace mpt {
inline namespace MPT_INLINE_NS {



namespace tests {
namespace endian {
namespace integer {

#if MPT_COMPILER_CLANG
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wglobal-constructors"
#endif
MPT_TEST_GROUP_INLINE("mpt/endian/int24")
#if MPT_COMPILER_CLANG
#pragma clang diagnostic pop
#endif
{
	static_assert(std::numeric_limits<int24le>::min() == std::numeric_limits<int24>::min());
	static_assert(std::numeric_limits<uint24le>::min() == std::numeric_limits<uint24>::min());

	{
		int24le le24;
		le24.set(int24(0x123456));
		int24be be24;
		be24.set(int24(0x123456));
		MPT_TEST_EXPECT_EQUAL(le24, int24(0x123456));
		MPT_TEST_EXPECT_EQUAL(be24, int24(0x123456));
		MPT_TEST_EXPECT_EQUAL(std::memcmp(&le24, "\x56\x34\x12", 3), 0);
		MPT_TEST_EXPECT_EQUAL(std::memcmp(&be24, "\x12\x34\x56", 3), 0);
	}
	{
		int24le le24;
		le24.set(int24(-0x7fffff));
		int24be be24;
		be24.set(int24(-0x7fffff));
		MPT_TEST_EXPECT_EQUAL(le24, int24(-0x7fffff));
		MPT_TEST_EXPECT_EQUAL(be24, int24(-0x7fffff));
		MPT_TEST_EXPECT_EQUAL(std::memcmp(&le24, "\x01\x00\x80", 3), 0);
		MPT_TEST_EXPECT_EQUAL(std::memcmp(&be24, "\x80\x00\x01", 3), 0);
	}
	{
		int24le le24;
		le24.set(int24(-0x000001));
		int24be be24;
		be24.set(int24(-0x000001));
		MPT_TEST_EXPECT_EQUAL(le24, int24(-0x000001));
		MPT_TEST_EXPECT_EQUAL(be24, int24(-0x000001));
		MPT_TEST_EXPECT_EQUAL(std::memcmp(&le24, "\xff\xff\xff", 3), 0);
		MPT_TEST_EXPECT_EQUAL(std::memcmp(&be24, "\xff\xff\xff", 3), 0);
	}
	{
		int24le le24;
		le24.set(int24(0x7fffff));
		int24be be24;
		be24.set(int24(0x7fffff));
		MPT_TEST_EXPECT_EQUAL(le24, int24(0x7fffff));
		MPT_TEST_EXPECT_EQUAL(be24, int24(0x7fffff));
		MPT_TEST_EXPECT_EQUAL(std::memcmp(&le24, "\xff\xff\x7f", 3), 0);
		MPT_TEST_EXPECT_EQUAL(std::memcmp(&be24, "\x7f\xff\xff", 3), 0);
	}
	{
		int24le le24;
		le24.set(int24(-0x800000));
		int24be be24;
		be24.set(int24(-0x800000));
		MPT_TEST_EXPECT_EQUAL(le24, int24(-0x800000));
		MPT_TEST_EXPECT_EQUAL(be24, int24(-0x800000));
		MPT_TEST_EXPECT_EQUAL(std::memcmp(&le24, "\x00\x00\x80", 3), 0);
		MPT_TEST_EXPECT_EQUAL(std::memcmp(&be24, "\x80\x00\x00", 3), 0);
	}

	{
		uint24le le24;
		le24.set(uint24(0x123456));
		uint24be be24;
		be24.set(uint24(0x123456));
		MPT_TEST_EXPECT_EQUAL(le24, uint24(0x123456));
		MPT_TEST_EXPECT_EQUAL(be24, uint24(0x123456));
		MPT_TEST_EXPECT_EQUAL(std::memcmp(&le24, "\x56\x34\x12", 3), 0);
		MPT_TEST_EXPECT_EQUAL(std::memcmp(&be24, "\x12\x34\x56", 3), 0);
	}
	{
		uint24le le24;
		le24.set(uint24(0xffffff));
		uint24be be24;
		be24.set(uint24(0xffffff));
		MPT_TEST_EXPECT_EQUAL(le24, uint24(0xffffff));
		MPT_TEST_EXPECT_EQUAL(be24, uint24(0xffffff));
		MPT_TEST_EXPECT_EQUAL(std::memcmp(&le24, "\xff\xff\xff", 3), 0);
		MPT_TEST_EXPECT_EQUAL(std::memcmp(&be24, "\xff\xff\xff", 3), 0);
	}
	{
		uint24le le24;
		le24.set(uint24(0x7fffff));
		uint24be be24;
		be24.set(uint24(0x7fffff));
		MPT_TEST_EXPECT_EQUAL(le24, uint24(0x7fffff));
		MPT_TEST_EXPECT_EQUAL(be24, uint24(0x7fffff));
		MPT_TEST_EXPECT_EQUAL(std::memcmp(&le24, "\xff\xff\x7f", 3), 0);
		MPT_TEST_EXPECT_EQUAL(std::memcmp(&be24, "\x7f\xff\xff", 3), 0);
	}
	{
		uint24le le24;
		le24.set(uint24(0x800000));
		uint24be be24;
		be24.set(uint24(0x800000));
		MPT_TEST_EXPECT_EQUAL(le24, uint24(0x800000));
		MPT_TEST_EXPECT_EQUAL(be24, uint24(0x800000));
		MPT_TEST_EXPECT_EQUAL(std::memcmp(&le24, "\x00\x00\x80", 3), 0);
		MPT_TEST_EXPECT_EQUAL(std::memcmp(&be24, "\x80\x00\x00", 3), 0);
	}
}

} // namespace integer
} // namespace endian
} // namespace tests



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_ENDIAN_TESTS_ENDIAN_INT24_HPP
