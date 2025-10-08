/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_BASE_TESTS_MATH_HPP
#define MPT_BASE_TESTS_MATH_HPP



#include "mpt/base/detect_compiler.hpp"
#include "mpt/base/math.hpp"
#include "mpt/base/namespace.hpp"
#include "mpt/test/test.hpp"
#include "mpt/test/test_macros.hpp"

#include <cmath>



namespace mpt {
inline namespace MPT_INLINE_NS {



namespace tests {
namespace base {
namespace math {

#if MPT_COMPILER_CLANG
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wglobal-constructors"
#endif
MPT_TEST_GROUP_INLINE("mpt/base/math")
#if MPT_COMPILER_CLANG
#pragma clang diagnostic pop
#endif
{
	MPT_TEST_EXPECT_EQUAL(mpt::round(1.99f), 2.0f);
	MPT_TEST_EXPECT_EQUAL(mpt::round(1.5f), 2.0f);
	MPT_TEST_EXPECT_EQUAL(mpt::round(1.1f), 1.0f);
	MPT_TEST_EXPECT_EQUAL(mpt::round(-0.1f), 0.0f);
	MPT_TEST_EXPECT_EQUAL(mpt::round(-0.5f), -1.0f);
	MPT_TEST_EXPECT_EQUAL(mpt::round(-0.9f), -1.0f);
	MPT_TEST_EXPECT_EQUAL(mpt::round(-1.4f), -1.0f);
	MPT_TEST_EXPECT_EQUAL(mpt::round(-1.7f), -2.0f);
	MPT_TEST_EXPECT_EQUAL(mpt::round(1.99), 2.0);
	MPT_TEST_EXPECT_EQUAL(mpt::round(1.5), 2.0);
	MPT_TEST_EXPECT_EQUAL(mpt::round(1.1), 1.0);
	MPT_TEST_EXPECT_EQUAL(mpt::round(-0.1), 0.0);
	MPT_TEST_EXPECT_EQUAL(mpt::round(-0.5), -1.0);
	MPT_TEST_EXPECT_EQUAL(mpt::round(-0.9), -1.0);
	MPT_TEST_EXPECT_EQUAL(mpt::round(-1.4), -1.0);
	MPT_TEST_EXPECT_EQUAL(mpt::round(-1.7), -2.0);
	MPT_TEST_EXPECT_EQUAL(mpt::round(1.99l), 2.0l);
	MPT_TEST_EXPECT_EQUAL(mpt::round(1.5l), 2.0l);
	MPT_TEST_EXPECT_EQUAL(mpt::round(1.1l), 1.0l);
	MPT_TEST_EXPECT_EQUAL(mpt::round(-0.1l), 0.0l);
	MPT_TEST_EXPECT_EQUAL(mpt::round(-0.5l), -1.0l);
	MPT_TEST_EXPECT_EQUAL(mpt::round(-0.9l), -1.0l);
	MPT_TEST_EXPECT_EQUAL(mpt::round(-1.4l), -1.0l);
	MPT_TEST_EXPECT_EQUAL(mpt::round(-1.7l), -2.0l);

	MPT_TEST_EXPECT_EQUAL(std::ceil(1.99f), 2.0f);
	MPT_TEST_EXPECT_EQUAL(std::ceil(1.5f), 2.0f);
	MPT_TEST_EXPECT_EQUAL(std::ceil(1.1f), 2.0f);
	MPT_TEST_EXPECT_EQUAL(std::ceil(-0.1f), 0.0f);
	MPT_TEST_EXPECT_EQUAL(std::ceil(-0.5f), 0.0f);
	MPT_TEST_EXPECT_EQUAL(std::ceil(-0.9f), 0.0f);
	MPT_TEST_EXPECT_EQUAL(std::ceil(-1.4f), -1.0f);
	MPT_TEST_EXPECT_EQUAL(std::ceil(-1.7f), -1.0f);
	MPT_TEST_EXPECT_EQUAL(std::ceil(1.99), 2.0);
	MPT_TEST_EXPECT_EQUAL(std::ceil(1.5), 2.0);
	MPT_TEST_EXPECT_EQUAL(std::ceil(1.1), 2.0);
	MPT_TEST_EXPECT_EQUAL(std::ceil(-0.1), 0.0);
	MPT_TEST_EXPECT_EQUAL(std::ceil(-0.5), 0.0);
	MPT_TEST_EXPECT_EQUAL(std::ceil(-0.9), 0.0);
	MPT_TEST_EXPECT_EQUAL(std::ceil(-1.4), -1.0);
	MPT_TEST_EXPECT_EQUAL(std::ceil(-1.7), -1.0);
	MPT_TEST_EXPECT_EQUAL(std::ceil(1.99l), 2.0l);
	MPT_TEST_EXPECT_EQUAL(std::ceil(1.5l), 2.0l);
	MPT_TEST_EXPECT_EQUAL(std::ceil(1.1l), 2.0l);
	MPT_TEST_EXPECT_EQUAL(std::ceil(-0.1l), 0.0l);
	MPT_TEST_EXPECT_EQUAL(std::ceil(-0.5l), 0.0l);
	MPT_TEST_EXPECT_EQUAL(std::ceil(-0.9l), 0.0l);
	MPT_TEST_EXPECT_EQUAL(std::ceil(-1.4l), -1.0l);
	MPT_TEST_EXPECT_EQUAL(std::ceil(-1.7l), -1.0l);

	MPT_TEST_EXPECT_EQUAL(std::floor(1.99f), 1.0f);
	MPT_TEST_EXPECT_EQUAL(std::floor(1.5f), 1.0f);
	MPT_TEST_EXPECT_EQUAL(std::floor(1.1f), 1.0f);
	MPT_TEST_EXPECT_EQUAL(std::floor(-0.1f), -1.0f);
	MPT_TEST_EXPECT_EQUAL(std::floor(-0.5f), -1.0f);
	MPT_TEST_EXPECT_EQUAL(std::floor(-0.9f), -1.0f);
	MPT_TEST_EXPECT_EQUAL(std::floor(-1.4f), -2.0f);
	MPT_TEST_EXPECT_EQUAL(std::floor(-1.7f), -2.0f);
	MPT_TEST_EXPECT_EQUAL(std::floor(1.99), 1.0);
	MPT_TEST_EXPECT_EQUAL(std::floor(1.5), 1.0);
	MPT_TEST_EXPECT_EQUAL(std::floor(1.1), 1.0);
	MPT_TEST_EXPECT_EQUAL(std::floor(-0.1), -1.0);
	MPT_TEST_EXPECT_EQUAL(std::floor(-0.5), -1.0);
	MPT_TEST_EXPECT_EQUAL(std::floor(-0.9), -1.0);
	MPT_TEST_EXPECT_EQUAL(std::floor(-1.4), -2.0);
	MPT_TEST_EXPECT_EQUAL(std::floor(-1.7), -2.0);
	MPT_TEST_EXPECT_EQUAL(std::floor(1.99l), 1.0l);
	MPT_TEST_EXPECT_EQUAL(std::floor(1.5l), 1.0l);
	MPT_TEST_EXPECT_EQUAL(std::floor(1.1l), 1.0l);
	MPT_TEST_EXPECT_EQUAL(std::floor(-0.1l), -1.0l);
	MPT_TEST_EXPECT_EQUAL(std::floor(-0.5l), -1.0l);
	MPT_TEST_EXPECT_EQUAL(std::floor(-0.9l), -1.0l);
	MPT_TEST_EXPECT_EQUAL(std::floor(-1.4l), -2.0l);
	MPT_TEST_EXPECT_EQUAL(std::floor(-1.7l), -2.0l);

	MPT_TEST_EXPECT_EQUAL(mpt::trunc(1.99f), 1.0f);
	MPT_TEST_EXPECT_EQUAL(mpt::trunc(1.5f), 1.0f);
	MPT_TEST_EXPECT_EQUAL(mpt::trunc(1.1f), 1.0f);
	MPT_TEST_EXPECT_EQUAL(mpt::trunc(-0.1f), 0.0f);
	MPT_TEST_EXPECT_EQUAL(mpt::trunc(-0.5f), 0.0f);
	MPT_TEST_EXPECT_EQUAL(mpt::trunc(-0.9f), 0.0f);
	MPT_TEST_EXPECT_EQUAL(mpt::trunc(-1.4f), -1.0f);
	MPT_TEST_EXPECT_EQUAL(mpt::trunc(-1.7f), -1.0f);
	MPT_TEST_EXPECT_EQUAL(mpt::trunc(1.99), 1.0);
	MPT_TEST_EXPECT_EQUAL(mpt::trunc(1.5), 1.0);
	MPT_TEST_EXPECT_EQUAL(mpt::trunc(1.1), 1.0);
	MPT_TEST_EXPECT_EQUAL(mpt::trunc(-0.1), 0.0);
	MPT_TEST_EXPECT_EQUAL(mpt::trunc(-0.5), 0.0);
	MPT_TEST_EXPECT_EQUAL(mpt::trunc(-0.9), 0.0);
	MPT_TEST_EXPECT_EQUAL(mpt::trunc(-1.4), -1.0);
	MPT_TEST_EXPECT_EQUAL(mpt::trunc(-1.7), -1.0);
	MPT_TEST_EXPECT_EQUAL(mpt::trunc(1.99l), 1.0l);
	MPT_TEST_EXPECT_EQUAL(mpt::trunc(1.5l), 1.0l);
	MPT_TEST_EXPECT_EQUAL(mpt::trunc(1.1l), 1.0l);
	MPT_TEST_EXPECT_EQUAL(mpt::trunc(-0.1l), 0.0l);
	MPT_TEST_EXPECT_EQUAL(mpt::trunc(-0.5l), 0.0l);
	MPT_TEST_EXPECT_EQUAL(mpt::trunc(-0.9l), 0.0l);
	MPT_TEST_EXPECT_EQUAL(mpt::trunc(-1.4l), -1.0l);
	MPT_TEST_EXPECT_EQUAL(mpt::trunc(-1.7l), -1.0l);
}

} // namespace math
} // namespace base
} // namespace tests



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_BASE_TESTS_MATH_HPP
