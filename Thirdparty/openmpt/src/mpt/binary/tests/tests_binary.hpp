/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_BASE_TESTS_BINARY_HPP
#define MPT_BASE_TESTS_BINARY_HPP



#include "mpt/base/alloc.hpp"
#include "mpt/base/detect_compiler.hpp"
#include "mpt/base/memory.hpp"
#include "mpt/base/namespace.hpp"
#include "mpt/base/span.hpp"
#include "mpt/binary/base64.hpp"
#include "mpt/binary/base64url.hpp"
#include "mpt/string/types.hpp"
#include "mpt/test/test.hpp"
#include "mpt/test/test_macros.hpp"

#include <string>
#include <vector>

#include <cstddef>



namespace mpt {
inline namespace MPT_INLINE_NS {



namespace tests {
namespace binary {

#if MPT_COMPILER_CLANG
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wglobal-constructors"
#endif
MPT_TEST_GROUP_INLINE("mpt/binary")
#if MPT_COMPILER_CLANG
#pragma clang diagnostic pop
#endif
{
	{
		std::string expecteds = std::string("pleasure.");
		std::vector<std::byte> expected(mpt::byte_cast<mpt::const_byte_span>(mpt::as_span(expecteds)).data(), mpt::byte_cast<mpt::const_byte_span>(mpt::as_span(expecteds)).data() + mpt::byte_cast<mpt::const_byte_span>(mpt::as_span(expecteds)).size());
		MPT_TEST_EXPECT_EQUAL(mpt::encode_base64(mpt::as_span(expected)), MPT_USTRING("cGxlYXN1cmUu"));
	}
	{
		std::string expecteds = std::string("leasure.");
		std::vector<std::byte> expected(mpt::byte_cast<mpt::const_byte_span>(mpt::as_span(expecteds)).data(), mpt::byte_cast<mpt::const_byte_span>(mpt::as_span(expecteds)).data() + mpt::byte_cast<mpt::const_byte_span>(mpt::as_span(expecteds)).size());
		MPT_TEST_EXPECT_EQUAL(mpt::encode_base64(mpt::as_span(expected)), MPT_USTRING("bGVhc3VyZS4="));
	}
	{
		std::string expecteds = std::string("easure.");
		std::vector<std::byte> expected(mpt::byte_cast<mpt::const_byte_span>(mpt::as_span(expecteds)).data(), mpt::byte_cast<mpt::const_byte_span>(mpt::as_span(expecteds)).data() + mpt::byte_cast<mpt::const_byte_span>(mpt::as_span(expecteds)).size());
		MPT_TEST_EXPECT_EQUAL(mpt::encode_base64(mpt::as_span(expected)), MPT_USTRING("ZWFzdXJlLg=="));
	}
	{
		std::string expecteds = std::string("pleasure.");
		std::vector<std::byte> expected(mpt::byte_cast<mpt::const_byte_span>(mpt::as_span(expecteds)).data(), mpt::byte_cast<mpt::const_byte_span>(mpt::as_span(expecteds)).data() + mpt::byte_cast<mpt::const_byte_span>(mpt::as_span(expecteds)).size());
		MPT_TEST_EXPECT_EQUAL(expected, mpt::decode_base64(MPT_USTRING("cGxlYXN1cmUu")));
	}
	{
		std::string expecteds = std::string("leasure.");
		std::vector<std::byte> expected(mpt::byte_cast<mpt::const_byte_span>(mpt::as_span(expecteds)).data(), mpt::byte_cast<mpt::const_byte_span>(mpt::as_span(expecteds)).data() + mpt::byte_cast<mpt::const_byte_span>(mpt::as_span(expecteds)).size());
		MPT_TEST_EXPECT_EQUAL(expected, mpt::decode_base64(MPT_USTRING("bGVhc3VyZS4=")));
	}
	{
		std::string expecteds = std::string("easure.");
		std::vector<std::byte> expected(mpt::byte_cast<mpt::const_byte_span>(mpt::as_span(expecteds)).data(), mpt::byte_cast<mpt::const_byte_span>(mpt::as_span(expecteds)).data() + mpt::byte_cast<mpt::const_byte_span>(mpt::as_span(expecteds)).size());
		MPT_TEST_EXPECT_EQUAL(expected, mpt::decode_base64(MPT_USTRING("ZWFzdXJlLg==")));
	}

	{
		std::string expecteds = std::string("pleasure.");
		std::vector<std::byte> expected(mpt::byte_cast<mpt::const_byte_span>(mpt::as_span(expecteds)).data(), mpt::byte_cast<mpt::const_byte_span>(mpt::as_span(expecteds)).data() + mpt::byte_cast<mpt::const_byte_span>(mpt::as_span(expecteds)).size());
		MPT_TEST_EXPECT_EQUAL(mpt::encode_base64url(mpt::as_span(expected)), MPT_USTRING("cGxlYXN1cmUu"));
	}
	{
		std::string expecteds = std::string("leasure.");
		std::vector<std::byte> expected(mpt::byte_cast<mpt::const_byte_span>(mpt::as_span(expecteds)).data(), mpt::byte_cast<mpt::const_byte_span>(mpt::as_span(expecteds)).data() + mpt::byte_cast<mpt::const_byte_span>(mpt::as_span(expecteds)).size());
		MPT_TEST_EXPECT_EQUAL(mpt::encode_base64url(mpt::as_span(expected)), MPT_USTRING("bGVhc3VyZS4"));
	}
	{
		std::string expecteds = std::string("easure.");
		std::vector<std::byte> expected(mpt::byte_cast<mpt::const_byte_span>(mpt::as_span(expecteds)).data(), mpt::byte_cast<mpt::const_byte_span>(mpt::as_span(expecteds)).data() + mpt::byte_cast<mpt::const_byte_span>(mpt::as_span(expecteds)).size());
		MPT_TEST_EXPECT_EQUAL(mpt::encode_base64url(mpt::as_span(expected)), MPT_USTRING("ZWFzdXJlLg"));
	}
	{
		std::string expecteds = std::string("pleasure.");
		std::vector<std::byte> expected(mpt::byte_cast<mpt::const_byte_span>(mpt::as_span(expecteds)).data(), mpt::byte_cast<mpt::const_byte_span>(mpt::as_span(expecteds)).data() + mpt::byte_cast<mpt::const_byte_span>(mpt::as_span(expecteds)).size());
		MPT_TEST_EXPECT_EQUAL(expected, mpt::decode_base64url(MPT_USTRING("cGxlYXN1cmUu")));
	}
	{
		std::string expecteds = std::string("leasure.");
		std::vector<std::byte> expected(mpt::byte_cast<mpt::const_byte_span>(mpt::as_span(expecteds)).data(), mpt::byte_cast<mpt::const_byte_span>(mpt::as_span(expecteds)).data() + mpt::byte_cast<mpt::const_byte_span>(mpt::as_span(expecteds)).size());
		MPT_TEST_EXPECT_EQUAL(expected, mpt::decode_base64url(MPT_USTRING("bGVhc3VyZS4")));
	}
	{
		std::string expecteds = std::string("easure.");
		std::vector<std::byte> expected(mpt::byte_cast<mpt::const_byte_span>(mpt::as_span(expecteds)).data(), mpt::byte_cast<mpt::const_byte_span>(mpt::as_span(expecteds)).data() + mpt::byte_cast<mpt::const_byte_span>(mpt::as_span(expecteds)).size());
		MPT_TEST_EXPECT_EQUAL(expected, mpt::decode_base64url(MPT_USTRING("ZWFzdXJlLg")));
	}
}

} // namespace binary
} // namespace tests



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_BASE_TESTS_BINARY_HPP
