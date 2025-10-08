/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_BASE_TESTS_WRAPPING_DIVIDE_HPP
#define MPT_BASE_TESTS_WRAPPING_DIVIDE_HPP



#include "mpt/base/detect_compiler.hpp"
#include "mpt/base/integer.hpp"
#include "mpt/base/namespace.hpp"
#include "mpt/base/wrapping_divide.hpp"
#include "mpt/test/test.hpp"
#include "mpt/test/test_macros.hpp"

#include <limits>



namespace mpt {
inline namespace MPT_INLINE_NS {



namespace tests {
namespace base {
namespace wrapping_divide {

#if MPT_COMPILER_CLANG
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wglobal-constructors"
#endif
MPT_TEST_GROUP_INLINE("mpt/base/wrapping_divide")
#if MPT_COMPILER_CLANG
#pragma clang diagnostic pop
#endif
{
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_modulo(-25, 12), 11);
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_modulo(-24, 12), 0);
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_modulo(-23, 12), 1);
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_modulo(-8, 7), 6);
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_modulo(-7, 7), 0);
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_modulo(-6, 7), 1);
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_modulo(-5, 7), 2);
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_modulo(-4, 7), 3);
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_modulo(-3, 7), 4);
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_modulo(-2, 7), 5);
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_modulo(-1, 7), 6);
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_modulo(0, 12), 0);
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_modulo(0, 7), 0);
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_modulo(1, 7), 1);
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_modulo(2, 7), 2);
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_modulo(3, 7), 3);
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_modulo(4, 7), 4);
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_modulo(5, 7), 5);
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_modulo(6, 7), 6);
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_modulo(7, 7), 0);
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_modulo(8, 7), 1);
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_modulo(23, 12), 11);
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_modulo(24, 12), 0);
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_modulo(25, 12), 1);
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_modulo(uint32(0x7fffffff), uint32(0x80000000)), uint32(0x7fffffff));
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_modulo(int32(0x7ffffffe), int32(0x7fffffff)), int32(0x7ffffffe));

	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_modulo(int32(-0x80000000ll), int32(1)), int32(0));
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_modulo(int32(-0x80000000ll), int32(2)), int32(0));
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_modulo(int32(-0x7fffffff), int32(1)), int32(0));
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_modulo(int32(-0x7fffffff), int32(2)), int32(1));
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_modulo(int32(-0x7ffffffe), int32(1)), int32(0));
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_modulo(int32(-0x7ffffffe), int32(2)), int32(0));

	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_modulo(int32(-0x80000000ll), int32(0x7fffffff)), int32(0x7ffffffe));
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_modulo(int32(-0x7fffffff), int32(0x7fffffff)), int32(0));
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_modulo(int32(-0x7ffffffe), int32(0x7fffffff)), int32(1));

	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_modulo(int32(-0x80000000ll), int32(0x7ffffffe)), int32(0x7ffffffc));
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_modulo(int32(-0x7fffffff), int32(0x7ffffffe)), int32(0x7ffffffd));
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_modulo(int32(-0x7ffffffe), int32(0x7ffffffe)), int32(0));

	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_modulo(int32(-0x80000000ll), int32(0x7ffffffd)), int32(0x7ffffffa));
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_modulo(int32(-0x7fffffff), int32(0x7ffffffd)), int32(0x7ffffffb));
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_modulo(int32(-0x7ffffffe), int32(0x7ffffffd)), int32(0x7ffffffc));

	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_modulo(int32(0), int32(0x7fffffff)), int32(0));
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_modulo(int32(-1), int32(0x7fffffff)), int32(0x7ffffffe));
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_modulo(int32(-2), int32(0x7fffffff)), int32(0x7ffffffd));

	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_modulo(int32(0), int32(0x7ffffffe)), int32(0));
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_modulo(int32(-1), int32(0x7ffffffe)), int32(0x7ffffffd));
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_modulo(int32(-2), int32(0x7ffffffe)), int32(0x7ffffffc));

	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_modulo(int32(-0x80000000ll), uint32(1)), uint32(0));
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_modulo(int32(-0x80000000ll), uint32(2)), uint32(0));
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_modulo(int32(-0x7fffffff), uint32(1)), uint32(0));
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_modulo(int32(-0x7fffffff), uint32(2)), uint32(1));
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_modulo(int32(-0x7ffffffe), uint32(1)), uint32(0));
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_modulo(int32(-0x7ffffffe), uint32(2)), uint32(0));

	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_modulo(int32(-0x40000001), uint32(0xffffffff)), uint32(0xbffffffe));
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_modulo(int32(-0x40000000), uint32(0xffffffff)), uint32(0xbfffffff));
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_modulo(int32(-0x3fffffff), uint32(0xffffffff)), uint32(0xc0000000));

	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_modulo(int32(-0x80000000ll), uint32(0x80000000)), uint32(0));
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_modulo(int32(-0x7fffffff), uint32(0x80000000)), uint32(1));
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_modulo(int32(-0x7ffffffe), uint32(0x80000000)), uint32(2));

	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_modulo(int32(-0x80000000ll), uint32(0x80000001)), uint32(1));
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_modulo(int32(-0x7fffffff), uint32(0x80000001)), uint32(2));
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_modulo(int32(-0x7ffffffe), uint32(0x80000001)), uint32(3));

	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_modulo(int32(-0x80000000ll), uint32(0x80000000)), uint32(0));
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_modulo(int32(-0x7fffffff), uint32(0x80000000)), uint32(1));
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_modulo(int32(-0x7ffffffe), uint32(0x80000000)), uint32(2));

	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_modulo(int32(-0x80000000ll), uint32(0x7fffffff)), uint32(0x7ffffffe));
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_modulo(int32(-0x7fffffff), uint32(0x7fffffff)), uint32(0));
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_modulo(int32(-0x7ffffffe), uint32(0x7fffffff)), uint32(1));

	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_modulo(int32(-0x80000000ll), uint32(0x7ffffffe)), uint32(0x7ffffffc));
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_modulo(int32(-0x7fffffff), uint32(0x7ffffffe)), uint32(0x7ffffffd));
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_modulo(int32(-0x7ffffffe), uint32(0x7ffffffe)), uint32(0));

	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_modulo(int32(-0x80000000ll), uint32(0x7ffffffd)), uint32(0x7ffffffa));
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_modulo(int32(-0x7fffffff), uint32(0x7ffffffd)), uint32(0x7ffffffb));
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_modulo(int32(-0x7ffffffe), uint32(0x7ffffffd)), uint32(0x7ffffffc));

	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_modulo(int32(0), uint32(0x7fffffff)), uint32(0));
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_modulo(int32(-1), uint32(0x7fffffff)), uint32(0x7ffffffe));
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_modulo(int32(-2), uint32(0x7fffffff)), uint32(0x7ffffffd));

	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_modulo(int32(0), uint32(0x7ffffffe)), uint32(0));
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_modulo(int32(-1), uint32(0x7ffffffe)), uint32(0x7ffffffd));
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_modulo(int32(-2), uint32(0x7ffffffe)), uint32(0x7ffffffc));

	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_divide(-15, 7), -3);
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_divide(-14, 7), -2);
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_divide(-13, 7), -2);
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_divide(-12, 7), -2);
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_divide(-11, 7), -2);
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_divide(-10, 7), -2);
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_divide(-9, 7), -2);
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_divide(-8, 7), -2);
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_divide(-7, 7), -1);
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_divide(-6, 7), -1);
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_divide(-5, 7), -1);
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_divide(-4, 7), -1);
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_divide(-3, 7), -1);
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_divide(-2, 7), -1);
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_divide(-1, 7), -1);
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_divide(0, 7), 0);
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_divide(1, 7), 0);
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_divide(2, 7), 0);
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_divide(3, 7), 0);
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_divide(4, 7), 0);
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_divide(5, 7), 0);
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_divide(6, 7), 0);
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_divide(7, 7), 1);
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_divide(8, 7), 1);
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_divide(9, 7), 1);
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_divide(10, 7), 1);
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_divide(11, 7), 1);
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_divide(12, 7), 1);
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_divide(13, 7), 1);
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_divide(14, 7), 2);
	MPT_TEST_EXPECT_EQUAL(mpt::wrapping_divide(15, 7), 2);
}

} // namespace wrapping_divide
} // namespace base
} // namespace tests



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_BASE_TESTS_WRAPPING_DIVIDE_HPP
