/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_BINARY_BASE64URL_HPP
#define MPT_BINARY_BASE64URL_HPP



#include "mpt/base/integer.hpp"
#include "mpt/base/memory.hpp"
#include "mpt/base/namespace.hpp"
#include "mpt/string/types.hpp"

#include <array>
#include <stdexcept>
#include <vector>

#include <cstddef>



namespace mpt {
inline namespace MPT_INLINE_NS {



class base64url_parse_error : public std::runtime_error {
public:
	base64url_parse_error()
		: std::runtime_error("invalid Base64URL encoding") {
	}
};


inline constexpr std::array<mpt::uchar, 64> base64url = {
	{MPT_UCHAR('A'), MPT_UCHAR('B'), MPT_UCHAR('C'), MPT_UCHAR('D'), MPT_UCHAR('E'), MPT_UCHAR('F'), MPT_UCHAR('G'), MPT_UCHAR('H'), MPT_UCHAR('I'), MPT_UCHAR('J'), MPT_UCHAR('K'), MPT_UCHAR('L'), MPT_UCHAR('M'), MPT_UCHAR('N'), MPT_UCHAR('O'), MPT_UCHAR('P'),
	 MPT_UCHAR('Q'), MPT_UCHAR('R'), MPT_UCHAR('S'), MPT_UCHAR('T'), MPT_UCHAR('U'), MPT_UCHAR('V'), MPT_UCHAR('W'), MPT_UCHAR('X'), MPT_UCHAR('Y'), MPT_UCHAR('Z'), MPT_UCHAR('a'), MPT_UCHAR('b'), MPT_UCHAR('c'), MPT_UCHAR('d'), MPT_UCHAR('e'), MPT_UCHAR('f'),
	 MPT_UCHAR('g'), MPT_UCHAR('h'), MPT_UCHAR('i'), MPT_UCHAR('j'), MPT_UCHAR('k'), MPT_UCHAR('l'), MPT_UCHAR('m'), MPT_UCHAR('n'), MPT_UCHAR('o'), MPT_UCHAR('p'), MPT_UCHAR('q'), MPT_UCHAR('r'), MPT_UCHAR('s'), MPT_UCHAR('t'), MPT_UCHAR('u'), MPT_UCHAR('v'),
	 MPT_UCHAR('w'), MPT_UCHAR('x'), MPT_UCHAR('y'), MPT_UCHAR('z'), MPT_UCHAR('0'), MPT_UCHAR('1'), MPT_UCHAR('2'), MPT_UCHAR('3'), MPT_UCHAR('4'), MPT_UCHAR('5'), MPT_UCHAR('6'), MPT_UCHAR('7'), MPT_UCHAR('8'), MPT_UCHAR('9'), MPT_UCHAR('-'), MPT_UCHAR('_')}
};

template <typename Tbyte>
inline mpt::ustring encode_base64url(mpt::span<Tbyte> src_) {
	mpt::const_byte_span src = mpt::byte_cast<mpt::const_byte_span>(src_);
	mpt::ustring result;
	result.reserve(4 * ((src.size() + 2) / 3));
	uint32 bits = 0;
	std::size_t bytes = 0;
	for (std::byte byte : src) {
		bits <<= 8;
		bits |= mpt::byte_cast<uint8>(byte);
		bytes++;
		if (bytes == 3) {
			result.push_back(base64url[(bits >> 18) & 0x3f]);
			result.push_back(base64url[(bits >> 12) & 0x3f]);
			result.push_back(base64url[(bits >> 6) & 0x3f]);
			result.push_back(base64url[(bits >> 0) & 0x3f]);
			bits = 0;
			bytes = 0;
		}
	}
	std::size_t padding = 0;
	while (bytes != 0) {
		bits <<= 8;
		padding++;
		bytes++;
		if (bytes == 3) {
			result.push_back(base64url[(bits >> 18) & 0x3f]);
			result.push_back(base64url[(bits >> 12) & 0x3f]);
			if (padding <= 1) {
				result.push_back(base64url[(bits >> 6) & 0x3f]);
			}
			if (padding <= 0) {
				result.push_back(base64url[(bits >> 0) & 0x3f]);
			}
			bits = 0;
			bytes = 0;
		}
	}
	return result;
}

inline uint8 decode_base64url_bits(mpt::uchar c) {
	for (uint8 i = 0; i < 64; ++i)
	{
		if (base64url[i] == c)
		{
			return i;
		}
	}
	throw base64url_parse_error();
}

inline std::vector<std::byte> decode_base64url(const mpt::ustring & src) {
	std::vector<std::byte> result;
	result.reserve(3 * ((src.length() + 2) / 4));
	uint32 bits = 0;
	std::size_t chars = 0;
	for (mpt::uchar c : src) {
		bits <<= 6;
		bits |= decode_base64url_bits(c);
		chars++;
		if (chars == 4) {
			result.push_back(mpt::byte_cast<std::byte>(static_cast<uint8>((bits >> 16) & 0xff)));
			result.push_back(mpt::byte_cast<std::byte>(static_cast<uint8>((bits >> 8) & 0xff)));
			result.push_back(mpt::byte_cast<std::byte>(static_cast<uint8>((bits >> 0) & 0xff)));
			bits = 0;
			chars = 0;
		}
	}
	uint32 padding = 0;
	if (chars != 0 && chars < 2) {
		throw base64url_parse_error();
	}
	while (chars != 0) {
		bits <<= 6;
		padding++;
		chars++;
		if (chars == 4) {
			result.push_back(mpt::byte_cast<std::byte>(static_cast<uint8>((bits >> 16) & 0xff)));
			if (padding < 2) {
				result.push_back(mpt::byte_cast<std::byte>(static_cast<uint8>((bits >> 8) & 0xff)));
			}
			if (padding < 1) {
				result.push_back(mpt::byte_cast<std::byte>(static_cast<uint8>((bits >> 0) & 0xff)));
			}
			bits = 0;
			chars = 0;
			padding = 0;
		}
	}
	return result;
}


} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_BINARY_BASE64URL_HPP
