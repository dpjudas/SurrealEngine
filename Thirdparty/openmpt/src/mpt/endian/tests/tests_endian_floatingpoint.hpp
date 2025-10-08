/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_ENDIAN_TESTS_ENDIAN_FLOATINGPOINT_HPP
#define MPT_ENDIAN_TESTS_ENDIAN_FLOATINGPOINT_HPP



#include "mpt/base/detect_compiler.hpp"
#include "mpt/base/memory.hpp"
#include "mpt/base/namespace.hpp"
#include "mpt/endian/floatingpoint.hpp"
#include "mpt/test/test.hpp"
#include "mpt/test/test_macros.hpp"



namespace mpt {
inline namespace MPT_INLINE_NS {



namespace tests {
namespace endian {
namespace floatingpoint {

#if MPT_COMPILER_CLANG
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wglobal-constructors"
#endif
MPT_TEST_GROUP_INLINE("mpt/endian/floatingpoint")
#if MPT_COMPILER_CLANG
#pragma clang diagnostic pop
#endif
{
	MPT_TEST_EXPECT_EQUAL(mpt::EncodeIEEE754binary32(1.0f), 0x3f800000u);
	MPT_TEST_EXPECT_EQUAL(mpt::EncodeIEEE754binary32(-1.0f), 0xbf800000u);
	MPT_TEST_EXPECT_EQUAL(mpt::DecodeIEEE754binary32(0x00000000u), 0.0f);
	MPT_TEST_EXPECT_EQUAL(mpt::DecodeIEEE754binary32(0x41840000u), 16.5f);
	MPT_TEST_EXPECT_EQUAL(mpt::DecodeIEEE754binary32(0x3faa0000u), 1.328125f);
	MPT_TEST_EXPECT_EQUAL(mpt::DecodeIEEE754binary32(0xbfaa0000u), -1.328125f);
	MPT_TEST_EXPECT_EQUAL(mpt::DecodeIEEE754binary32(0x3f800000u), 1.0f);
	MPT_TEST_EXPECT_EQUAL(mpt::DecodeIEEE754binary32(0x00000000u), 0.0f);
	MPT_TEST_EXPECT_EQUAL(mpt::DecodeIEEE754binary32(0xbf800000u), -1.0f);
	MPT_TEST_EXPECT_EQUAL(mpt::DecodeIEEE754binary32(0x3f800000u), 1.0f);
	MPT_TEST_EXPECT_EQUAL(IEEE754binary32LE(1.0f).GetInt32(), 0x3f800000u);
	MPT_TEST_EXPECT_EQUAL(IEEE754binary32BE(1.0f).GetInt32(), 0x3f800000u);
	MPT_TEST_EXPECT_EQUAL(IEEE754binary32LE(mpt::as_byte(0x00), mpt::as_byte(0x00), mpt::as_byte(0x80), mpt::as_byte(0x3f)), 1.0f);
	MPT_TEST_EXPECT_EQUAL(IEEE754binary32BE(mpt::as_byte(0x3f), mpt::as_byte(0x80), mpt::as_byte(0x00), mpt::as_byte(0x00)), 1.0f);
	MPT_TEST_EXPECT_EQUAL(IEEE754binary32LE(1.0f), IEEE754binary32LE(mpt::as_byte(0x00), mpt::as_byte(0x00), mpt::as_byte(0x80), mpt::as_byte(0x3f)));
	MPT_TEST_EXPECT_EQUAL(IEEE754binary32BE(1.0f), IEEE754binary32BE(mpt::as_byte(0x3f), mpt::as_byte(0x80), mpt::as_byte(0x00), mpt::as_byte(0x00)));

	MPT_TEST_EXPECT_EQUAL(mpt::EncodeIEEE754binary64(1.0), 0x3ff0000000000000ull);
	MPT_TEST_EXPECT_EQUAL(mpt::EncodeIEEE754binary64(-1.0), 0xbff0000000000000ull);
	MPT_TEST_EXPECT_EQUAL(mpt::DecodeIEEE754binary64(0x0000000000000000ull), 0.0);
	MPT_TEST_EXPECT_EQUAL(mpt::DecodeIEEE754binary64(0x4030800000000000ull), 16.5);
	MPT_TEST_EXPECT_EQUAL(mpt::DecodeIEEE754binary64(0x3FF5400000000000ull), 1.328125);
	MPT_TEST_EXPECT_EQUAL(mpt::DecodeIEEE754binary64(0xBFF5400000000000ull), -1.328125);
	MPT_TEST_EXPECT_EQUAL(mpt::DecodeIEEE754binary64(0x3ff0000000000000ull), 1.0);
	MPT_TEST_EXPECT_EQUAL(mpt::DecodeIEEE754binary64(0x0000000000000000ull), 0.0);
	MPT_TEST_EXPECT_EQUAL(mpt::DecodeIEEE754binary64(0xbff0000000000000ull), -1.0);
	MPT_TEST_EXPECT_EQUAL(mpt::DecodeIEEE754binary64(0x3ff0000000000000ull), 1.0);
	MPT_TEST_EXPECT_EQUAL(IEEE754binary64LE(1.0).GetInt64(), 0x3ff0000000000000ull);
	MPT_TEST_EXPECT_EQUAL(IEEE754binary64BE(1.0).GetInt64(), 0x3ff0000000000000ull);
	MPT_TEST_EXPECT_EQUAL(IEEE754binary64LE(mpt::as_byte(0x00), mpt::as_byte(0x00), mpt::as_byte(0x00), mpt::as_byte(0x00), mpt::as_byte(0x00), mpt::as_byte(0x00), mpt::as_byte(0xf0), mpt::as_byte(0x3f)), 1.0);
	MPT_TEST_EXPECT_EQUAL(IEEE754binary64BE(mpt::as_byte(0x3f), mpt::as_byte(0xf0), mpt::as_byte(0x00), mpt::as_byte(0x00), mpt::as_byte(0x00), mpt::as_byte(0x00), mpt::as_byte(0x00), mpt::as_byte(0x00)), 1.0);
	MPT_TEST_EXPECT_EQUAL(IEEE754binary64LE(1.0), IEEE754binary64LE(mpt::as_byte(0x00), mpt::as_byte(0x00), mpt::as_byte(0x00), mpt::as_byte(0x00), mpt::as_byte(0x00), mpt::as_byte(0x00), mpt::as_byte(0xf0), mpt::as_byte(0x3f)));
	MPT_TEST_EXPECT_EQUAL(IEEE754binary64BE(1.0), IEEE754binary64BE(mpt::as_byte(0x3f), mpt::as_byte(0xf0), mpt::as_byte(0x00), mpt::as_byte(0x00), mpt::as_byte(0x00), mpt::as_byte(0x00), mpt::as_byte(0x00), mpt::as_byte(0x00)));
}

} // namespace floatingpoint
} // namespace endian
} // namespace tests



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_ENDIAN_TESTS_ENDIAN_FLOATINGPOINT_HPP
