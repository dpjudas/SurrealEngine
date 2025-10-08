/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_ENDIAN_FLOATINGPOINT_HPP
#define MPT_ENDIAN_FLOATINGPOINT_HPP



#include "mpt/base/bit.hpp"
#include "mpt/base/float.hpp"
#include "mpt/base/macros.hpp"
#include "mpt/base/memory.hpp"
#include "mpt/base/namespace.hpp"
#include "mpt/endian/type_traits.hpp"

#include <limits>

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>



namespace mpt {
inline namespace MPT_INLINE_NS {



// 1.0f --> 0x3f800000u
MPT_FORCEINLINE uint32 EncodeIEEE754binary32(somefloat32 f) {
	if constexpr (mpt::float_traits<somefloat32>::is_float32 && mpt::float_traits<somefloat32>::is_ieee754_binary && mpt::float_traits<somefloat32>::is_native_endian) {
		return mpt::bit_cast<uint32>(f);
	} else {
		int e = 0;
		float m = std::frexp(f, &e);
		if (e == 0 && std::fabs(m) == 0.0f) {
			uint32 expo = 0u;
			uint32 sign = std::signbit(m) ? 0x01u : 0x00u;
			uint32 mant = 0u;
			uint32 i = 0u;
			i |= (mant << 0) & 0x007fffffu;
			i |= (expo << 23) & 0x7f800000u;
			i |= (sign << 31) & 0x80000000u;
			return i;
		} else {
			uint32 expo = e + 127 - 1;
			uint32 sign = std::signbit(m) ? 0x01u : 0x00u;
			uint32 mant = static_cast<uint32>(std::fabs(std::ldexp(m, 24)));
			uint32 i = 0u;
			i |= (mant << 0) & 0x007fffffu;
			i |= (expo << 23) & 0x7f800000u;
			i |= (sign << 31) & 0x80000000u;
			return i;
		}
	}
}

MPT_FORCEINLINE uint64 EncodeIEEE754binary64(somefloat64 f) {
	if constexpr (mpt::float_traits<somefloat64>::is_float64 && mpt::float_traits<somefloat64>::is_ieee754_binary && mpt::float_traits<somefloat64>::is_native_endian) {
		return mpt::bit_cast<uint64>(f);
	} else {
		int e = 0;
		double m = std::frexp(f, &e);
		if (e == 0 && std::fabs(m) == 0.0) {
			uint64 expo = 0u;
			uint64 sign = std::signbit(m) ? 0x01u : 0x00u;
			uint64 mant = 0u;
			uint64 i = 0u;
			i |= (mant << 0) & 0x000fffffffffffffull;
			i |= (expo << 52) & 0x7ff0000000000000ull;
			i |= (sign << 63) & 0x8000000000000000ull;
			return i;
		} else {
			uint64 expo = static_cast<int64>(e) + 1023 - 1;
			uint64 sign = std::signbit(m) ? 0x01u : 0x00u;
			uint64 mant = static_cast<uint64>(std::fabs(std::ldexp(m, 53)));
			uint64 i = 0u;
			i |= (mant << 0) & 0x000fffffffffffffull;
			i |= (expo << 52) & 0x7ff0000000000000ull;
			i |= (sign << 63) & 0x8000000000000000ull;
			return i;
		}
	}
}

// 0x3f800000u --> 1.0f
MPT_FORCEINLINE somefloat32 DecodeIEEE754binary32(uint32 i) {
	if constexpr (mpt::float_traits<somefloat32>::is_float32 && mpt::float_traits<somefloat32>::is_ieee754_binary && mpt::float_traits<somefloat32>::is_native_endian) {
		return mpt::bit_cast<somefloat32>(i);
	} else {
		uint32 mant = (i & 0x007fffffu) >> 0;
		uint32 expo = (i & 0x7f800000u) >> 23;
		uint32 sign = (i & 0x80000000u) >> 31;
		if (expo == 0) {
			float m = sign ? -static_cast<float>(mant) : static_cast<float>(mant);
			int e = static_cast<int>(expo) - 127 + 1 - 24;
			float f = std::ldexp(m, e);
			return static_cast<somefloat32>(f);
		} else {
			mant |= 0x00800000u;
			float m = sign ? -static_cast<float>(mant) : static_cast<float>(mant);
			int e = static_cast<int>(expo) - 127 + 1 - 24;
			float f = std::ldexp(m, e);
			return static_cast<somefloat32>(f);
		}
	}
}

MPT_FORCEINLINE somefloat64 DecodeIEEE754binary64(uint64 i) {
	if constexpr (mpt::float_traits<somefloat64>::is_float64 && mpt::float_traits<somefloat64>::is_ieee754_binary && mpt::float_traits<somefloat64>::is_native_endian) {
		return mpt::bit_cast<somefloat64>(i);
	} else {
		uint64 mant = (i & 0x000fffffffffffffull) >> 0;
		uint64 expo = (i & 0x7ff0000000000000ull) >> 52;
		uint64 sign = (i & 0x8000000000000000ull) >> 63;
		if (expo == 0) {
			double m = sign ? -static_cast<double>(mant) : static_cast<double>(mant);
			int e = static_cast<int>(expo) - 1023 + 1 - 53;
			double f = std::ldexp(m, e);
			return static_cast<somefloat64>(f);
		} else {
			mant |= 0x0010000000000000ull;
			double m = sign ? -static_cast<double>(mant) : static_cast<double>(mant);
			int e = static_cast<int>(expo) - 1023 + 1 - 53;
			double f = std::ldexp(m, e);
			return static_cast<somefloat64>(f);
		}
	}
}


// template parameters are byte indices corresponding to the individual bytes of iee754 in memory
template <std::size_t hihi, std::size_t hilo, std::size_t lohi, std::size_t lolo>
struct IEEE754binary32Emulated {
public:
	using self_t = IEEE754binary32Emulated<hihi, hilo, lohi, lolo>;
	std::byte bytes[4];

public:
	MPT_FORCEINLINE std::byte GetByte(std::size_t i) const {
		return bytes[i];
	}
	IEEE754binary32Emulated() = default;
	MPT_FORCEINLINE explicit IEEE754binary32Emulated(somefloat32 f) {
		SetInt32(EncodeIEEE754binary32(f));
	}
	MPT_FORCEINLINE IEEE754binary32Emulated & operator=(somefloat32 f) {
		SetInt32(EncodeIEEE754binary32(f));
		return *this;
	}
	// b0...b3 are in memory order, i.e. depend on the endianness of this type
	// little endian: (0x00,0x00,0x80,0x3f)
	// big endian:    (0x3f,0x80,0x00,0x00)
	MPT_FORCEINLINE explicit IEEE754binary32Emulated(std::byte b0, std::byte b1, std::byte b2, std::byte b3) {
		bytes[0] = b0;
		bytes[1] = b1;
		bytes[2] = b2;
		bytes[3] = b3;
	}
	MPT_FORCEINLINE operator somefloat32() const {
		return DecodeIEEE754binary32(GetInt32());
	}
	MPT_FORCEINLINE self_t & set(somefloat32 f) {
		SetInt32(EncodeIEEE754binary32(f));
		return *this;
	}
	MPT_FORCEINLINE somefloat32 get() const {
		return DecodeIEEE754binary32(GetInt32());
	}
	MPT_FORCEINLINE self_t & SetInt32(uint32 i) {
		bytes[hihi] = static_cast<std::byte>(i >> 24);
		bytes[hilo] = static_cast<std::byte>(i >> 16);
		bytes[lohi] = static_cast<std::byte>(i >> 8);
		bytes[lolo] = static_cast<std::byte>(i >> 0);
		return *this;
	}
	MPT_FORCEINLINE uint32 GetInt32() const {
		return 0u
			 | (static_cast<uint32>(bytes[hihi]) << 24)
			 | (static_cast<uint32>(bytes[hilo]) << 16)
			 | (static_cast<uint32>(bytes[lohi]) << 8)
			 | (static_cast<uint32>(bytes[lolo]) << 0);
	}
	MPT_FORCEINLINE bool operator==(const self_t & cmp) const {
		return true
			&& bytes[0] == cmp.bytes[0]
			&& bytes[1] == cmp.bytes[1]
			&& bytes[2] == cmp.bytes[2]
			&& bytes[3] == cmp.bytes[3];
	}
	MPT_FORCEINLINE bool operator!=(const self_t & cmp) const {
		return !(*this == cmp);
	}
};
template <std::size_t hihihi, std::size_t hihilo, std::size_t hilohi, std::size_t hilolo, std::size_t lohihi, std::size_t lohilo, std::size_t lolohi, std::size_t lololo>
struct IEEE754binary64Emulated {
public:
	using self_t = IEEE754binary64Emulated<hihihi, hihilo, hilohi, hilolo, lohihi, lohilo, lolohi, lololo>;
	std::byte bytes[8];

public:
	MPT_FORCEINLINE std::byte GetByte(std::size_t i) const {
		return bytes[i];
	}
	IEEE754binary64Emulated() = default;
	MPT_FORCEINLINE explicit IEEE754binary64Emulated(somefloat64 f) {
		SetInt64(EncodeIEEE754binary64(f));
	}
	MPT_FORCEINLINE IEEE754binary64Emulated & operator=(somefloat64 f) {
		SetInt64(EncodeIEEE754binary64(f));
		return *this;
	}
	MPT_FORCEINLINE explicit IEEE754binary64Emulated(std::byte b0, std::byte b1, std::byte b2, std::byte b3, std::byte b4, std::byte b5, std::byte b6, std::byte b7) {
		bytes[0] = b0;
		bytes[1] = b1;
		bytes[2] = b2;
		bytes[3] = b3;
		bytes[4] = b4;
		bytes[5] = b5;
		bytes[6] = b6;
		bytes[7] = b7;
	}
	MPT_FORCEINLINE operator somefloat64() const {
		return DecodeIEEE754binary64(GetInt64());
	}
	MPT_FORCEINLINE self_t & set(somefloat64 f) {
		SetInt64(EncodeIEEE754binary64(f));
		return *this;
	}
	MPT_FORCEINLINE somefloat64 get() const {
		return DecodeIEEE754binary64(GetInt64());
	}
	MPT_FORCEINLINE self_t & SetInt64(uint64 i) {
		bytes[hihihi] = static_cast<std::byte>(i >> 56);
		bytes[hihilo] = static_cast<std::byte>(i >> 48);
		bytes[hilohi] = static_cast<std::byte>(i >> 40);
		bytes[hilolo] = static_cast<std::byte>(i >> 32);
		bytes[lohihi] = static_cast<std::byte>(i >> 24);
		bytes[lohilo] = static_cast<std::byte>(i >> 16);
		bytes[lolohi] = static_cast<std::byte>(i >> 8);
		bytes[lololo] = static_cast<std::byte>(i >> 0);
		return *this;
	}
	MPT_FORCEINLINE uint64 GetInt64() const {
		return 0u
			 | (static_cast<uint64>(bytes[hihihi]) << 56)
			 | (static_cast<uint64>(bytes[hihilo]) << 48)
			 | (static_cast<uint64>(bytes[hilohi]) << 40)
			 | (static_cast<uint64>(bytes[hilolo]) << 32)
			 | (static_cast<uint64>(bytes[lohihi]) << 24)
			 | (static_cast<uint64>(bytes[lohilo]) << 16)
			 | (static_cast<uint64>(bytes[lolohi]) << 8)
			 | (static_cast<uint64>(bytes[lololo]) << 0);
	}
	MPT_FORCEINLINE bool operator==(const self_t & cmp) const {
		return true
			&& bytes[0] == cmp.bytes[0]
			&& bytes[1] == cmp.bytes[1]
			&& bytes[2] == cmp.bytes[2]
			&& bytes[3] == cmp.bytes[3]
			&& bytes[4] == cmp.bytes[4]
			&& bytes[5] == cmp.bytes[5]
			&& bytes[6] == cmp.bytes[6]
			&& bytes[7] == cmp.bytes[7];
	}
	MPT_FORCEINLINE bool operator!=(const self_t & cmp) const {
		return !(*this == cmp);
	}
};

using IEEE754binary32EmulatedBE = IEEE754binary32Emulated<0, 1, 2, 3>;
using IEEE754binary32EmulatedLE = IEEE754binary32Emulated<3, 2, 1, 0>;
using IEEE754binary64EmulatedBE = IEEE754binary64Emulated<0, 1, 2, 3, 4, 5, 6, 7>;
using IEEE754binary64EmulatedLE = IEEE754binary64Emulated<7, 6, 5, 4, 3, 2, 1, 0>;

constexpr bool declare_binary_safe(const IEEE754binary32EmulatedBE &) {
	return true;
}
constexpr bool declare_binary_safe(const IEEE754binary32EmulatedLE &) {
	return true;
}
constexpr bool declare_binary_safe(const IEEE754binary64EmulatedBE &) {
	return true;
}
constexpr bool declare_binary_safe(const IEEE754binary64EmulatedLE &) {
	return true;
}

static_assert(mpt::check_binary_size<IEEE754binary32EmulatedBE>(4));
static_assert(mpt::check_binary_size<IEEE754binary32EmulatedLE>(4));
static_assert(mpt::check_binary_size<IEEE754binary64EmulatedBE>(8));
static_assert(mpt::check_binary_size<IEEE754binary64EmulatedLE>(8));

template <mpt::endian endian = mpt::endian::native>
struct IEEE754binary32Native {
public:
	somefloat32 value;

public:
	MPT_FORCEINLINE std::byte GetByte(std::size_t i) const {
		static_assert(endian == mpt::endian::little || endian == mpt::endian::big);
		if constexpr (endian == mpt::endian::little) {
			return static_cast<std::byte>(EncodeIEEE754binary32(value) >> (i * 8));
		}
		if constexpr (endian == mpt::endian::big) {
			return static_cast<std::byte>(EncodeIEEE754binary32(value) >> ((4 - 1 - i) * 8));
		}
	}
	IEEE754binary32Native() = default;
	MPT_FORCEINLINE explicit IEEE754binary32Native(somefloat32 f) {
		value = f;
	}
	MPT_FORCEINLINE IEEE754binary32Native & operator=(somefloat32 f) {
		value = f;
		return *this;
	}
	// b0...b3 are in memory order, i.e. depend on the endianness of this type
	// little endian: (0x00,0x00,0x80,0x3f)
	// big endian:    (0x3f,0x80,0x00,0x00)
	MPT_FORCEINLINE explicit IEEE754binary32Native(std::byte b0, std::byte b1, std::byte b2, std::byte b3) {
		static_assert(endian == mpt::endian::little || endian == mpt::endian::big);
		if constexpr (endian == mpt::endian::little) {
			value = DecodeIEEE754binary32(0u | (static_cast<uint32>(b0) << 0) | (static_cast<uint32>(b1) << 8) | (static_cast<uint32>(b2) << 16) | (static_cast<uint32>(b3) << 24));
		}
		if constexpr (endian == mpt::endian::big) {
			value = DecodeIEEE754binary32(0u | (static_cast<uint32>(b0) << 24) | (static_cast<uint32>(b1) << 16) | (static_cast<uint32>(b2) << 8) | (static_cast<uint32>(b3) << 0));
		}
	}
	MPT_FORCEINLINE operator somefloat32() const {
		return value;
	}
	MPT_FORCEINLINE IEEE754binary32Native & set(somefloat32 f) {
		value = f;
		return *this;
	}
	MPT_FORCEINLINE somefloat32 get() const {
		return value;
	}
	MPT_FORCEINLINE IEEE754binary32Native & SetInt32(uint32 i) {
		value = DecodeIEEE754binary32(i);
		return *this;
	}
	MPT_FORCEINLINE uint32 GetInt32() const {
		return EncodeIEEE754binary32(value);
	}
	MPT_FORCEINLINE bool operator==(const IEEE754binary32Native & cmp) const {
		return value == cmp.value;
	}
	MPT_FORCEINLINE bool operator!=(const IEEE754binary32Native & cmp) const {
		return value != cmp.value;
	}
};

template <mpt::endian endian = mpt::endian::native>
struct IEEE754binary64Native {
public:
	somefloat64 value;

public:
	MPT_FORCEINLINE std::byte GetByte(std::size_t i) const {
		static_assert(endian == mpt::endian::little || endian == mpt::endian::big);
		if constexpr (endian == mpt::endian::little) {
			return mpt::byte_cast<std::byte>(static_cast<uint8>(EncodeIEEE754binary64(value) >> (i * 8)));
		}
		if constexpr (endian == mpt::endian::big) {
			return mpt::byte_cast<std::byte>(static_cast<uint8>(EncodeIEEE754binary64(value) >> ((8 - 1 - i) * 8)));
		}
	}
	IEEE754binary64Native() = default;
	MPT_FORCEINLINE explicit IEEE754binary64Native(somefloat64 f) {
		value = f;
	}
	MPT_FORCEINLINE IEEE754binary64Native & operator=(somefloat64 f) {
		value = f;
		return *this;
	}
	MPT_FORCEINLINE explicit IEEE754binary64Native(std::byte b0, std::byte b1, std::byte b2, std::byte b3, std::byte b4, std::byte b5, std::byte b6, std::byte b7) {
		static_assert(endian == mpt::endian::little || endian == mpt::endian::big);
		if constexpr (endian == mpt::endian::little) {
			value = DecodeIEEE754binary64(0ull | (static_cast<uint64>(b0) << 0) | (static_cast<uint64>(b1) << 8) | (static_cast<uint64>(b2) << 16) | (static_cast<uint64>(b3) << 24) | (static_cast<uint64>(b4) << 32) | (static_cast<uint64>(b5) << 40) | (static_cast<uint64>(b6) << 48) | (static_cast<uint64>(b7) << 56));
		}
		if constexpr (endian == mpt::endian::big) {
			value = DecodeIEEE754binary64(0ull | (static_cast<uint64>(b0) << 56) | (static_cast<uint64>(b1) << 48) | (static_cast<uint64>(b2) << 40) | (static_cast<uint64>(b3) << 32) | (static_cast<uint64>(b4) << 24) | (static_cast<uint64>(b5) << 16) | (static_cast<uint64>(b6) << 8) | (static_cast<uint64>(b7) << 0));
		}
	}
	MPT_FORCEINLINE operator somefloat64() const {
		return value;
	}
	MPT_FORCEINLINE IEEE754binary64Native & set(somefloat64 f) {
		value = f;
		return *this;
	}
	MPT_FORCEINLINE somefloat64 get() const {
		return value;
	}
	MPT_FORCEINLINE IEEE754binary64Native & SetInt64(uint64 i) {
		value = DecodeIEEE754binary64(i);
		return *this;
	}
	MPT_FORCEINLINE uint64 GetInt64() const {
		return EncodeIEEE754binary64(value);
	}
	MPT_FORCEINLINE bool operator==(const IEEE754binary64Native & cmp) const {
		return value == cmp.value;
	}
	MPT_FORCEINLINE bool operator!=(const IEEE754binary64Native & cmp) const {
		return value != cmp.value;
	}
};

static_assert((sizeof(IEEE754binary32Native<>) == 4));
static_assert((sizeof(IEEE754binary64Native<>) == 8));

constexpr bool declare_binary_safe(const IEEE754binary32Native<> &) noexcept {
	return true;
}
constexpr bool declare_binary_safe(const IEEE754binary64Native<> &) noexcept {
	return true;
}

template <bool is_ieee754, mpt::endian endian = mpt::endian::native>
struct IEEE754binary_types {
	using IEEE754binary32LE = IEEE754binary32EmulatedLE;
	using IEEE754binary32BE = IEEE754binary32EmulatedBE;
	using IEEE754binary64LE = IEEE754binary64EmulatedLE;
	using IEEE754binary64BE = IEEE754binary64EmulatedBE;
};
template <>
struct IEEE754binary_types<true, mpt::endian::little> {
	using IEEE754binary32LE = IEEE754binary32Native<>;
	using IEEE754binary32BE = IEEE754binary32EmulatedBE;
	using IEEE754binary64LE = IEEE754binary64Native<>;
	using IEEE754binary64BE = IEEE754binary64EmulatedBE;
};
template <>
struct IEEE754binary_types<true, mpt::endian::big> {
	using IEEE754binary32LE = IEEE754binary32EmulatedLE;
	using IEEE754binary32BE = IEEE754binary32Native<>;
	using IEEE754binary64LE = IEEE754binary64EmulatedLE;
	using IEEE754binary64BE = IEEE754binary64Native<>;
};

using IEEE754binary32LE = IEEE754binary_types<mpt::float_traits<somefloat32>::is_float32 && mpt::float_traits<somefloat32>::is_ieee754_binary && mpt::float_traits<somefloat32>::is_native_endian, mpt::endian::native>::IEEE754binary32LE;
using IEEE754binary32BE = IEEE754binary_types<mpt::float_traits<somefloat32>::is_float32 && mpt::float_traits<somefloat32>::is_ieee754_binary && mpt::float_traits<somefloat32>::is_native_endian, mpt::endian::native>::IEEE754binary32BE;
using IEEE754binary64LE = IEEE754binary_types<mpt::float_traits<somefloat32>::is_float64 && mpt::float_traits<somefloat32>::is_ieee754_binary && mpt::float_traits<somefloat32>::is_native_endian, mpt::endian::native>::IEEE754binary64LE;
using IEEE754binary64BE = IEEE754binary_types<mpt::float_traits<somefloat32>::is_float64 && mpt::float_traits<somefloat32>::is_ieee754_binary && mpt::float_traits<somefloat32>::is_native_endian, mpt::endian::native>::IEEE754binary64BE;

static_assert(sizeof(IEEE754binary32LE) == 4);
static_assert(sizeof(IEEE754binary32BE) == 4);
static_assert(sizeof(IEEE754binary64LE) == 8);
static_assert(sizeof(IEEE754binary64BE) == 8);


// unaligned

using float32le = IEEE754binary32EmulatedLE;
using float32be = IEEE754binary32EmulatedBE;
using float64le = IEEE754binary64EmulatedLE;
using float64be = IEEE754binary64EmulatedBE;

static_assert(sizeof(float32le) == 4);
static_assert(sizeof(float32be) == 4);
static_assert(sizeof(float64le) == 8);
static_assert(sizeof(float64be) == 8);


// potentially aligned

using float32le_fast = IEEE754binary32LE;
using float32be_fast = IEEE754binary32BE;
using float64le_fast = IEEE754binary64LE;
using float64be_fast = IEEE754binary64BE;

static_assert(sizeof(float32le_fast) == 4);
static_assert(sizeof(float32be_fast) == 4);
static_assert(sizeof(float64le_fast) == 8);
static_assert(sizeof(float64be_fast) == 8);



template <>
struct make_endian<mpt::endian::little, float> {
	using type = IEEE754binary32LE;
};

template <>
struct make_endian<mpt::endian::big, float> {
	using type = IEEE754binary32BE;
};

template <>
struct make_endian<mpt::endian::little, double> {
	using type = IEEE754binary64LE;
};

template <>
struct make_endian<mpt::endian::big, double> {
	using type = IEEE754binary64BE;
};



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_ENDIAN_FLOATINGPOINT_HPP
