/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_ENDIAN_INTEGER_HPP
#define MPT_ENDIAN_INTEGER_HPP



#include "mpt/base/detect.hpp"
#include "mpt/base/bit.hpp"
#include "mpt/base/integer.hpp"
#include "mpt/base/macros.hpp"
#include "mpt/base/memory.hpp"
#include "mpt/base/namespace.hpp"
#include "mpt/endian/type_traits.hpp"

#include <array>
#include <limits>
#include <type_traits>

#include <cstddef>
#include <cstdint>
#include <cstring>



namespace mpt {
inline namespace MPT_INLINE_NS {



template <typename base_type, mpt::endian endian, typename int_type = base_type>
MPT_CONSTEXPRINLINE std::array<std::byte, sizeof(base_type)> EndianEncodeImpl(base_type val) noexcept {
	static_assert(endian == mpt::endian::little || endian == mpt::endian::big);
	static_assert(std::numeric_limits<int_type>::is_integer);
	using unsigned_int_type = typename std::make_unsigned<int_type>::type;
	unsigned_int_type uval = static_cast<unsigned_int_type>(static_cast<int_type>(val));
	std::array<std::byte, sizeof(base_type)> data{};
	if constexpr (endian == mpt::endian::little) {
		for (std::size_t i = 0; i < sizeof(base_type); ++i) {
			data[i] = static_cast<std::byte>(static_cast<uint8>((uval >> (i * 8)) & 0xffu));
		}
	} else {
		for (std::size_t i = 0; i < sizeof(base_type); ++i) {
			data[(sizeof(base_type) - 1) - i] = static_cast<std::byte>(static_cast<uint8>((uval >> (i * 8)) & 0xffu));
		}
	}
	return data;
}

template <typename base_type, mpt::endian endian, typename int_type = base_type>
MPT_CONSTEXPRINLINE std::array<std::byte, sizeof(base_type)> EndianEncode(base_type val) noexcept {
	return EndianEncodeImpl<base_type, endian, int_type>(val);
}

template <typename base_type, mpt::endian endian, typename int_type = base_type>
MPT_CONSTEXPRINLINE base_type EndianDecodeImpl(std::array<std::byte, sizeof(base_type)> data) noexcept {
	static_assert(endian == mpt::endian::little || endian == mpt::endian::big);
	static_assert(std::numeric_limits<int_type>::is_integer);
	using unsigned_int_type = typename std::make_unsigned<int_type>::type;
	base_type val = base_type();
	unsigned_int_type uval = unsigned_int_type();
	if constexpr (endian == mpt::endian::little) {
		for (std::size_t i = 0; i < sizeof(base_type); ++i) {
			uval |= static_cast<unsigned_int_type>(static_cast<unsigned_int_type>(static_cast<uint8>(data[i])) << (i * 8));
		}
	} else {
		for (std::size_t i = 0; i < sizeof(base_type); ++i) {
			uval |= static_cast<unsigned_int_type>(static_cast<unsigned_int_type>(static_cast<uint8>(data[(sizeof(base_type) - 1) - i])) << (i * 8));
		}
	}
	val = static_cast<base_type>(static_cast<int_type>(uval));
	return val;
}

template <typename base_type, mpt::endian endian, typename int_type = base_type>
MPT_CONSTEXPRINLINE base_type EndianDecode(std::array<std::byte, sizeof(base_type)> data) noexcept {
	return EndianDecodeImpl<base_type, endian, int_type>(data);
}



template <typename T>
struct packed_int_type {
	using type = T;
};

// On-disk integer types with defined endianness and no alignemnt requirements
// Note: To easily debug module loaders (and anything else that uses this
// wrapper struct), you can use the Debugger Visualizers available in
// build/vs/debug/ to conveniently view the wrapped contents.

template <typename T, mpt::endian endian, typename Tint = typename mpt::packed_int_type<T>::type>
struct packed {
public:
	using base_type = T;
	using int_type = Tint;

public:
	std::array<std::byte, sizeof(base_type)> data;

public:
	MPT_CONSTEXPR20_FUN void set(base_type val) noexcept {
		static_assert(std::numeric_limits<int_type>::is_integer);
		MPT_MAYBE_CONSTANT_IF (MPT_IS_CONSTANT_EVALUATED20()) {
			if constexpr (endian == mpt::endian::big) {
				typename std::make_unsigned<int_type>::type uval = val;
				for (std::size_t i = 0; i < sizeof(base_type); ++i) {
					data[i] = static_cast<std::byte>((uval >> (8 * (sizeof(base_type) - 1 - i))) & 0xffu);
				}
			} else {
				typename std::make_unsigned<int_type>::type uval = val;
				for (std::size_t i = 0; i < sizeof(base_type); ++i) {
					data[i] = static_cast<std::byte>((uval >> (8 * i)) & 0xffu);
				}
			}
		} else {
			if constexpr (std::is_integral<base_type>::value && (mpt::endian::native == mpt::endian::little || mpt::endian::native == mpt::endian::big)) {
				if constexpr (mpt::endian::native != endian) {
					val = mpt::byteswap(val);
				}
				std::memcpy(data.data(), &val, sizeof(val));
			} else {
				data = EndianEncode<base_type, endian, int_type>(val);
			}
		}
	}
	MPT_CONSTEXPR20_FUN base_type get() const noexcept {
		static_assert(std::numeric_limits<int_type>::is_integer);
		MPT_MAYBE_CONSTANT_IF (MPT_IS_CONSTANT_EVALUATED20()) {
			if constexpr (endian == mpt::endian::big) {
				typename std::make_unsigned<int_type>::type uval = 0;
				for (std::size_t i = 0; i < sizeof(base_type); ++i) {
					uval |= static_cast<typename std::make_unsigned<int_type>::type>(static_cast<typename std::make_unsigned<int_type>::type>(data[i]) << (8 * (sizeof(base_type) - 1 - i)));
				}
				return static_cast<base_type>(uval);
			} else {
				typename std::make_unsigned<int_type>::type uval = 0;
				for (std::size_t i = 0; i < sizeof(base_type); ++i) {
					uval |= static_cast<typename std::make_unsigned<int_type>::type>(static_cast<typename std::make_unsigned<int_type>::type>(data[i]) << (8 * i));
				}
				return static_cast<base_type>(uval);
			}
		} else {
			if constexpr (std::is_integral<base_type>::value && (mpt::endian::native == mpt::endian::little || mpt::endian::native == mpt::endian::big)) {
				base_type val = base_type();
				std::memcpy(&val, data.data(), sizeof(val));
				if constexpr (mpt::endian::native != endian) {
					val = mpt::byteswap(val);
				}
				return val;
			} else {
				return EndianDecode<base_type, endian, int_type>(data);
			}
		}
	}
	MPT_CONSTEXPR20_FUN packed & operator=(const base_type & val) noexcept {
		set(val);
		return *this;
	}
	MPT_CONSTEXPR20_FUN operator base_type() const noexcept {
		return get();
	}

public:
	MPT_CONSTEXPR20_FUN packed & operator&=(base_type val) noexcept {
		set(get() & val);
		return *this;
	}
	MPT_CONSTEXPR20_FUN packed & operator|=(base_type val) noexcept {
		set(get() | val);
		return *this;
	}
	MPT_CONSTEXPR20_FUN packed & operator^=(base_type val) noexcept {
		set(get() ^ val);
		return *this;
	}
	MPT_CONSTEXPR20_FUN packed & operator+=(base_type val) noexcept {
		set(get() + val);
		return *this;
	}
	MPT_CONSTEXPR20_FUN packed & operator-=(base_type val) noexcept {
		set(get() - val);
		return *this;
	}
	MPT_CONSTEXPR20_FUN packed & operator*=(base_type val) noexcept {
		set(get() * val);
		return *this;
	}
	MPT_CONSTEXPR20_FUN packed & operator/=(base_type val) noexcept {
		set(get() / val);
		return *this;
	}
	MPT_CONSTEXPR20_FUN packed & operator%=(base_type val) noexcept {
		set(get() % val);
		return *this;
	}
	MPT_CONSTEXPR20_FUN packed & operator++() noexcept { // prefix
		set(get() + 1);
		return *this;
	}
	MPT_CONSTEXPR20_FUN packed & operator--() noexcept { // prefix
		set(get() - 1);
		return *this;
	}
	MPT_CONSTEXPR20_FUN base_type operator++(int) noexcept { // postfix
		base_type old = get();
		set(old + 1);
		return old;
	}
	MPT_CONSTEXPR20_FUN base_type operator--(int) noexcept { // postfix
		base_type old = get();
		set(old - 1);
		return old;
	}
};

using int64le = packed<int64, mpt::endian::little>;
using int32le = packed<int32, mpt::endian::little>;
using int16le = packed<int16, mpt::endian::little>;
using int8le = packed<int8, mpt::endian::little>;
using uint64le = packed<uint64, mpt::endian::little>;
using uint32le = packed<uint32, mpt::endian::little>;
using uint16le = packed<uint16, mpt::endian::little>;
using uint8le = packed<uint8, mpt::endian::little>;

using int64be = packed<int64, mpt::endian::big>;
using int32be = packed<int32, mpt::endian::big>;
using int16be = packed<int16, mpt::endian::big>;
using int8be = packed<int8, mpt::endian::big>;
using uint64be = packed<uint64, mpt::endian::big>;
using uint32be = packed<uint32, mpt::endian::big>;
using uint16be = packed<uint16, mpt::endian::big>;
using uint8be = packed<uint8, mpt::endian::big>;

constexpr bool declare_binary_safe(const int64le &) {
	return true;
}
constexpr bool declare_binary_safe(const int32le &) {
	return true;
}
constexpr bool declare_binary_safe(const int16le &) {
	return true;
}
constexpr bool declare_binary_safe(const int8le &) {
	return true;
}
constexpr bool declare_binary_safe(const uint64le &) {
	return true;
}
constexpr bool declare_binary_safe(const uint32le &) {
	return true;
}
constexpr bool declare_binary_safe(const uint16le &) {
	return true;
}
constexpr bool declare_binary_safe(const uint8le &) {
	return true;
}

constexpr bool declare_binary_safe(const int64be &) {
	return true;
}
constexpr bool declare_binary_safe(const int32be &) {
	return true;
}
constexpr bool declare_binary_safe(const int16be &) {
	return true;
}
constexpr bool declare_binary_safe(const int8be &) {
	return true;
}
constexpr bool declare_binary_safe(const uint64be &) {
	return true;
}
constexpr bool declare_binary_safe(const uint32be &) {
	return true;
}
constexpr bool declare_binary_safe(const uint16be &) {
	return true;
}
constexpr bool declare_binary_safe(const uint8be &) {
	return true;
}

static_assert(mpt::check_binary_size<int64le>(8));
static_assert(mpt::check_binary_size<int32le>(4));
static_assert(mpt::check_binary_size<int16le>(2));
static_assert(mpt::check_binary_size<int8le>(1));
static_assert(mpt::check_binary_size<uint64le>(8));
static_assert(mpt::check_binary_size<uint32le>(4));
static_assert(mpt::check_binary_size<uint16le>(2));
static_assert(mpt::check_binary_size<uint8le>(1));

static_assert(mpt::check_binary_size<int64be>(8));
static_assert(mpt::check_binary_size<int32be>(4));
static_assert(mpt::check_binary_size<int16be>(2));
static_assert(mpt::check_binary_size<int8be>(1));
static_assert(mpt::check_binary_size<uint64be>(8));
static_assert(mpt::check_binary_size<uint32be>(4));
static_assert(mpt::check_binary_size<uint16be>(2));
static_assert(mpt::check_binary_size<uint8be>(1));



template <typename T>
struct make_endian<mpt::endian::little, T> {
	using type = packed<typename std::remove_const<T>::type, mpt::endian::little>;
};

template <typename T>
struct make_endian<mpt::endian::big, T> {
	using type = packed<typename std::remove_const<T>::type, mpt::endian::big>;
};



template <typename T>
MPT_CONSTEXPR20_FUN auto as_le(T v) noexcept -> typename mpt::make_le<typename std::remove_const<T>::type>::type {
	typename mpt::make_le<typename std::remove_const<T>::type>::type res{};
	res = v;
	return res;
}

template <typename T>
MPT_CONSTEXPR20_FUN auto as_be(T v) noexcept -> typename mpt::make_be<typename std::remove_const<T>::type>::type {
	typename mpt::make_be<typename std::remove_const<T>::type>::type res{};
	res = v;
	return res;
}



template <typename Tpacked>
MPT_CONSTEXPR20_FUN Tpacked as_endian(typename Tpacked::base_type v) noexcept {
	Tpacked res{};
	res = v;
	return res;
}



} // namespace MPT_INLINE_NS
} // namespace mpt



namespace std {
template <typename T, mpt::endian endian>
class numeric_limits<mpt::packed<T, endian>> : public std::numeric_limits<T> { };
template <typename T, mpt::endian endian>
class numeric_limits<const mpt::packed<T, endian>> : public std::numeric_limits<const T> { };
} // namespace std



#endif // MPT_ENDIAN_INTEGER_HPP
