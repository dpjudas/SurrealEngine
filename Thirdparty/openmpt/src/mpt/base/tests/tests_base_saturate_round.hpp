/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_BASE_TESTS_SATURATE_ROUND_HPP
#define MPT_BASE_TESTS_SATURATE_ROUND_HPP



#include "mpt/base/detect_compiler.hpp"
#include "mpt/base/integer.hpp"
#include "mpt/base/namespace.hpp"
#include "mpt/base/saturate_round.hpp"
#include "mpt/test/test.hpp"
#include "mpt/test/test_macros.hpp"

#include <limits>



namespace mpt {
inline namespace MPT_INLINE_NS {



namespace tests {
namespace base {
namespace saturate_round {

#if MPT_COMPILER_CLANG
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wglobal-constructors"
#endif
MPT_TEST_GROUP_INLINE("mpt/base/saturate_round")
#if MPT_COMPILER_CLANG
#pragma clang diagnostic pop
#endif
{
	MPT_TEST_EXPECT_EQUAL(mpt::saturate_trunc<uint32>(static_cast<double>(std::numeric_limits<int64>::max())), std::numeric_limits<uint32>::max());

	MPT_TEST_EXPECT_EQUAL(mpt::saturate_round<int32>(std::numeric_limits<int32>::max() + 0.1), std::numeric_limits<int32>::max());
	MPT_TEST_EXPECT_EQUAL(mpt::saturate_round<int32>(std::numeric_limits<int32>::max() - 0.4), std::numeric_limits<int32>::max());
	MPT_TEST_EXPECT_EQUAL(mpt::saturate_round<int32>(std::numeric_limits<int32>::min() + 0.1), std::numeric_limits<int32>::min());
	MPT_TEST_EXPECT_EQUAL(mpt::saturate_round<int32>(std::numeric_limits<int32>::min() - 0.1), std::numeric_limits<int32>::min());
	MPT_TEST_EXPECT_EQUAL(mpt::saturate_round<uint32>(std::numeric_limits<uint32>::max() + 0.499), std::numeric_limits<uint32>::max());
	MPT_TEST_EXPECT_EQUAL(mpt::saturate_round<int8>(110.1), 110);
	MPT_TEST_EXPECT_EQUAL(mpt::saturate_round<int8>(-110.1), -110);

	MPT_TEST_EXPECT_EQUAL(mpt::saturate_trunc<int8>(-0.6), 0);
	MPT_TEST_EXPECT_EQUAL(mpt::saturate_trunc<int8>(-0.5), 0);
	MPT_TEST_EXPECT_EQUAL(mpt::saturate_trunc<int8>(-0.4), 0);
	MPT_TEST_EXPECT_EQUAL(mpt::saturate_trunc<int8>(0.4), 0);
	MPT_TEST_EXPECT_EQUAL(mpt::saturate_trunc<int8>(0.5), 0);
	MPT_TEST_EXPECT_EQUAL(mpt::saturate_trunc<int8>(0.6), 0);

	MPT_TEST_EXPECT_EQUAL(mpt::saturate_round<int8>(-0.6), -1);
	MPT_TEST_EXPECT_EQUAL(mpt::saturate_round<int8>(-0.5), -1);
	MPT_TEST_EXPECT_EQUAL(mpt::saturate_round<int8>(-0.4), 0);
	MPT_TEST_EXPECT_EQUAL(mpt::saturate_round<int8>(0.4), 0);
	MPT_TEST_EXPECT_EQUAL(mpt::saturate_round<int8>(0.5), 1);
	MPT_TEST_EXPECT_EQUAL(mpt::saturate_round<int8>(0.6), 1);

	MPT_TEST_EXPECT_EQUAL(mpt::saturate_ceil<int8>(-0.6), 0);
	MPT_TEST_EXPECT_EQUAL(mpt::saturate_ceil<int8>(-0.5), 0);
	MPT_TEST_EXPECT_EQUAL(mpt::saturate_ceil<int8>(-0.4), 0);
	MPT_TEST_EXPECT_EQUAL(mpt::saturate_ceil<int8>(0.4), 1);
	MPT_TEST_EXPECT_EQUAL(mpt::saturate_ceil<int8>(0.5), 1);
	MPT_TEST_EXPECT_EQUAL(mpt::saturate_ceil<int8>(0.6), 1);

	MPT_TEST_EXPECT_EQUAL(mpt::saturate_floor<int8>(-0.6), -1);
	MPT_TEST_EXPECT_EQUAL(mpt::saturate_floor<int8>(-0.5), -1);
	MPT_TEST_EXPECT_EQUAL(mpt::saturate_floor<int8>(-0.4), -1);
	MPT_TEST_EXPECT_EQUAL(mpt::saturate_floor<int8>(0.4), 0);
	MPT_TEST_EXPECT_EQUAL(mpt::saturate_floor<int8>(0.5), 0);
	MPT_TEST_EXPECT_EQUAL(mpt::saturate_floor<int8>(0.6), 0);

	// These should fail to compile
	//mpt::saturate_round<std::string>(1.0);
	//mpt::saturate_round<int64>(1.0);
	//mpt::saturate_round<uint64>(1.0);

	// This should trigger assert in Round.
	//MPT_TEST_EXPECT_EQUAL(mpt::saturate_round<int8>(-129), 0);
}

} // namespace saturate_round
} // namespace base
} // namespace tests



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_BASE_TESTS_SATURATE_ROUND_HPP
