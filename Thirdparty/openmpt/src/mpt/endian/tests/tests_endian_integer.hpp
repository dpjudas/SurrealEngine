/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_ENDIAN_TESTS_ENDIAN_INTEGER_HPP
#define MPT_ENDIAN_TESTS_ENDIAN_INTEGER_HPP



#include "mpt/base/detect_compiler.hpp"
#include "mpt/base/integer.hpp"
#include "mpt/base/macros.hpp"
#include "mpt/base/namespace.hpp"
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
MPT_TEST_GROUP_INLINE("mpt/endian/integer")
#if MPT_COMPILER_CLANG
#pragma clang diagnostic pop
#endif
{
	static_assert(std::numeric_limits<int8le>::min() == std::numeric_limits<int8>::min());
	static_assert(std::numeric_limits<uint8le>::min() == std::numeric_limits<uint8>::min());

	static_assert(std::numeric_limits<int16le>::min() == std::numeric_limits<int16>::min());
	static_assert(std::numeric_limits<uint16le>::min() == std::numeric_limits<uint16>::min());

	static_assert(std::numeric_limits<int32le>::min() == std::numeric_limits<int32>::min());
	static_assert(std::numeric_limits<uint32le>::min() == std::numeric_limits<uint32>::min());

	static_assert(std::numeric_limits<int64le>::min() == std::numeric_limits<int64>::min());
	static_assert(std::numeric_limits<uint64le>::min() == std::numeric_limits<uint64>::min());

	static_assert(std::numeric_limits<int8le>::max() == std::numeric_limits<int8>::max());
	static_assert(std::numeric_limits<uint8le>::max() == std::numeric_limits<uint8>::max());

	static_assert(std::numeric_limits<int16le>::max() == std::numeric_limits<int16>::max());
	static_assert(std::numeric_limits<uint16le>::max() == std::numeric_limits<uint16>::max());

	static_assert(std::numeric_limits<int32le>::max() == std::numeric_limits<int32>::max());
	static_assert(std::numeric_limits<uint32le>::max() == std::numeric_limits<uint32>::max());

	static_assert(std::numeric_limits<int64le>::max() == std::numeric_limits<int64>::max());
	static_assert(std::numeric_limits<uint64le>::max() == std::numeric_limits<uint64>::max());

	struct test_endian_constexpr {
		static MPT_CONSTEXPR20_FUN int32le test(uint32 x) {
			int32le foo{};
			foo = x;
			return foo;
		}
	};

	MPT_CONSTEXPR20_VAR int32le foo = test_endian_constexpr::test(23);
	static_cast<void>(foo);

	// Packed integers with defined endianness
	{
		int8le le8;
		le8.set(-128);
		int8be be8;
		be8.set(-128);
		MPT_TEST_EXPECT_EQUAL(le8, -128);
		MPT_TEST_EXPECT_EQUAL(be8, -128);
		MPT_TEST_EXPECT_EQUAL(std::memcmp(&le8, "\x80", 1), 0);
		MPT_TEST_EXPECT_EQUAL(std::memcmp(&be8, "\x80", 1), 0);
		int16le le16;
		le16.set(0x1234);
		int16be be16;
		be16.set(0x1234);
		MPT_TEST_EXPECT_EQUAL(le16, 0x1234);
		MPT_TEST_EXPECT_EQUAL(be16, 0x1234);
		MPT_TEST_EXPECT_EQUAL(std::memcmp(&le16, "\x34\x12", 2), 0);
		MPT_TEST_EXPECT_EQUAL(std::memcmp(&be16, "\x12\x34", 2), 0);
		uint32le le32;
		le32.set(0xFFEEDDCCu);
		uint32be be32;
		be32.set(0xFFEEDDCCu);
		MPT_TEST_EXPECT_EQUAL(le32, 0xFFEEDDCCu);
		MPT_TEST_EXPECT_EQUAL(be32, 0xFFEEDDCCu);
		MPT_TEST_EXPECT_EQUAL(std::memcmp(&le32, "\xCC\xDD\xEE\xFF", 4), 0);
		MPT_TEST_EXPECT_EQUAL(std::memcmp(&be32, "\xFF\xEE\xDD\xCC", 4), 0);
		uint64le le64;
		le64.set(0xDEADC0DE15C0FFEEull);
		uint64be be64;
		be64.set(0xDEADC0DE15C0FFEEull);
		MPT_TEST_EXPECT_EQUAL(le64, 0xDEADC0DE15C0FFEEull);
		MPT_TEST_EXPECT_EQUAL(be64, 0xDEADC0DE15C0FFEEull);
		MPT_TEST_EXPECT_EQUAL(std::memcmp(&le64, "\xEE\xFF\xC0\x15\xDE\xC0\xAD\xDE", 8), 0);
		MPT_TEST_EXPECT_EQUAL(std::memcmp(&be64, "\xDE\xAD\xC0\xDE\x15\xC0\xFF\xEE", 8), 0);
	}
}

} // namespace integer
} // namespace endian
} // namespace tests



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_ENDIAN_TESTS_ENDIAN_INTEGER_HPP
