/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_CRC_CRC_HPP
#define MPT_CRC_CRC_HPP



#include "mpt/base/array.hpp"
#include "mpt/base/detect.hpp"
#include "mpt/base/integer.hpp"
#include "mpt/base/memory.hpp"
#include "mpt/base/namespace.hpp"

#include <array>

#include <cstddef>



namespace mpt {
inline namespace MPT_INLINE_NS {



template <typename T, T polynomial, T initial, T resultXOR, bool reverseData>
class crc {

public:
	using self_type = crc;
	using value_type = T;
	using byte_type = uint8;

	static constexpr std::size_t size_bytes = sizeof(value_type);
	static constexpr std::size_t size_bits = sizeof(value_type) * 8;
	static constexpr value_type top_bit = static_cast<value_type>(1) << ((sizeof(value_type) * 8) - 1);

private:
	template <typename Tint>
	static constexpr Tint reverse(Tint value) noexcept {
		const std::size_t bits = sizeof(Tint) * 8;
		Tint result = 0;
		for (std::size_t i = 0; i < bits; ++i) {
			result <<= 1;
			result |= static_cast<Tint>(value & 0x1);
			value >>= 1;
		}
		return result;
	}

	static constexpr value_type calculate_table_entry(byte_type pos) noexcept {
		value_type value = 0;
		value = (static_cast<value_type>(reverseData ? reverse(pos) : pos) << (size_bits - 8));
		for (std::size_t bit = 0; bit < 8; ++bit) {
			if (value & top_bit) {
				value = (value << 1) ^ polynomial;
			} else {
				value = (value << 1);
			}
		}
		value = (reverseData ? reverse(value) : value);
		return value;
	}

private:
	static constexpr std::array<value_type, 256> calculate_table() noexcept {
		std::array<value_type, 256> t = mpt::init_array<value_type, 256>(value_type{});
		for (std::size_t i = 0; i < 256; ++i) {
			t[i] = calculate_table_entry(static_cast<byte_type>(i));
		}
		return t;
	}

	static constexpr std::array<value_type, 256> table = calculate_table();

private:
	constexpr value_type read_table(byte_type pos) const noexcept {
		return table[pos];
	}

private:
	value_type value;

public:
	constexpr crc() noexcept
		: value(initial) {
		return;
	}

	constexpr void processByte(byte_type byte) noexcept {
		if constexpr (reverseData) {
			value = (value >> 8) ^ read_table(static_cast<byte_type>((value & 0xff) ^ byte));
		} else {
			value = (value << 8) ^ read_table(static_cast<byte_type>(((value >> (size_bits - 8)) & 0xff) ^ byte));
		}
	}

	constexpr value_type result() const noexcept {
		return (value ^ resultXOR);
	}

public:
	constexpr operator value_type() const noexcept {
		return result();
	}

	inline crc & process(char c) noexcept {
		processByte(mpt::byte_cast<byte_type>(c));
		return *this;
	}

	inline crc & process(signed char c) noexcept {
		processByte(static_cast<byte_type>(c));
		return *this;
	}

	inline crc & process(unsigned char c) noexcept {
		processByte(mpt::byte_cast<byte_type>(c));
		return *this;
	}

	inline crc & process(std::byte c) noexcept {
		processByte(mpt::byte_cast<byte_type>(c));
		return *this;
	}

	template <typename InputIt>
	inline crc & process(InputIt beg, InputIt end) {
		for (InputIt it = beg; it != end; ++it) {
			static_assert(sizeof(*it) == 1, "1 byte type required");
			process(*it);
		}
		return *this;
	}

	template <typename Container>
	inline crc & process(const Container & data) {
		operator()(data.begin(), data.end());
		return *this;
	}

	inline crc & operator()(char c) noexcept {
		processByte(mpt::byte_cast<byte_type>(c));
		return *this;
	}

	inline crc & operator()(signed char c) noexcept {
		processByte(static_cast<byte_type>(c));
		return *this;
	}

	inline crc & operator()(unsigned char c) noexcept {
		processByte(mpt::byte_cast<byte_type>(c));
		return *this;
	}

	inline crc & operator()(std::byte c) noexcept {
		processByte(mpt::byte_cast<byte_type>(c));
		return *this;
	}

	template <typename InputIt>
	crc & operator()(InputIt beg, InputIt end) {
		for (InputIt it = beg; it != end; ++it) {
			static_assert(sizeof(*it) == 1, "1 byte type required");
			operator()(*it);
		}
		return *this;
	}

	template <typename Container>
	inline crc & operator()(const Container & data) {
		operator()(data.begin(), data.end());
		return *this;
	}

	template <typename InputIt>
	crc(InputIt beg, InputIt end)
		: value(initial) {
		for (InputIt it = beg; it != end; ++it) {
			static_assert(sizeof(*it) == 1, "1 byte type required");
			process(*it);
		}
	}

	template <typename Container>
	inline crc(const Container & data)
		: value(initial) {
		process(data.begin(), data.end());
	}
};

using crc16 = crc<uint16, 0x8005, 0, 0, true>;
using crc32 = crc<uint32, 0x04C11DB7, 0xFFFFFFFF, 0xFFFFFFFF, true>;
using crc32_ogg = crc<uint32, 0x04C11DB7, 0, 0, false>;
using crc32c = crc<uint32, 0x1EDC6F41, 0xFFFFFFFF, 0xFFFFFFFF, true>;
using crc64_jones = crc<uint64, 0xAD93D23594C935A9ull, 0xFFFFFFFFFFFFFFFFull, 0, true>;


} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_CRC_CRC_HPP
