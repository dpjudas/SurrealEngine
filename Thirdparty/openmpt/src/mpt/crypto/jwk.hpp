/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_CRYPTO_JWK_HPP
#define MPT_CRYPTO_JWK_HPP



#include "mpt/base/alloc.hpp"
#include "mpt/base/detect.hpp"
#include "mpt/base/memory.hpp"
#include "mpt/base/namespace.hpp"
#include "mpt/base/saturate_cast.hpp"
#include "mpt/base/span.hpp"
#include "mpt/binary/base64url.hpp"
#include "mpt/crypto/config.hpp"
#include "mpt/crypto/exception.hpp"
#include "mpt/crypto/hash.hpp"
#include "mpt/detect/nlohmann_json.hpp"
#include "mpt/json/json.hpp"
#include "mpt/out_of_memory/out_of_memory.hpp"
#include "mpt/string/types.hpp"
#include "mpt/string/utility.hpp"
#include "mpt/string_transcode/transcode.hpp"

#include <algorithm>
#include <stdexcept>
#include <string>
#include <vector>

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>

#if defined(MPT_CRYPTO_WINDOWS)
#include <windows.h> // must be before wincrypt.h for clang-cl
#include <bcrypt.h>
#include <wincrypt.h> // must be before ncrypt.h
#include <ncrypt.h>
#endif // MPT_CRYPTO_WINDOWS

#if defined(MPT_CRYPTO_CRYPTOPP)
#if MPT_COMPILER_MSVC
#pragma warning(push)
#endif // MPT_COMPILER_MSVC
#if MPT_COMPILER_GCC
#pragma GCC diagnostic push
#endif // MPT_COMPILER_GCC
#if MPT_COMPILER_CLANG
#pragma clang diagnostic push
#endif // MPT_COMPILER_CLANG
#include <cryptopp/cryptlib.h>
#include <cryptopp/pssr.h>
#include <cryptopp/rsa.h>
#if MPT_COMPILER_CLANG
#pragma clang diagnostic pop
#endif // MPT_COMPILER_CLANG
#if MPT_COMPILER_GCC
#pragma GCC diagnostic pop
#endif // MPT_COMPILER_GCC
#if MPT_COMPILER_MSVC
#pragma warning(pop)
#endif // MPT_COMPILER_MSVC
#endif // MPT_CRYPTO_CRYPTOPP


