/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_BASE_TESTS_BASE_BIT_HPP
#define MPT_BASE_TESTS_BASE_BIT_HPP



#include "mpt/base/detect.hpp"
#include "mpt/base/integer.hpp"
#include "mpt/base/macros.hpp"
#include "mpt/base/namespace.hpp"
#include "mpt/test/test.hpp"
#include "mpt/test/test_macros.hpp"

#include <limits>



namespace mpt {
inline namespace MPT_INLINE_NS {



namespace tests {
namespace base {
namespace bit {

#if MPT_COMPILER_CLANG
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wglobal-constructors"
#endif
MPT_TEST_GROUP_INLINE("mpt/base/bit")
#if MPT_COMPILER_CLANG
#pragma clang diagnostic pop
#endif
{
#if MPT_CXX_BEFORE(20)
	MPT_TEST_EXPECT_EQUAL(mpt::get_endian(), mpt::endian_probe());
#endif
	MPT_MAYBE_CONSTANT_IF (mpt::endian_is_little()) {
		MPT_TEST_EXPECT_EQUAL(mpt::get_endian(), mpt::endian::little);
		MPT_MAYBE_CONSTANT_IF ((mpt::endian::native == mpt::endian::little) || (mpt::endian::native == mpt::endian::big)) {
			MPT_TEST_EXPECT_EQUAL(mpt::endian::native, mpt::endian::little);
		}
#if MPT_CXX_BEFORE(20)
		MPT_TEST_EXPECT_EQUAL(mpt::endian_probe(), mpt::endian::little);
#endif
	}
	MPT_MAYBE_CONSTANT_IF (mpt::endian_is_big()) {
		MPT_TEST_EXPECT_EQUAL(mpt::get_endian(), mpt::endian::big);
		MPT_MAYBE_CONSTANT_IF ((mpt::endian::native == mpt::endian::little) || (mpt::endian::native == mpt::endian::big)) {
			MPT_TEST_EXPECT_EQUAL(mpt::endian::native, mpt::endian::big);
		}
#if MPT_CXX_BEFORE(20)
		MPT_TEST_EXPECT_EQUAL(mpt::endian_probe(), mpt::endian::big);
#endif
	}

	MPT_TEST_EXPECT_EQUAL(mpt::popcount(static_cast<uint32>(int32(-1))), 32);
	MPT_TEST_EXPECT_EQUAL(mpt::popcount(0u), 0);
	MPT_TEST_EXPECT_EQUAL(mpt::popcount(1u), 1);
	MPT_TEST_EXPECT_EQUAL(mpt::popcount(2u), 1);
	MPT_TEST_EXPECT_EQUAL(mpt::popcount(3u), 2);

	MPT_TEST_EXPECT_EQUAL(mpt::has_single_bit(0u), false);
	MPT_TEST_EXPECT_EQUAL(mpt::has_single_bit(1u), true);
	MPT_TEST_EXPECT_EQUAL(mpt::has_single_bit(2u), true);
	MPT_TEST_EXPECT_EQUAL(mpt::has_single_bit(3u), false);
	MPT_TEST_EXPECT_EQUAL(mpt::has_single_bit(4u), true);
	MPT_TEST_EXPECT_EQUAL(mpt::has_single_bit(5u), false);
	MPT_TEST_EXPECT_EQUAL(mpt::has_single_bit(6u), false);
	MPT_TEST_EXPECT_EQUAL(mpt::has_single_bit(7u), false);
	MPT_TEST_EXPECT_EQUAL(mpt::has_single_bit(8u), true);
	MPT_TEST_EXPECT_EQUAL(mpt::has_single_bit(9u), false);
	MPT_TEST_EXPECT_EQUAL(mpt::has_single_bit(uint32(0x7fffffffu)), false);
	MPT_TEST_EXPECT_EQUAL(mpt::has_single_bit(uint32(0x80000000u)), true);
	MPT_TEST_EXPECT_EQUAL(mpt::has_single_bit(uint32(0x80000001u)), false);
	MPT_TEST_EXPECT_EQUAL(mpt::has_single_bit(uint32(0xfffffffeu)), false);
	MPT_TEST_EXPECT_EQUAL(mpt::has_single_bit(uint32(0xffffffffu)), false);

	MPT_TEST_EXPECT_EQUAL(mpt::bit_ceil(0u), 1u);
	MPT_TEST_EXPECT_EQUAL(mpt::bit_ceil(1u), 1u);
	MPT_TEST_EXPECT_EQUAL(mpt::bit_ceil(2u), 2u);
	MPT_TEST_EXPECT_EQUAL(mpt::bit_ceil(3u), 4u);
	MPT_TEST_EXPECT_EQUAL(mpt::bit_ceil(4u), 4u);
	MPT_TEST_EXPECT_EQUAL(mpt::bit_ceil(5u), 8u);
	MPT_TEST_EXPECT_EQUAL(mpt::bit_ceil(6u), 8u);
	MPT_TEST_EXPECT_EQUAL(mpt::bit_ceil(7u), 8u);
	MPT_TEST_EXPECT_EQUAL(mpt::bit_ceil(8u), 8u);
	MPT_TEST_EXPECT_EQUAL(mpt::bit_ceil(9u), 16u);
	MPT_TEST_EXPECT_EQUAL(mpt::bit_ceil(uint32(0x7fffffffu)), 0x80000000u);
	MPT_TEST_EXPECT_EQUAL(mpt::bit_ceil(uint32(0x80000000u)), 0x80000000u);
	//MPT_TEST_EXPECT_EQUAL(mpt::bit_ceil(uint32(0x80000001u)), 0u);
	//MPT_TEST_EXPECT_EQUAL(mpt::bit_ceil(uint32(0xfffffffeu)), 0u);
	//MPT_TEST_EXPECT_EQUAL(mpt::bit_ceil(uint32(0xffffffffu)), 0u);

	MPT_TEST_EXPECT_EQUAL(mpt::bit_floor(0u), 0u);
	MPT_TEST_EXPECT_EQUAL(mpt::bit_floor(1u), 1u);
	MPT_TEST_EXPECT_EQUAL(mpt::bit_floor(2u), 2u);
	MPT_TEST_EXPECT_EQUAL(mpt::bit_floor(3u), 2u);
	MPT_TEST_EXPECT_EQUAL(mpt::bit_floor(4u), 4u);
	MPT_TEST_EXPECT_EQUAL(mpt::bit_floor(5u), 4u);
	MPT_TEST_EXPECT_EQUAL(mpt::bit_floor(6u), 4u);
	MPT_TEST_EXPECT_EQUAL(mpt::bit_floor(7u), 4u);
	MPT_TEST_EXPECT_EQUAL(mpt::bit_floor(8u), 8u);
	MPT_TEST_EXPECT_EQUAL(mpt::bit_floor(9u), 8u);
	MPT_TEST_EXPECT_EQUAL(mpt::bit_floor(uint32(0x7fffffffu)), 0x40000000u);
	MPT_TEST_EXPECT_EQUAL(mpt::bit_floor(uint32(0x80000000u)), 0x80000000u);
	MPT_TEST_EXPECT_EQUAL(mpt::bit_floor(uint32(0x80000001u)), 0x80000000u);
	MPT_TEST_EXPECT_EQUAL(mpt::bit_floor(uint32(0xfffffffeu)), 0x80000000u);
	MPT_TEST_EXPECT_EQUAL(mpt::bit_floor(uint32(0xffffffffu)), 0x80000000u);

	MPT_TEST_EXPECT_EQUAL(mpt::bit_width(0u), 0);
	MPT_TEST_EXPECT_EQUAL(mpt::bit_width(1u), 1);
	MPT_TEST_EXPECT_EQUAL(mpt::bit_width(2u), 2);
	MPT_TEST_EXPECT_EQUAL(mpt::bit_width(3u), 2);
	MPT_TEST_EXPECT_EQUAL(mpt::bit_width(4u), 3);
	MPT_TEST_EXPECT_EQUAL(mpt::bit_width(5u), 3);
	MPT_TEST_EXPECT_EQUAL(mpt::bit_width(6u), 3);
	MPT_TEST_EXPECT_EQUAL(mpt::bit_width(7u), 3);
	MPT_TEST_EXPECT_EQUAL(mpt::bit_width(8u), 4);
	MPT_TEST_EXPECT_EQUAL(mpt::bit_width(9u), 4);
	MPT_TEST_EXPECT_EQUAL(mpt::bit_width(uint32(0x7fffffffu)), 31);
	MPT_TEST_EXPECT_EQUAL(mpt::bit_width(uint32(0x80000000u)), 32);
	MPT_TEST_EXPECT_EQUAL(mpt::bit_width(uint32(0x80000001u)), 32);
	MPT_TEST_EXPECT_EQUAL(mpt::bit_width(uint32(0xfffffffeu)), 32);
	MPT_TEST_EXPECT_EQUAL(mpt::bit_width(uint32(0xffffffffu)), 32);

	MPT_TEST_EXPECT_EQUAL(mpt::countl_one(uint8(0b00000000)), 0);
	MPT_TEST_EXPECT_EQUAL(mpt::countl_one(uint8(0b00000001)), 0);
	MPT_TEST_EXPECT_EQUAL(mpt::countl_one(uint8(0b00000011)), 0);
	MPT_TEST_EXPECT_EQUAL(mpt::countl_one(uint8(0b00000111)), 0);
	MPT_TEST_EXPECT_EQUAL(mpt::countl_one(uint8(0b00001111)), 0);
	MPT_TEST_EXPECT_EQUAL(mpt::countl_one(uint8(0b00011111)), 0);
	MPT_TEST_EXPECT_EQUAL(mpt::countl_one(uint8(0b00111111)), 0);
	MPT_TEST_EXPECT_EQUAL(mpt::countl_one(uint8(0b01111111)), 0);
	MPT_TEST_EXPECT_EQUAL(mpt::countl_one(uint8(0b11111111)), 8);
	MPT_TEST_EXPECT_EQUAL(mpt::countl_one(uint8(0b11111110)), 7);
	MPT_TEST_EXPECT_EQUAL(mpt::countl_one(uint8(0b11111100)), 6);
	MPT_TEST_EXPECT_EQUAL(mpt::countl_one(uint8(0b11111000)), 5);
	MPT_TEST_EXPECT_EQUAL(mpt::countl_one(uint8(0b11110000)), 4);
	MPT_TEST_EXPECT_EQUAL(mpt::countl_one(uint8(0b11100000)), 3);
	MPT_TEST_EXPECT_EQUAL(mpt::countl_one(uint8(0b11000000)), 2);
	MPT_TEST_EXPECT_EQUAL(mpt::countl_one(uint8(0b10000000)), 1);
	MPT_TEST_EXPECT_EQUAL(mpt::countl_one(uint8(0b00000000)), 0);

	MPT_TEST_EXPECT_EQUAL(mpt::countl_zero(uint8(0b00000000)), 8);
	MPT_TEST_EXPECT_EQUAL(mpt::countl_zero(uint8(0b00000001)), 7);
	MPT_TEST_EXPECT_EQUAL(mpt::countl_zero(uint8(0b00000011)), 6);
	MPT_TEST_EXPECT_EQUAL(mpt::countl_zero(uint8(0b00000111)), 5);
	MPT_TEST_EXPECT_EQUAL(mpt::countl_zero(uint8(0b00001111)), 4);
	MPT_TEST_EXPECT_EQUAL(mpt::countl_zero(uint8(0b00011111)), 3);
	MPT_TEST_EXPECT_EQUAL(mpt::countl_zero(uint8(0b00111111)), 2);
	MPT_TEST_EXPECT_EQUAL(mpt::countl_zero(uint8(0b01111111)), 1);
	MPT_TEST_EXPECT_EQUAL(mpt::countl_zero(uint8(0b11111111)), 0);
	MPT_TEST_EXPECT_EQUAL(mpt::countl_zero(uint8(0b11111110)), 0);
	MPT_TEST_EXPECT_EQUAL(mpt::countl_zero(uint8(0b11111100)), 0);
	MPT_TEST_EXPECT_EQUAL(mpt::countl_zero(uint8(0b11111000)), 0);
	MPT_TEST_EXPECT_EQUAL(mpt::countl_zero(uint8(0b11110000)), 0);
	MPT_TEST_EXPECT_EQUAL(mpt::countl_zero(uint8(0b11100000)), 0);
	MPT_TEST_EXPECT_EQUAL(mpt::countl_zero(uint8(0b11000000)), 0);
	MPT_TEST_EXPECT_EQUAL(mpt::countl_zero(uint8(0b10000000)), 0);
	MPT_TEST_EXPECT_EQUAL(mpt::countl_zero(uint8(0b00000000)), 8);

	MPT_TEST_EXPECT_EQUAL(mpt::countr_one(uint8(0b00000000)), 0);
	MPT_TEST_EXPECT_EQUAL(mpt::countr_one(uint8(0b00000001)), 1);
	MPT_TEST_EXPECT_EQUAL(mpt::countr_one(uint8(0b00000011)), 2);
	MPT_TEST_EXPECT_EQUAL(mpt::countr_one(uint8(0b00000111)), 3);
	MPT_TEST_EXPECT_EQUAL(mpt::countr_one(uint8(0b00001111)), 4);
	MPT_TEST_EXPECT_EQUAL(mpt::countr_one(uint8(0b00011111)), 5);
	MPT_TEST_EXPECT_EQUAL(mpt::countr_one(uint8(0b00111111)), 6);
	MPT_TEST_EXPECT_EQUAL(mpt::countr_one(uint8(0b01111111)), 7);
	MPT_TEST_EXPECT_EQUAL(mpt::countr_one(uint8(0b11111111)), 8);
	MPT_TEST_EXPECT_EQUAL(mpt::countr_one(uint8(0b11111110)), 0);
	MPT_TEST_EXPECT_EQUAL(mpt::countr_one(uint8(0b11111100)), 0);
	MPT_TEST_EXPECT_EQUAL(mpt::countr_one(uint8(0b11111000)), 0);
	MPT_TEST_EXPECT_EQUAL(mpt::countr_one(uint8(0b11110000)), 0);
	MPT_TEST_EXPECT_EQUAL(mpt::countr_one(uint8(0b11100000)), 0);
	MPT_TEST_EXPECT_EQUAL(mpt::countr_one(uint8(0b11000000)), 0);
	MPT_TEST_EXPECT_EQUAL(mpt::countr_one(uint8(0b10000000)), 0);
	MPT_TEST_EXPECT_EQUAL(mpt::countr_one(uint8(0b00000000)), 0);

	MPT_TEST_EXPECT_EQUAL(mpt::countr_zero(uint8(0b00000000)), 8);
	MPT_TEST_EXPECT_EQUAL(mpt::countr_zero(uint8(0b00000001)), 0);
	MPT_TEST_EXPECT_EQUAL(mpt::countr_zero(uint8(0b00000011)), 0);
	MPT_TEST_EXPECT_EQUAL(mpt::countr_zero(uint8(0b00000111)), 0);
	MPT_TEST_EXPECT_EQUAL(mpt::countr_zero(uint8(0b00001111)), 0);
	MPT_TEST_EXPECT_EQUAL(mpt::countr_zero(uint8(0b00011111)), 0);
	MPT_TEST_EXPECT_EQUAL(mpt::countr_zero(uint8(0b00111111)), 0);
	MPT_TEST_EXPECT_EQUAL(mpt::countr_zero(uint8(0b01111111)), 0);
	MPT_TEST_EXPECT_EQUAL(mpt::countr_zero(uint8(0b11111111)), 0);
	MPT_TEST_EXPECT_EQUAL(mpt::countr_zero(uint8(0b11111110)), 1);
	MPT_TEST_EXPECT_EQUAL(mpt::countr_zero(uint8(0b11111100)), 2);
	MPT_TEST_EXPECT_EQUAL(mpt::countr_zero(uint8(0b11111000)), 3);
	MPT_TEST_EXPECT_EQUAL(mpt::countr_zero(uint8(0b11110000)), 4);
	MPT_TEST_EXPECT_EQUAL(mpt::countr_zero(uint8(0b11100000)), 5);
	MPT_TEST_EXPECT_EQUAL(mpt::countr_zero(uint8(0b11000000)), 6);
	MPT_TEST_EXPECT_EQUAL(mpt::countr_zero(uint8(0b10000000)), 7);
	MPT_TEST_EXPECT_EQUAL(mpt::countr_zero(uint8(0b00000000)), 8);

	MPT_TEST_EXPECT_EQUAL(mpt::lower_bound_entropy_bits(0xffffffffu), 32);
	MPT_TEST_EXPECT_EQUAL(mpt::lower_bound_entropy_bits(0xfffffffeu), 31);

	MPT_TEST_EXPECT_EQUAL(mpt::lower_bound_entropy_bits(0x80000000u), 31);
	MPT_TEST_EXPECT_EQUAL(mpt::lower_bound_entropy_bits(0x7fffffffu), 31);
	MPT_TEST_EXPECT_EQUAL(mpt::lower_bound_entropy_bits(0x7ffffffeu), 30);

	MPT_TEST_EXPECT_EQUAL(mpt::lower_bound_entropy_bits(0x00000007u), 3);
	MPT_TEST_EXPECT_EQUAL(mpt::lower_bound_entropy_bits(0x00000006u), 2);
	MPT_TEST_EXPECT_EQUAL(mpt::lower_bound_entropy_bits(0x00000005u), 2);
	MPT_TEST_EXPECT_EQUAL(mpt::lower_bound_entropy_bits(0x00000004u), 2);
	MPT_TEST_EXPECT_EQUAL(mpt::lower_bound_entropy_bits(0x00000003u), 2);
	MPT_TEST_EXPECT_EQUAL(mpt::lower_bound_entropy_bits(0x00000002u), 1);
	MPT_TEST_EXPECT_EQUAL(mpt::lower_bound_entropy_bits(0x00000001u), 1);
	MPT_TEST_EXPECT_EQUAL(mpt::lower_bound_entropy_bits(0x00000000u), 0);

	MPT_TEST_EXPECT_EQUAL(mpt::byteswap(uint8(0x12)), 0x12);
	MPT_TEST_EXPECT_EQUAL(mpt::byteswap(uint16(0x1234)), 0x3412);
	MPT_TEST_EXPECT_EQUAL(mpt::byteswap(uint32(0x12345678u)), 0x78563412u);
	MPT_TEST_EXPECT_EQUAL(mpt::byteswap(uint64(0x123456789abcdef0ull)), 0xf0debc9a78563412ull);

	MPT_TEST_EXPECT_EQUAL(mpt::byteswap(int8(std::numeric_limits<int8>::min())), std::numeric_limits<int8>::min());
	MPT_TEST_EXPECT_EQUAL(mpt::byteswap(int16(std::numeric_limits<int16>::min())), int16(0x80));
	MPT_TEST_EXPECT_EQUAL(mpt::byteswap(int32(std::numeric_limits<int32>::min())), int32(0x80));
	MPT_TEST_EXPECT_EQUAL(mpt::byteswap(int64(std::numeric_limits<int64>::min())), int64(0x80));
}

} // namespace bit
} // namespace base
} // namespace tests



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_BASE_TESTS_BASE_BIT_HPP
