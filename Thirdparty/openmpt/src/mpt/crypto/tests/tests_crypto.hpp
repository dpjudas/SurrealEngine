/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_BASE_TESTS_CRYPTO_HPP
#define MPT_BASE_TESTS_CRYPTO_HPP



#include "mpt/base/detect.hpp"
#include "mpt/base/memory.hpp"
#include "mpt/base/namespace.hpp"
#include "mpt/base/span.hpp"
#include "mpt/crypto/config.hpp"
#include "mpt/crypto/exception.hpp"
#include "mpt/crypto/hash.hpp"
#include "mpt/crypto/jwk.hpp"
#include "mpt/detect/nlohmann_json.hpp"
#include "mpt/string/types.hpp"
#include "mpt/test/test.hpp"
#include "mpt/test/test_macros.hpp"

#include <string>
#include <vector>

#include <cstddef>



namespace mpt {
inline namespace MPT_INLINE_NS {



namespace tests {
namespace crypto {

#if MPT_COMPILER_CLANG
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wglobal-constructors"
#endif
MPT_TEST_GROUP_INLINE("mpt/crypto")
#if MPT_COMPILER_CLANG
#pragma clang diagnostic pop
#endif
{
#if defined(MPT_CRYPTO_CRYPTOPP) || defined(MPT_CRYPTO_WINDOWS)
	mpt::crypto::hash::SHA512::result_type sha512_abc{
		std::byte{0xdd}, std::byte{0xaf}, std::byte{0x35}, std::byte{0xa1}, std::byte{0x93}, std::byte{0x61}, std::byte{0x7a}, std::byte{0xba},
		std::byte{0xcc}, std::byte{0x41}, std::byte{0x73}, std::byte{0x49}, std::byte{0xae}, std::byte{0x20}, std::byte{0x41}, std::byte{0x31},
		std::byte{0x12}, std::byte{0xe6}, std::byte{0xfa}, std::byte{0x4e}, std::byte{0x89}, std::byte{0xa9}, std::byte{0x7e}, std::byte{0xa2},
		std::byte{0x0a}, std::byte{0x9e}, std::byte{0xee}, std::byte{0xe6}, std::byte{0x4b}, std::byte{0x55}, std::byte{0xd3}, std::byte{0x9a},
		std::byte{0x21}, std::byte{0x92}, std::byte{0x99}, std::byte{0x2a}, std::byte{0x27}, std::byte{0x4f}, std::byte{0xc1}, std::byte{0xa8},
		std::byte{0x36}, std::byte{0xba}, std::byte{0x3c}, std::byte{0x23}, std::byte{0xa3}, std::byte{0xfe}, std::byte{0xeb}, std::byte{0xbd},
		std::byte{0x45}, std::byte{0x4d}, std::byte{0x44}, std::byte{0x23}, std::byte{0x64}, std::byte{0x3c}, std::byte{0xe8}, std::byte{0x0e},
		std::byte{0x2a}, std::byte{0x9a}, std::byte{0xc9}, std::byte{0x4f}, std::byte{0xa5}, std::byte{0x4c}, std::byte{0xa4}, std::byte{0x9f}};
	MPT_TEST_EXPECT_EQUAL(mpt::crypto::hash::SHA512().process(mpt::byte_cast<mpt::const_byte_span>(mpt::as_span(std::string("abc")))).result(), sha512_abc);

#endif // MPT_WITH_CRYPTOPP || MPT_OS_WINDOWS

#if MPT_OS_WINDOWS && defined(MPT_CRYPTO_WINDOWS) && !defined(MPT_CRYPTO_CRYPTOPP) && MPT_DETECTED_NLOHMANN_JSON

	{

		std::vector<std::byte> data = {std::byte{0x11}, std::byte{0x12}, std::byte{0x13}, std::byte{0x14}};

		mpt::crypto::keystore keystore(mpt::crypto::keystore::domain::user);

		mpt::crypto::asymmetric::rsassa_pss<>::managed_private_key key(keystore, U_("OpenMPT Test Key 1"));

		auto publickeydata = key.get_public_key_data();

		mpt::crypto::asymmetric::rsassa_pss<>::public_key pk{publickeydata};
		mpt::crypto::asymmetric::rsassa_pss<>::public_key pk_copy{pk};
		mpt::ustring jwk = publickeydata.as_jwk();

		std::vector<std::byte> signature = key.sign(mpt::as_span(data));
		mpt::ustring jws = key.jws_sign(mpt::as_span(data));
		mpt::ustring jws_compact = key.jws_compact_sign(mpt::as_span(data));

		try {
			pk.verify(mpt::as_span(data), signature);
			auto verifieddata1 = mpt::crypto::asymmetric::rsassa_pss<>::public_key(mpt::crypto::asymmetric::rsassa_pss<>::public_key_data::from_jwk(jwk)).jws_verify(jws);
			auto verifieddata2 = mpt::crypto::asymmetric::rsassa_pss<>::public_key(mpt::crypto::asymmetric::rsassa_pss<>::public_key_data::from_jwk(jwk)).jws_compact_verify(jws_compact);
			MPT_TEST_EXPECT_EQUAL(true, true);
			MPT_TEST_EXPECT_EQUAL(data, verifieddata1);
			MPT_TEST_EXPECT_EQUAL(data, verifieddata2);
		} catch (const mpt::crypto::asymmetric::signature_verification_failed &) {
			MPT_TEST_EXPECT_EQUAL(true, false);
		}

		key.destroy();
	}

#endif //  MPT_OS_WINDOWS && MPT_CRYPTO_WINDOWS && MPT_DETECTED_NLOHMANN_JSON
}


} // namespace crypto
} // namespace tests



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_BASE_TESTS_CRYPTO_HPP
