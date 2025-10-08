/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_BASE_TESTS_UUID_HPP
#define MPT_BASE_TESTS_UUID_HPP



#include "mpt/base/detect.hpp"
#include "mpt/base/namespace.hpp"
#include "mpt/random/default_engines.hpp"
#include "mpt/random/device.hpp"
#include "mpt/random/seed.hpp"
#include "mpt/string/types.hpp"
#include "mpt/test/test.hpp"
#include "mpt/test/test_macros.hpp"
#include "mpt/uuid/guid.hpp"
#include "mpt/uuid/uuid.hpp"

#include <cstddef>
#include <cstring>



namespace mpt {
inline namespace MPT_INLINE_NS {



namespace tests {
namespace uuid {

#if MPT_COMPILER_CLANG
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wglobal-constructors"
#endif
MPT_TEST_GROUP_INLINE("mpt/uuid")
#if MPT_COMPILER_CLANG
#pragma clang diagnostic pop
#endif
{
	using namespace mpt::uuid_literals;

	MPT_TEST_EXPECT_EQUAL(mpt::UUID(0x2ed6593au, 0xdfe6, 0x4cf8, 0xb2e575ad7f600c32ull).ToUString(), MPT_USTRING("2ed6593a-dfe6-4cf8-b2e5-75ad7f600c32"));
#if MPT_OS_WINDOWS && !defined(MPT_COMPILER_QUIRK_NO_WCHAR)
	constexpr mpt::UUID uuid_tmp = "2ed6593a-dfe6-4cf8-b2e5-75ad7f600c32"_uuid;
	MPT_TEST_EXPECT_EQUAL(mpt::UUID(0x2ed6593au, 0xdfe6, 0x4cf8, 0xb2e575ad7f600c32ull), uuid_tmp);
	MPT_TEST_EXPECT_EQUAL(mpt::UUID(0x2ed6593au, 0xdfe6, 0x4cf8, 0xb2e575ad7f600c32ull), mpt::UUID(mpt::StringToGUID(TEXT("{2ed6593a-dfe6-4cf8-b2e5-75ad7f600c32}"))));
	MPT_TEST_EXPECT_EQUAL(mpt::UUID(0x2ed6593au, 0xdfe6, 0x4cf8, 0xb2e575ad7f600c32ull), mpt::UUID(mpt::StringToCLSID(TEXT("{2ed6593a-dfe6-4cf8-b2e5-75ad7f600c32}"))));
	MPT_TEST_EXPECT_EQUAL(mpt::UUID(0x00112233u, 0x4455, 0x6677, 0x8899AABBCCDDEEFFull), mpt::UUID(mpt::StringToGUID(TEXT("{00112233-4455-6677-8899-AABBCCDDEEFF}"))));
	MPT_TEST_EXPECT_EQUAL(mpt::UUID(0x00112233u, 0x4455, 0x6677, 0xC899AABBCCDDEEFFull), mpt::UUID(mpt::StringToGUID(TEXT("{00112233-4455-6677-C899-AABBCCDDEEFF}"))));
	MPT_TEST_EXPECT_EQUAL(mpt::GUIDToString(mpt::UUID(0x00112233u, 0x4455, 0x6677, 0x8899AABBCCDDEEFFull)), TEXT("{00112233-4455-6677-8899-AABBCCDDEEFF}"));
	MPT_TEST_EXPECT_EQUAL(mpt::GUIDToString(mpt::UUID(0x00112233u, 0x4455, 0x6677, 0xC899AABBCCDDEEFFull)), TEXT("{00112233-4455-6677-C899-AABBCCDDEEFF}"));
#endif // MPT_OS_WINDOWS && !MPT_COMPILER_QUIRK_NO_WCHAR

	mpt::sane_random_device rd;
	mpt::good_engine prng = mpt::make_prng<mpt::good_engine>(rd);

#if MPT_OS_WINDOWS && !defined(MPT_COMPILER_QUIRK_NO_WCHAR)
	MPT_TEST_EXPECT_EQUAL(mpt::IsValid(mpt::CreateGUID()), true);
	{
		mpt::UUID uuid = mpt::UUID::Generate(prng);
		MPT_TEST_EXPECT_EQUAL(uuid, mpt::UUID::FromString(mpt::UUID(uuid).ToUString()));
		MPT_TEST_EXPECT_EQUAL(uuid, mpt::UUID(mpt::StringToGUID(mpt::GUIDToString(uuid))));
		MPT_TEST_EXPECT_EQUAL(uuid, mpt::UUID(mpt::StringToIID(mpt::IIDToString(uuid))));
		MPT_TEST_EXPECT_EQUAL(uuid, mpt::UUID(mpt::StringToCLSID(mpt::CLSIDToString(uuid))));
	}
	{
		GUID guid = mpt::UUID::Generate(prng);
		MPT_TEST_EXPECT_EQUAL(IsEqualGUID(guid, static_cast<GUID>(mpt::UUID::FromString(mpt::UUID(guid).ToUString()))), TRUE);
		MPT_TEST_EXPECT_EQUAL(IsEqualGUID(guid, mpt::StringToGUID(mpt::GUIDToString(guid))), TRUE);
		MPT_TEST_EXPECT_EQUAL(IsEqualGUID(guid, mpt::StringToIID(mpt::IIDToString(guid))), TRUE);
		MPT_TEST_EXPECT_EQUAL(IsEqualGUID(guid, mpt::StringToCLSID(mpt::CLSIDToString(guid))), TRUE);
	}
#endif // MPT_OS_WINDOWS && !MPT_COMPILER_QUIRK_NO_WCHAR
	MPT_TEST_EXPECT_EQUAL(mpt::UUID::Generate(prng).IsValid(), true);
	MPT_TEST_EXPECT_EQUAL(mpt::UUID::GenerateLocalUseOnly(prng).IsValid(), true);
	MPT_TEST_EXPECT_EQUAL(mpt::UUID::Generate(prng) != mpt::UUID::Generate(prng), true);
	mpt::UUID a = mpt::UUID::Generate(prng);
	MPT_TEST_EXPECT_EQUAL(a, mpt::UUID::FromString(a.ToUString()));
	std::byte uuiddata[16]{};
	for (std::size_t i = 0; i < 16; ++i) {
		uuiddata[i] = mpt::byte_cast<std::byte>(static_cast<uint8>(i));
	}
	static_assert(sizeof(mpt::UUID) == 16);
	mpt::UUIDbin uuid2;
	std::memcpy(&uuid2, uuiddata, 16);
	MPT_TEST_EXPECT_EQUAL(mpt::UUID(uuid2).ToUString(), MPT_USTRING("00010203-0405-0607-0809-0a0b0c0d0e0f"));

	constexpr mpt::UUID uuid3 = "2ed6593a-dfe6-4cf8-b2e5-75ad7f600c32"_uuid;
	MPT_TEST_EXPECT_EQUAL(mpt::UUID(0x2ed6593au, 0xdfe6, 0x4cf8, 0xb2e575ad7f600c32ull), uuid3);
}

} // namespace uuid
} // namespace tests



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_BASE_TESTS_UUID_HPP
