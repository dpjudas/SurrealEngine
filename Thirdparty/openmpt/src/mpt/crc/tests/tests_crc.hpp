/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_BASE_TESTS_CRC_HPP
#define MPT_BASE_TESTS_CRC_HPP



#include "mpt/base/detect_compiler.hpp"
#include "mpt/base/namespace.hpp"
#include "mpt/crc/crc.hpp"
#include "mpt/test/test.hpp"
#include "mpt/test/test_macros.hpp"

#include <string>



namespace mpt {
inline namespace MPT_INLINE_NS {



namespace tests {
namespace crc {

#if MPT_COMPILER_CLANG
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wglobal-constructors"
#endif
MPT_TEST_GROUP_INLINE("mpt/crc")
#if MPT_COMPILER_CLANG
#pragma clang diagnostic pop
#endif
{
	MPT_TEST_EXPECT_EQUAL(mpt::crc32(std::string("123456789")), 0xCBF43926u);
	MPT_TEST_EXPECT_EQUAL(mpt::crc32_ogg(std::string("123456789")), 0x89a1897fu);
}

} // namespace crc
} // namespace tests



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_BASE_TESTS_CRC_HPP
