/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_BINARY_HEX_HPP
#define MPT_BINARY_HEX_HPP



#include "mpt/base/integer.hpp"
#include "mpt/base/memory.hpp"
#include "mpt/base/namespace.hpp"
#include "mpt/string/types.hpp"

#include <array>
#include <vector>

#include <cstddef>



namespace mpt {
inline namespace MPT_INLINE_NS {



inline constexpr std::array<mpt::uchar, 16> encode_nibble = {
	{MPT_UCHAR('0'), MPT_UCHAR('1'), MPT_UCHAR('2'), MPT_UCHAR('3'),
	 MPT_UCHAR('4'), MPT_UCHAR('5'), MPT_UCHAR('6'), MPT_UCHAR('7'),
	 MPT_UCHAR('8'), MPT_UCHAR('9'), MPT_UCHAR('A'), MPT_UCHAR('B'),
	 MPT_UCHAR('C'), MPT_UCHAR('D'), MPT_UCHAR('E'), MPT_UCHAR('F')}
};


inline bool decode_byte(uint8 & byte, mpt::uchar c1, mpt::uchar c2) {
	byte = 0;
	if (MPT_UCHAR('0') <= c1 && c1 <= MPT_UCHAR('9')) {
		byte += static_cast<uint8>((c1 - MPT_UCHAR('0')) << 4);
	} else if (MPT_UCHAR('A') <= c1 && c1 <= MPT_UCHAR('F')) {
		byte += static_cast<uint8>((c1 - MPT_UCHAR('A') + 10) << 4);
	} else if (MPT_UCHAR('a') <= c1 && c1 <= MPT_UCHAR('f')) {
		byte += static_cast<uint8>((c1 - MPT_UCHAR('a') + 10) << 4);
	} else {
		return false;
	}
	if (MPT_UCHAR('0') <= c2 && c2 <= MPT_UCHAR('9')) {
		byte += static_cast<uint8>(c2 - MPT_UCHAR('0'));
	} else if (MPT_UCHAR('A') <= c2 && c2 <= MPT_UCHAR('F')) {
		byte += static_cast<uint8>(c2 - MPT_UCHAR('A') + 10);
	} else if (MPT_UCHAR('a') <= c2 && c2 <= MPT_UCHAR('f')) {
		byte += static_cast<uint8>(c2 - MPT_UCHAR('a') + 10);
	} else {
		return false;
	}
	return true;
}


template <typename Tbyte>
inline mpt::ustring encode_hex(mpt::span<Tbyte> src_) {
	mpt::const_byte_span src = mpt::byte_cast<mpt::const_byte_span>(src_);
	mpt::ustring result;
	result.reserve(src.size() * 2);
	for (std::byte byte : src) {
		result.push_back(encode_nibble[(mpt::byte_cast<uint8>(byte) & 0xf0) >> 4]);
		result.push_back(encode_nibble[mpt::byte_cast<uint8>(byte) & 0x0f]);
	}
	return result;
}

inline std::vector<std::byte> decode_hex(const mpt::ustring & src) {
	std::vector<std::byte> result;
	result.reserve(src.size() / 2);
	for (std::size_t i = 0; (i + 1) < src.size(); i += 2) {
		uint8 byte = 0;
		if (!decode_byte(byte, src[i], src[i + 1])) {
			return result;
		}
		result.push_back(mpt::byte_cast<std::byte>(byte));
	}
	return result;
}



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_BINARY_HEX_HPP
