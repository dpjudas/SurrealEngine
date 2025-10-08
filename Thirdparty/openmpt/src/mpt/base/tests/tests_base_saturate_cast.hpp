/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_BASE_TESTS_SATURATE_CAST_HPP
#define MPT_BASE_TESTS_SATURATE_CAST_HPP



#include "mpt/base/detect_compiler.hpp"
#include "mpt/base/integer.hpp"
#include "mpt/base/namespace.hpp"
#include "mpt/base/saturate_cast.hpp"
#include "mpt/test/test.hpp"
#include "mpt/test/test_macros.hpp"

#include <limits>



namespace mpt {
inline namespace MPT_INLINE_NS {



namespace tests {
namespace base {
namespace saturate_cast {

#if MPT_COMPILER_CLANG
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wglobal-constructors"
#endif
MPT_TEST_GROUP_INLINE("mpt/base/saturate_cast")
#if MPT_COMPILER_CLANG
#pragma clang diagnostic pop
#endif
{
	// trivials
	MPT_TEST_EXPECT_EQUAL(mpt::saturate_cast<int>(-1), -1);
	MPT_TEST_EXPECT_EQUAL(mpt::saturate_cast<int>(0), 0);
	MPT_TEST_EXPECT_EQUAL(mpt::saturate_cast<int>(1), 1);
	MPT_TEST_EXPECT_EQUAL(mpt::saturate_cast<int>(std::numeric_limits<int>::min()), std::numeric_limits<int>::min());
	MPT_TEST_EXPECT_EQUAL(mpt::saturate_cast<int>(std::numeric_limits<int>::max()), std::numeric_limits<int>::max());

	// signed / unsigned
	MPT_TEST_EXPECT_EQUAL(mpt::saturate_cast<int16>(std::numeric_limits<uint16>::min()), std::numeric_limits<uint16>::min());
	MPT_TEST_EXPECT_EQUAL(mpt::saturate_cast<int16>(std::numeric_limits<uint16>::max()), std::numeric_limits<int16>::max());
	MPT_TEST_EXPECT_EQUAL(mpt::saturate_cast<int32>(std::numeric_limits<uint32>::min()), (int32)std::numeric_limits<uint32>::min());
	MPT_TEST_EXPECT_EQUAL(mpt::saturate_cast<int32>(std::numeric_limits<uint32>::max()), std::numeric_limits<int32>::max());
	MPT_TEST_EXPECT_EQUAL(mpt::saturate_cast<int64>(std::numeric_limits<uint64>::min()), (int64)std::numeric_limits<uint64>::min());
	MPT_TEST_EXPECT_EQUAL(mpt::saturate_cast<int64>(std::numeric_limits<uint64>::max()), std::numeric_limits<int64>::max());
	MPT_TEST_EXPECT_EQUAL(mpt::saturate_cast<uint16>(std::numeric_limits<int16>::min()), std::numeric_limits<uint16>::min());
	MPT_TEST_EXPECT_EQUAL(mpt::saturate_cast<uint16>(std::numeric_limits<int16>::max()), std::numeric_limits<int16>::max());
	MPT_TEST_EXPECT_EQUAL(mpt::saturate_cast<uint32>(std::numeric_limits<int32>::min()), std::numeric_limits<uint32>::min());
	MPT_TEST_EXPECT_EQUAL(mpt::saturate_cast<uint32>(std::numeric_limits<int32>::max()), (uint32)std::numeric_limits<int32>::max());
	MPT_TEST_EXPECT_EQUAL(mpt::saturate_cast<uint64>(std::numeric_limits<int64>::min()), std::numeric_limits<uint64>::min());
	MPT_TEST_EXPECT_EQUAL(mpt::saturate_cast<uint64>(std::numeric_limits<int64>::max()), (uint64)std::numeric_limits<int64>::max());

	// overflow
	MPT_TEST_EXPECT_EQUAL(mpt::saturate_cast<int16>(std::numeric_limits<int16>::min() - 1), std::numeric_limits<int16>::min());
	MPT_TEST_EXPECT_EQUAL(mpt::saturate_cast<int16>(std::numeric_limits<int16>::max() + 1), std::numeric_limits<int16>::max());
	MPT_TEST_EXPECT_EQUAL(mpt::saturate_cast<int32>(std::numeric_limits<int32>::min() - int64(1)), std::numeric_limits<int32>::min());
	MPT_TEST_EXPECT_EQUAL(mpt::saturate_cast<int32>(std::numeric_limits<int32>::max() + int64(1)), std::numeric_limits<int32>::max());

	MPT_TEST_EXPECT_EQUAL(mpt::saturate_cast<uint16>(std::numeric_limits<int16>::min() - 1), std::numeric_limits<uint16>::min());
	MPT_TEST_EXPECT_EQUAL(mpt::saturate_cast<uint16>(std::numeric_limits<int16>::max() + 1), (uint16)std::numeric_limits<int16>::max() + 1);
	MPT_TEST_EXPECT_EQUAL(mpt::saturate_cast<uint32>(std::numeric_limits<int32>::min() - int64(1)), std::numeric_limits<uint32>::min());
	MPT_TEST_EXPECT_EQUAL(mpt::saturate_cast<uint32>(std::numeric_limits<int32>::max() + int64(1)), (uint32)std::numeric_limits<int32>::max() + 1);

	MPT_TEST_EXPECT_EQUAL(mpt::saturate_cast<int8>(int16(32000)), 127);
	MPT_TEST_EXPECT_EQUAL(mpt::saturate_cast<int8>(int16(-32000)), -128);
	MPT_TEST_EXPECT_EQUAL(mpt::saturate_cast<int8>(uint16(32000)), 127);
	MPT_TEST_EXPECT_EQUAL(mpt::saturate_cast<int8>(uint16(64000)), 127);
	MPT_TEST_EXPECT_EQUAL(mpt::saturate_cast<uint8>(int16(32000)), 255);
	MPT_TEST_EXPECT_EQUAL(mpt::saturate_cast<uint8>(int16(-32000)), 0);
	MPT_TEST_EXPECT_EQUAL(mpt::saturate_cast<uint8>(uint16(32000)), 255);
	MPT_TEST_EXPECT_EQUAL(mpt::saturate_cast<uint8>(uint16(64000)), 255);
	MPT_TEST_EXPECT_EQUAL(mpt::saturate_cast<int16>(int16(32000)), 32000);
	MPT_TEST_EXPECT_EQUAL(mpt::saturate_cast<int16>(int16(-32000)), -32000);
	MPT_TEST_EXPECT_EQUAL(mpt::saturate_cast<int16>(uint16(32000)), 32000);
	MPT_TEST_EXPECT_EQUAL(mpt::saturate_cast<int16>(uint16(64000)), 32767);
	MPT_TEST_EXPECT_EQUAL(mpt::saturate_cast<uint16>(int16(32000)), 32000);
	MPT_TEST_EXPECT_EQUAL(mpt::saturate_cast<uint16>(int16(-32000)), 0);
	MPT_TEST_EXPECT_EQUAL(mpt::saturate_cast<uint16>(uint16(32000)), 32000);
	MPT_TEST_EXPECT_EQUAL(mpt::saturate_cast<uint16>(uint16(64000)), 64000);
	MPT_TEST_EXPECT_EQUAL(mpt::saturate_cast<int32>(int16(32000)), 32000);
	MPT_TEST_EXPECT_EQUAL(mpt::saturate_cast<int32>(int16(-32000)), -32000);
	MPT_TEST_EXPECT_EQUAL(mpt::saturate_cast<int32>(uint16(32000)), 32000);
	MPT_TEST_EXPECT_EQUAL(mpt::saturate_cast<int32>(uint16(64000)), 64000);
	MPT_TEST_EXPECT_EQUAL(mpt::saturate_cast<uint32>(int16(32000)), 32000u);
	MPT_TEST_EXPECT_EQUAL(mpt::saturate_cast<uint32>(int16(-32000)), 0u);
	MPT_TEST_EXPECT_EQUAL(mpt::saturate_cast<uint32>(uint16(32000)), 32000u);
	MPT_TEST_EXPECT_EQUAL(mpt::saturate_cast<uint32>(uint16(64000)), 64000u);

	MPT_TEST_EXPECT_EQUAL(mpt::saturate_cast<uint32>(std::numeric_limits<int64>::max() - 1), std::numeric_limits<uint32>::max());

	MPT_TEST_EXPECT_EQUAL(mpt::saturate_cast<int32>(std::numeric_limits<uint64>::max() - 1), std::numeric_limits<int32>::max());
}

} // namespace saturate_cast
} // namespace base
} // namespace tests



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_BASE_TESTS_SATURATE_CAST_HPP
