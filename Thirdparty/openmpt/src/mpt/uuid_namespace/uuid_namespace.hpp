/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_UUID_NAMESPACE_UUID_NAMESPACE_HPP
#define MPT_UUID_NAMESPACE_UUID_NAMESPACE_HPP



#include "mpt/base/detect.hpp"
#include "mpt/base/memory.hpp"
#include "mpt/base/namespace.hpp"
#include "mpt/crypto/hash.hpp"
#include "mpt/string/types.hpp"
#include "mpt/string_transcode/transcode.hpp"
#include "mpt/uuid/uuid.hpp"

#include <algorithm>
#include <array>
#include <string>
#include <vector>



namespace mpt {
inline namespace MPT_INLINE_NS {



#if MPT_OS_WINDOWS

// Create a RFC4122 Version 3 namespace UUID
inline mpt::UUID UUIDRFC4122NamespaceV3(const mpt::UUID & ns, const mpt::ustring & name) {
	mpt::UUIDbin binns = ns;
	std::vector<std::byte> buf;
	buf.resize(sizeof(mpt::UUIDbin));
	std::copy(mpt::as_raw_memory(binns).data(), mpt::as_raw_memory(binns).data() + sizeof(mpt::UUIDbin), buf.data());
	std::string utf8name = mpt::transcode<std::string>(mpt::common_encoding::utf8, name);
	buf.resize(buf.size() + utf8name.length());
	std::transform(utf8name.begin(), utf8name.end(), buf.data() + sizeof(mpt::UUIDbin), [](char c) { return mpt::byte_cast<std::byte>(c); });
	std::array<std::byte, 16> hash = mpt::crypto::hash::MD5().process(mpt::as_span(buf)).result();
	mpt::UUIDbin uuidbin;
	std::copy(hash.begin(), hash.begin() + 16, mpt::as_raw_memory(uuidbin).data());
	mpt::UUID uuid{uuidbin};
	uuid.MakeRFC4122(3);
	return uuid;
}

// Create a RFC4122 Version 5 namespace UUID
inline mpt::UUID UUIDRFC4122NamespaceV5(const mpt::UUID & ns, const mpt::ustring & name) {
	mpt::UUIDbin binns = ns;
	std::vector<std::byte> buf;
	buf.resize(sizeof(mpt::UUIDbin));
	std::copy(mpt::as_raw_memory(binns).data(), mpt::as_raw_memory(binns).data() + sizeof(mpt::UUIDbin), buf.data());
	std::string utf8name = mpt::transcode<std::string>(mpt::common_encoding::utf8, name);
	buf.resize(buf.size() + utf8name.length());
	std::transform(utf8name.begin(), utf8name.end(), buf.data() + sizeof(mpt::UUIDbin), [](char c) { return mpt::byte_cast<std::byte>(c); });
	std::array<std::byte, 20> hash = mpt::crypto::hash::SHA1().process(mpt::as_span(buf)).result();
	UUIDbin uuidbin;
	std::copy(hash.begin(), hash.begin() + 16, mpt::as_raw_memory(uuidbin).data());
	mpt::UUID uuid{uuidbin};
	uuid.MakeRFC4122(5);
	return uuid;
}

#endif // MPT_OS_WINDOWS



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_UUID_NAMESPACE_UUID_NAMESPACE_HPP