namespace mpt {
inline namespace MPT_INLINE_NS {



namespace crypto {



#if MPT_DETECTED_NLOHMANN_JSON


#if defined(MPT_CRYPTO_WINDOWS)
#if MPT_COMPILER_CLANG
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winline-namespace-reopened-noninline"
#endif // MPT_COMPILER_CLANG
namespace windows {
#if MPT_COMPILER_CLANG
#pragma clang diagnostic pop
#endif // MPT_COMPILER_CLANG



class keystore {
public:
	enum class domain {
		system = 1,
		user = 2,
	};

private:
	NCRYPT_PROV_HANDLE hProv = NULL;
	domain ProvDomain = domain::user;

private:
	void cleanup() {
		if (hProv) {
			NCryptFreeObject(hProv);
			hProv = NULL;
		}
	}

public:
	keystore(domain d)
		: ProvDomain(d) {
		try {
			CheckSECURITY_STATUS(NCryptOpenStorageProvider(&hProv, MS_KEY_STORAGE_PROVIDER, 0), "NCryptOpenStorageProvider");
		} catch (...) {
			cleanup();
			throw;
		}
	}
	~keystore() {
		return;
	}
	operator NCRYPT_PROV_HANDLE() {
		return hProv;
	}
	keystore::domain store_domain() const {
		return ProvDomain;
	}
};



namespace asymmetric {



class signature_verification_failed
	: public std::runtime_error {
public:
	signature_verification_failed()
		: std::runtime_error("Signature Verification failed.") {
		return;
	}
};



inline std::vector<mpt::ustring> jws_get_keynames(const mpt::ustring & jws_) {
	std::vector<mpt::ustring> result;
	nlohmann::json jws = nlohmann::json::parse(mpt::transcode<std::string>(mpt::common_encoding::utf8, jws_));
	for (const auto & s : jws["signatures"]) {
		result.push_back(s["header"]["kid"].get<mpt::ustring>());
	}
	return result;
}



struct RSASSA_PSS_SHA512_traits {
	using hash_type = mpt::crypto::hash::SHA512;
	static constexpr const char * jwk_alg = "PS512";
};



template <typename Traits = RSASSA_PSS_SHA512_traits, std::size_t keysize = 4096>
class rsassa_pss {

public:
	using hash_type = typename Traits::hash_type;
	static constexpr const char * jwk_alg = Traits::jwk_alg;

	struct public_key_data {

		mpt::ustring name;
		uint32 length = 0;
		std::vector<std::byte> public_exp;
		std::vector<std::byte> modulus;

		std::vector<std::byte> as_cng_blob() const {
			BCRYPT_RSAKEY_BLOB rsakey_blob{};
			rsakey_blob.Magic = BCRYPT_RSAPUBLIC_MAGIC;
			rsakey_blob.BitLength = length;
			rsakey_blob.cbPublicExp = mpt::saturate_cast<ULONG>(public_exp.size());
			rsakey_blob.cbModulus = mpt::saturate_cast<ULONG>(modulus.size());
			std::vector<std::byte> result(sizeof(BCRYPT_RSAKEY_BLOB) + public_exp.size() + modulus.size());
			std::memcpy(result.data(), &rsakey_blob, sizeof(BCRYPT_RSAKEY_BLOB));
			std::memcpy(result.data() + sizeof(BCRYPT_RSAKEY_BLOB), public_exp.data(), public_exp.size());
			std::memcpy(result.data() + sizeof(BCRYPT_RSAKEY_BLOB) + public_exp.size(), modulus.data(), modulus.size());
			return result;
		}

		mpt::ustring as_jwk() const {
			nlohmann::json json = nlohmann::json::object();
			json["kid"] = name;
			json["kty"] = "RSA";
			json["alg"] = jwk_alg;
			json["use"] = "sig";
			json["e"] = mpt::encode_base64url(mpt::as_span(public_exp));
			json["n"] = mpt::encode_base64url(mpt::as_span(modulus));
			return mpt::transcode<mpt::ustring>(mpt::common_encoding::utf8, json.dump());
		}

		static public_key_data from_jwk(const mpt::ustring & jwk) {
			public_key_data result;
			try {
				nlohmann::json json = nlohmann::json::parse(mpt::transcode<std::string>(mpt::common_encoding::utf8, jwk));
				if (json["kty"] != "RSA") {
					throw std::runtime_error("Cannot parse RSA public key JWK.");
				}
				if (json["alg"] != jwk_alg) {
					throw std::runtime_error("Cannot parse RSA public key JWK.");
				}
				if (json["use"] != "sig") {
					throw std::runtime_error("Cannot parse RSA public key JWK.");
				}
				result.name = json["kid"].get<mpt::ustring>();
				result.public_exp = mpt::decode_base64url(json["e"].get<mpt::ustring>());
				result.modulus = mpt::decode_base64url(json["n"].get<mpt::ustring>());
				result.length = mpt::saturate_cast<uint32>(result.modulus.size() * 8);
			} catch (mpt::out_of_memory e) {
				mpt::rethrow_out_of_memory(e);
			} catch (...) {
				throw std::runtime_error("Cannot parse RSA public key JWK.");
			}
			return result;
		}

		static public_key_data from_cng_blob(const mpt::ustring & name, const std::vector<std::byte> & blob) {
			public_key_data result;
			BCRYPT_RSAKEY_BLOB rsakey_blob{};
			if (blob.size() < sizeof(BCRYPT_RSAKEY_BLOB)) {
				throw std::runtime_error("Cannot parse RSA public key blob.");
			}
			std::memcpy(&rsakey_blob, blob.data(), sizeof(BCRYPT_RSAKEY_BLOB));
			if (rsakey_blob.Magic != BCRYPT_RSAPUBLIC_MAGIC) {
				throw std::runtime_error("Cannot parse RSA public key blob.");
			}
			if (blob.size() != sizeof(BCRYPT_RSAKEY_BLOB) + rsakey_blob.cbPublicExp + rsakey_blob.cbModulus) {
				throw std::runtime_error("Cannot parse RSA public key blob.");
			}
			result.name = name;
			result.length = rsakey_blob.BitLength;
			result.public_exp = std::vector<std::byte>(blob.data() + sizeof(BCRYPT_RSAKEY_BLOB), blob.data() + sizeof(BCRYPT_RSAKEY_BLOB) + rsakey_blob.cbPublicExp);
			result.modulus = std::vector<std::byte>(blob.data() + sizeof(BCRYPT_RSAKEY_BLOB) + rsakey_blob.cbPublicExp, blob.data() + sizeof(BCRYPT_RSAKEY_BLOB) + rsakey_blob.cbPublicExp + rsakey_blob.cbModulus);
			return result;
		}
	};



	static std::vector<public_key_data> parse_jwk_set(const mpt::ustring & jwk_set_) {
		std::vector<public_key_data> result;
		nlohmann::json jwk_set = nlohmann::json::parse(mpt::transcode<std::string>(mpt::common_encoding::utf8, jwk_set_));
		for (const auto & k : jwk_set["keys"]) {
			try {
				result.push_back(public_key_data::from_jwk(mpt::transcode<mpt::ustring>(mpt::common_encoding::utf8, k.dump())));
			} catch (...) {
				// nothing
			}
		}
		return result;
	}



	class public_key {

	private:
		mpt::ustring name;
		BCRYPT_ALG_HANDLE hSignAlg = NULL;
		BCRYPT_KEY_HANDLE hKey = NULL;

	private:
		void cleanup() {
			if (hKey) {
				BCryptDestroyKey(hKey);
				hKey = NULL;
			}
			if (hSignAlg) {
				BCryptCloseAlgorithmProvider(hSignAlg, 0);
				hSignAlg = NULL;
			}
		}

	public:
		public_key(const public_key_data & data) {
			try {
				name = data.name;
				CheckNTSTATUS(BCryptOpenAlgorithmProvider(&hSignAlg, BCRYPT_RSA_ALGORITHM, NULL, 0), "BCryptOpenAlgorithmProvider");
				std::vector<std::byte> blob = data.as_cng_blob();
				CheckNTSTATUS(BCryptImportKeyPair(hSignAlg, NULL, BCRYPT_RSAPUBLIC_BLOB, &hKey, mpt::byte_cast<UCHAR *>(blob.data()), mpt::saturate_cast<ULONG>(blob.size()), 0), "BCryptImportKeyPair");
			} catch (...) {
				cleanup();
				throw;
			}
		}

		public_key(const public_key & other)
			: public_key(other.get_public_key_data()) {
			return;
		}

		public_key & operator=(const public_key & other) {
			if (&other == this) {
				return *this;
			}
			public_key copy(other);
			{
				using std::swap;
				swap(copy.name, name);
				swap(copy.hSignAlg, hSignAlg);
				swap(copy.hKey, hKey);
			}
			return *this;
		}

		~public_key() {
			cleanup();
		}

		mpt::ustring get_name() const {
			return name;
		}

		public_key_data get_public_key_data() const {
			DWORD bytes = 0;
			CheckNTSTATUS(BCryptExportKey(hKey, NULL, BCRYPT_RSAPUBLIC_BLOB, NULL, 0, &bytes, 0), "BCryptExportKey");
			std::vector<std::byte> blob(bytes);
			CheckNTSTATUS(BCryptExportKey(hKey, NULL, BCRYPT_RSAPUBLIC_BLOB, mpt::byte_cast<BYTE *>(blob.data()), mpt::saturate_cast<DWORD>(blob.size()), &bytes, 0), "BCryptExportKey");
			return public_key_data::from_cng_blob(name, blob);
		}

		void verify_hash(typename hash_type::result_type hash, std::vector<std::byte> signature) {
			BCRYPT_PSS_PADDING_INFO paddinginfo;
			paddinginfo.pszAlgId = hash_type::traits::bcrypt_name;
			paddinginfo.cbSalt = mpt::saturate_cast<ULONG>(hash_type::traits::output_bytes);
			NTSTATUS result = BCryptVerifySignature(hKey, &paddinginfo, mpt::byte_cast<UCHAR *>(hash.data()), mpt::saturate_cast<ULONG>(hash.size()), mpt::byte_cast<UCHAR *>(signature.data()), mpt::saturate_cast<ULONG>(signature.size()), BCRYPT_PAD_PSS);
			if (result == 0x00000000 /*STATUS_SUCCESS*/) {
				return;
			}
			if (static_cast<DWORD>(result) == 0xC000A000 /*STATUS_INVALID_SIGNATURE*/) {
				throw signature_verification_failed();
			}
			CheckNTSTATUS(result, "BCryptVerifySignature");
			throw signature_verification_failed();
		}

		void verify(mpt::const_byte_span payload, const std::vector<std::byte> & signature) {
			verify_hash(hash_type().process(payload).result(), signature);
		}

		std::vector<std::byte> jws_verify(const mpt::ustring & jws_) {
			nlohmann::json jws = nlohmann::json::parse(mpt::transcode<std::string>(mpt::common_encoding::utf8, jws_));
			std::vector<std::byte> payload = mpt::decode_base64url(jws["payload"].get<mpt::ustring>());
			nlohmann::json jsignature = nlohmann::json::object();
			bool sigfound = false;
			for (const auto & s : jws["signatures"]) {
				if (s["header"]["kid"] == mpt::transcode<std::string>(mpt::common_encoding::utf8, name)) {
					jsignature = s;
					sigfound = true;
				}
			}
			if (!sigfound) {
				throw signature_verification_failed();
			}
			std::vector<std::byte> protectedheaderraw = mpt::decode_base64url(jsignature["protected"].get<mpt::ustring>());
			std::vector<std::byte> signature = mpt::decode_base64url(jsignature["signature"].get<mpt::ustring>());
			nlohmann::json header = nlohmann::json::parse(mpt::buffer_cast<std::string>(protectedheaderraw));
			if (header["typ"] != "JWT") {
				throw signature_verification_failed();
			}
			if (header["alg"] != jwk_alg) {
				throw signature_verification_failed();
			}
			verify_hash(hash_type().process(mpt::byte_cast<mpt::const_byte_span>(mpt::as_span(mpt::transcode<std::string>(mpt::common_encoding::utf8, mpt::encode_base64url(mpt::as_span(protectedheaderraw)) + MPT_USTRING(".") + mpt::encode_base64url(mpt::as_span(payload)))))).result(), signature);
			return payload;
		}

		std::vector<std::byte> jws_compact_verify(const mpt::ustring & jws) {
			std::vector<mpt::ustring> parts = mpt::split<mpt::ustring>(jws, MPT_USTRING("."));
			if (parts.size() != 3) {
				throw signature_verification_failed();
			}
			std::vector<std::byte> protectedheaderraw = mpt::decode_base64url(parts[0]);
			std::vector<std::byte> payload = mpt::decode_base64url(parts[1]);
			std::vector<std::byte> signature = mpt::decode_base64url(parts[2]);
			nlohmann::json header = nlohmann::json::parse(mpt::buffer_cast<std::string>(protectedheaderraw));
			if (header["typ"] != "JWT") {
				throw signature_verification_failed();
			}
			if (header["alg"] != jwk_alg) {
				throw signature_verification_failed();
			}
			verify_hash(hash_type().process(mpt::byte_cast<mpt::const_byte_span>(mpt::as_span(mpt::transcode<std::string>(mpt::common_encoding::utf8, mpt::encode_base64url(mpt::as_span(protectedheaderraw)) + MPT_USTRING(".") + mpt::encode_base64url(mpt::as_span(payload)))))).result(), signature);
			return payload;
		}
	};



	static inline void jws_verify_at_least_one(std::vector<public_key> & keys, const std::vector<std::byte> & expectedPayload, const mpt::ustring & signature) {
		std::vector<mpt::ustring> keynames = mpt::crypto::asymmetric::jws_get_keynames(signature);
		bool sigchecked = false;
		for (const auto & keyname : keynames) {
			for (auto & key : keys) {
				if (key.get_name() == keyname) {
					if (expectedPayload != key.jws_verify(signature)) {
						throw mpt::crypto::asymmetric::signature_verification_failed();
					}
					sigchecked = true;
				}
			}
		}
		if (!sigchecked) {
			throw mpt::crypto::asymmetric::signature_verification_failed();
		}
	}



	static inline std::vector<std::byte> jws_verify_at_least_one(std::vector<public_key> & keys, const mpt::ustring & signature) {
		std::vector<mpt::ustring> keynames = mpt::crypto::asymmetric::jws_get_keynames(signature);
		for (const auto & keyname : keynames) {
			for (auto & key : keys) {
				if (key.get_name() == keyname) {
					return key.jws_verify(signature);
				}
			}
		}
		throw mpt::crypto::asymmetric::signature_verification_failed();
	}



	class managed_private_key {

	private:
		mpt::ustring name;
		NCRYPT_KEY_HANDLE hKey = NULL;

	private:
		void cleanup() {
			if (hKey) {
				NCryptFreeObject(hKey);
				hKey = NULL;
			}
		}

	public:
		managed_private_key() = delete;

		managed_private_key(const managed_private_key &) = delete;

		managed_private_key & operator=(const managed_private_key &) = delete;

		managed_private_key(keystore & keystore) {
			try {
				CheckSECURITY_STATUS(NCryptCreatePersistedKey(keystore, &hKey, BCRYPT_RSA_ALGORITHM, NULL, 0, 0), "NCryptCreatePersistedKey");
			} catch (...) {
				cleanup();
				throw;
			}
		}

		managed_private_key(keystore & keystore, const mpt::ustring & name_)
			: name(name_) {
			try {
				SECURITY_STATUS openKeyStatus = NCryptOpenKey(keystore, &hKey, mpt::transcode<std::wstring>(name).c_str(), 0, (keystore.store_domain() == keystore::domain::system ? NCRYPT_MACHINE_KEY_FLAG : 0));
				if (openKeyStatus == NTE_BAD_KEYSET) {
					CheckSECURITY_STATUS(NCryptCreatePersistedKey(keystore, &hKey, BCRYPT_RSA_ALGORITHM, mpt::transcode<std::wstring>(name).c_str(), 0, (keystore.store_domain() == keystore::domain::system ? NCRYPT_MACHINE_KEY_FLAG : 0)), "NCryptCreatePersistedKey");
					DWORD length = mpt::saturate_cast<DWORD>(keysize);
					CheckSECURITY_STATUS(NCryptSetProperty(hKey, NCRYPT_LENGTH_PROPERTY, (PBYTE)&length, mpt::saturate_cast<DWORD>(sizeof(DWORD)), 0), "NCryptSetProperty");
					CheckSECURITY_STATUS(NCryptFinalizeKey(hKey, 0), "NCryptFinalizeKey");
				} else {
					CheckSECURITY_STATUS(openKeyStatus, "NCryptOpenKey");
				}
			} catch (...) {
				cleanup();
				throw;
			}
		}

		~managed_private_key() {
			cleanup();
		}

		void destroy() {
			CheckSECURITY_STATUS(NCryptDeleteKey(hKey, 0), "NCryptDeleteKey");
			name = mpt::ustring();
			hKey = NULL;
		}

	public:
		public_key_data get_public_key_data() const {
			DWORD bytes = 0;
			CheckSECURITY_STATUS(NCryptExportKey(hKey, NULL, BCRYPT_RSAPUBLIC_BLOB, NULL, NULL, 0, &bytes, 0), "NCryptExportKey");
			std::vector<std::byte> blob(bytes);
			CheckSECURITY_STATUS(NCryptExportKey(hKey, NULL, BCRYPT_RSAPUBLIC_BLOB, NULL, mpt::byte_cast<BYTE *>(blob.data()), mpt::saturate_cast<DWORD>(blob.size()), &bytes, 0), "NCryptExportKey");
			return public_key_data::from_cng_blob(name, blob);
		}

		std::vector<std::byte> sign_hash(typename hash_type::result_type hash) {
			BCRYPT_PSS_PADDING_INFO paddinginfo;
			paddinginfo.pszAlgId = hash_type::traits::bcrypt_name;
			paddinginfo.cbSalt = mpt::saturate_cast<ULONG>(hash_type::traits::output_bytes);
			DWORD bytes = 0;
			CheckSECURITY_STATUS(NCryptSignHash(hKey, &paddinginfo, mpt::byte_cast<BYTE *>(hash.data()), mpt::saturate_cast<DWORD>(hash.size()), NULL, 0, &bytes, BCRYPT_PAD_PSS), "NCryptSignHash");
			std::vector<std::byte> result(bytes);
			CheckSECURITY_STATUS(NCryptSignHash(hKey, &paddinginfo, mpt::byte_cast<BYTE *>(hash.data()), mpt::saturate_cast<DWORD>(hash.size()), mpt::byte_cast<BYTE *>(result.data()), mpt::saturate_cast<DWORD>(result.size()), &bytes, BCRYPT_PAD_PSS), "NCryptSignHash");
			return result;
		}

		std::vector<std::byte> sign(mpt::const_byte_span payload) {
			return sign_hash(hash_type().process(payload).result());
		}

		mpt::ustring jws_compact_sign(mpt::const_byte_span payload) {
			nlohmann::json protectedheader = nlohmann::json::object();
			protectedheader["typ"] = "JWT";
			protectedheader["alg"] = jwk_alg;
			std::string protectedheaderstring = protectedheader.dump();
			std::vector<std::byte> signature = sign_hash(hash_type().process(mpt::byte_cast<mpt::const_byte_span>(mpt::as_span(mpt::transcode<std::string>(mpt::common_encoding::utf8, mpt::encode_base64url(mpt::as_span(protectedheaderstring)) + MPT_USTRING(".") + mpt::encode_base64url(payload))))).result());
			return mpt::encode_base64url(mpt::as_span(protectedheaderstring)) + MPT_USTRING(".") + mpt::encode_base64url(payload) + MPT_USTRING(".") + mpt::encode_base64url(mpt::as_span(signature));
		}

		mpt::ustring jws_sign(mpt::const_byte_span payload) {
			nlohmann::json protectedheader = nlohmann::json::object();
			protectedheader["typ"] = "JWT";
			protectedheader["alg"] = jwk_alg;
			std::string protectedheaderstring = protectedheader.dump();
			nlohmann::json header = nlohmann::json::object();
			header["kid"] = name;
			std::vector<std::byte> signature = sign_hash(hash_type().process(mpt::byte_cast<mpt::const_byte_span>(mpt::as_span(mpt::transcode<std::string>(mpt::common_encoding::utf8, mpt::encode_base64url(mpt::as_span(protectedheaderstring)) + MPT_USTRING(".") + mpt::encode_base64url(payload))))).result());
			nlohmann::json jws = nlohmann::json::object();
			jws["payload"] = mpt::encode_base64url(payload);
			jws["signatures"] = nlohmann::json::array();
			nlohmann::json jsignature = nlohmann::json::object();
			jsignature["header"] = header;
			jsignature["protected"] = mpt::encode_base64url(mpt::as_span(protectedheaderstring));
			jsignature["signature"] = mpt::encode_base64url(mpt::as_span(signature));
			jws["signatures"].push_back(jsignature);
			return mpt::transcode<mpt::ustring>(mpt::common_encoding::utf8, jws.dump());
		}
	};

}; // class rsassa_pss



} // namespace asymmetric



} // namespace windows
#endif // MPT_CRYPTO_WINDOWS



#if defined(MPT_CRYPTO_CRYPTOPP)
#if MPT_COMPILER_CLANG
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winline-namespace-reopened-noninline"
#endif // MPT_COMPILER_CLANG
namespace cryptopp {
#if MPT_COMPILER_CLANG
#pragma clang diagnostic pop
#endif // MPT_COMPILER_CLANG



namespace asymmetric {



class signature_verification_failed
	: public std::runtime_error {
public:
	signature_verification_failed()
		: std::runtime_error("Signature Verification failed.") {
		return;
	}
};



inline std::vector<mpt::ustring> jws_get_keynames(const mpt::ustring & jws_) {
	std::vector<mpt::ustring> result;
	nlohmann::json jws = nlohmann::json::parse(mpt::transcode<std::string>(mpt::common_encoding::utf8, jws_));
	for (const auto & s : jws["signatures"]) {
		result.push_back(s["header"]["kid"].get<mpt::ustring>());
	}
	return result;
}



struct RSASSA_PSS_SHA512_traits {
	using hash_type = mpt::crypto::hash::SHA512;
	static constexpr const char * jwk_alg = "PS512";
};



template <typename Traits = RSASSA_PSS_SHA512_traits, std::size_t keysize = 4096>
class rsassa_pss {

public:
	using hash_type = typename Traits::hash_type;
	static constexpr const char * jwk_alg = Traits::jwk_alg;

	struct public_key_data {

		mpt::ustring name;
		uint32 length = 0;
		std::vector<std::byte> public_exp;
		std::vector<std::byte> modulus;

		CryptoPP::RSA::PublicKey as_cryptopp_publickey() const {
			CryptoPP::RSA::PublicKey result{};
			result.SetPublicExponent(CryptoPP::Integer(mpt::byte_cast<const CryptoPP::byte *>(public_exp.data()), public_exp.size(), CryptoPP::Integer::UNSIGNED, CryptoPP::ByteOrder::BIG_ENDIAN_ORDER));
			result.SetModulus(CryptoPP::Integer(mpt::byte_cast<const CryptoPP::byte *>(modulus.data()), modulus.size(), CryptoPP::Integer::UNSIGNED, CryptoPP::ByteOrder::BIG_ENDIAN_ORDER));
			return result;
		}

		mpt::ustring as_jwk() const {
			nlohmann::json json = nlohmann::json::object();
			json["kid"] = name;
			json["kty"] = "RSA";
			json["alg"] = jwk_alg;
			json["use"] = "sig";
			json["e"] = mpt::encode_base64url(mpt::as_span(public_exp));
			json["n"] = mpt::encode_base64url(mpt::as_span(modulus));
			return mpt::transcode<mpt::ustring>(mpt::common_encoding::utf8, json.dump());
		}

		static public_key_data from_jwk(const mpt::ustring & jwk) {
			public_key_data result;
			try {
				nlohmann::json json = nlohmann::json::parse(mpt::transcode<std::string>(mpt::common_encoding::utf8, jwk));
				if (json["kty"] != "RSA") {
					throw std::runtime_error("Cannot parse RSA public key JWK.");
				}
				if (json["alg"] != jwk_alg) {
					throw std::runtime_error("Cannot parse RSA public key JWK.");
				}
				if (json["use"] != "sig") {
					throw std::runtime_error("Cannot parse RSA public key JWK.");
				}
				result.name = json["kid"].get<mpt::ustring>();
				result.public_exp = mpt::decode_base64url(json["e"].get<mpt::ustring>());
				result.modulus = mpt::decode_base64url(json["n"].get<mpt::ustring>());
				result.length = mpt::saturate_cast<uint32>(result.modulus.size() * 8);
			} catch (mpt::out_of_memory e) {
				mpt::rethrow_out_of_memory(e);
			} catch (...) {
				throw std::runtime_error("Cannot parse RSA public key JWK.");
			}
			return result;
		}

		static public_key_data from_cryptopp_publickey(const mpt::ustring & name, const CryptoPP::RSA::PublicKey & key) {
			public_key_data result;
			result.name = name;
			result.length = std::max(key.GetPublicExponent().BitCount(), key.GetModulus().BitCount());
			result.public_exp.resize(key.GetPublicExponent().ByteCount(), std::byte{0});
			key.GetPublicExponent().Encode(mpt::byte_cast<CryptoPP::byte *>(result.public_exp.data()), result.public_exp.size(), CryptoPP::Integer::UNSIGNED);
			result.modulus.resize(key.GetModulus().ByteCount(), std::byte{0});
			key.GetModulus().Encode(mpt::byte_cast<CryptoPP::byte *>(result.modulus.data()), result.modulus.size(), CryptoPP::Integer::UNSIGNED);
			return result;
		}
	};



	static std::vector<public_key_data> parse_jwk_set(const mpt::ustring & jwk_set_) {
		std::vector<public_key_data> result;
		nlohmann::json jwk_set = nlohmann::json::parse(mpt::transcode<std::string>(mpt::common_encoding::utf8, jwk_set_));
		for (const auto & k : jwk_set["keys"]) {
			try {
				result.push_back(public_key_data::from_jwk(mpt::transcode<mpt::ustring>(mpt::common_encoding::utf8, k.dump())));
			} catch (...) {
				// nothing
			}
		}
		return result;
	}



	class public_key {

	private:
		mpt::ustring name;
		CryptoPP::RSA::PublicKey key;

	public:
		public_key(const public_key_data & data)
			: name(data.name)
			, key(data.as_cryptopp_publickey()) {
			return;
		}

		public_key(const public_key & other)
			: public_key(other.get_public_key_data()) {
			return;
		}

		public_key & operator=(const public_key & other) {
			if (&other == this) {
				return *this;
			}
			public_key copy(other);
			{
				using std::swap;
				swap(copy.name, name);
				swap(copy.key, key);
			}
			return *this;
		}

		~public_key() = default;

		mpt::ustring get_name() const {
			return name;
		}

		public_key_data get_public_key_data() const {
			return public_key_data::from_cryptopp_publickey(name, key);
		}

		void verify(mpt::const_byte_span payload, const std::vector<std::byte> & signature) {
			if (!CryptoPP::RSASS<CryptoPP::PSS, typename hash_type::traits::cryptopp_type>::Verifier(key).VerifyMessage(mpt::byte_cast<const CryptoPP::byte *>(payload.data()), payload.size(), mpt::byte_cast<const CryptoPP::byte *>(signature.data()), signature.size())) {
				throw signature_verification_failed();
			}
		}

		std::vector<std::byte> jws_verify(const mpt::ustring & jws_) {
			nlohmann::json jws = nlohmann::json::parse(mpt::transcode<std::string>(mpt::common_encoding::utf8, jws_));
			std::vector<std::byte> payload = mpt::decode_base64url(jws["payload"].get<mpt::ustring>());
			nlohmann::json jsignature = nlohmann::json::object();
			bool sigfound = false;
			for (const auto & s : jws["signatures"]) {
				if (s["header"]["kid"] == mpt::transcode<std::string>(mpt::common_encoding::utf8, name)) {
					jsignature = s;
					sigfound = true;
				}
			}
			if (!sigfound) {
				throw signature_verification_failed();
			}
			std::vector<std::byte> protectedheaderraw = mpt::decode_base64url(jsignature["protected"].get<mpt::ustring>());
			std::vector<std::byte> signature = mpt::decode_base64url(jsignature["signature"].get<mpt::ustring>());
			nlohmann::json header = nlohmann::json::parse(mpt::buffer_cast<std::string>(protectedheaderraw));
			if (header["typ"] != "JWT") {
				throw signature_verification_failed();
			}
			if (header["alg"] != jwk_alg) {
				throw signature_verification_failed();
			}
			verify(mpt::byte_cast<mpt::const_byte_span>(mpt::as_span(mpt::transcode<std::string>(mpt::common_encoding::utf8, mpt::encode_base64url(mpt::as_span(protectedheaderraw)) + MPT_USTRING(".") + mpt::encode_base64url(mpt::as_span(payload))))), signature);
			return payload;
		}

		std::vector<std::byte> jws_compact_verify(const mpt::ustring & jws) {
			std::vector<mpt::ustring> parts = mpt::split<mpt::ustring>(jws, MPT_USTRING("."));
			if (parts.size() != 3) {
				throw signature_verification_failed();
			}
			std::vector<std::byte> protectedheaderraw = mpt::decode_base64url(parts[0]);
			std::vector<std::byte> payload = mpt::decode_base64url(parts[1]);
			std::vector<std::byte> signature = mpt::decode_base64url(parts[2]);
			nlohmann::json header = nlohmann::json::parse(mpt::buffer_cast<std::string>(protectedheaderraw));
			if (header["typ"] != "JWT") {
				throw signature_verification_failed();
			}
			if (header["alg"] != jwk_alg) {
				throw signature_verification_failed();
			}
			verify(mpt::byte_cast<mpt::const_byte_span>(mpt::as_span(mpt::transcode<std::string>(mpt::common_encoding::utf8, mpt::encode_base64url(mpt::as_span(protectedheaderraw)) + MPT_USTRING(".") + mpt::encode_base64url(mpt::as_span(payload))))), signature);
			return payload;
		}
	};



	static inline void jws_verify_at_least_one(std::vector<public_key> & keys, const std::vector<std::byte> & expectedPayload, const mpt::ustring & signature) {
		std::vector<mpt::ustring> keynames = mpt::crypto::asymmetric::jws_get_keynames(signature);
		bool sigchecked = false;
		for (const auto & keyname : keynames) {
			for (auto & key : keys) {
				if (key.get_name() == keyname) {
					if (expectedPayload != key.jws_verify(signature)) {
						throw mpt::crypto::asymmetric::signature_verification_failed();
					}
					sigchecked = true;
				}
			}
		}
		if (!sigchecked) {
			throw mpt::crypto::asymmetric::signature_verification_failed();
		}
	}



	static inline std::vector<std::byte> jws_verify_at_least_one(std::vector<public_key> & keys, const mpt::ustring & signature) {
		std::vector<mpt::ustring> keynames = mpt::crypto::asymmetric::jws_get_keynames(signature);
		for (const auto & keyname : keynames) {
			for (auto & key : keys) {
				if (key.get_name() == keyname) {
					return key.jws_verify(signature);
				}
			}
		}
		throw mpt::crypto::asymmetric::signature_verification_failed();
	}



}; // class rsassa_pss



} // namespace asymmetric



} // namespace cryptopp
#endif // MPT_CRYPTO_CRYPTOPP


#endif // MPT_DETECTED_NLOHMANN_JSON



} // namespace crypto



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_CRYPTO_JWK_HPP
