/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_BASE_BIT_HPP
#define MPT_BASE_BIT_HPP



#include "mpt/base/detect.hpp"
#include "mpt/base/integer.hpp"
#include "mpt/base/namespace.hpp"
#include "mpt/base/macros.hpp"

#if MPT_CXX_BEFORE(20)
#include <array>
#endif // !C++20
#if defined(MPT_COMPILER_QUIRK_BROKEN_BITCAST)
#include <atomic>
#endif
#if MPT_CXX_AT_LEAST(20)
#include <bit>
#endif // C++20
#if MPT_CXX_BEFORE(23) || MPT_COMPILER_MSVC || MPT_LIBCXX_GNU_BEFORE(12) || MPT_LIBCXX_LLVM_BEFORE(14000)
#include <limits>
#endif // !C++23
#include <type_traits>

#if MPT_CXX_BEFORE(20) || MPT_LIBCXX_GNU_BEFORE(11) || MPT_LIBCXX_LLVM_BEFORE(14000) || defined(MPT_COMPILER_QUIRK_BROKEN_BITCAST)
#include <cstring>
#endif // !C++20

#if MPT_CXX_BEFORE(23) && MPT_COMPILER_MSVC
#include <intrin.h>
#endif // !C++23



namespace mpt {
inline namespace MPT_INLINE_NS {



#if defined(MPT_COMPILER_QUIRK_BROKEN_BITCAST)
// VS2022 17.6.0 ARM64 gets confused about alignment in std::bit_cast (or equivalent code),
// causing an ICE with LTCG turned on.
// We try to work-around this problem by placing signal fences as an optimization barrier around the (presumably) confused operation.
template <typename Tdst, typename Tsrc>
MPT_FORCEINLINE typename std::enable_if<(sizeof(Tdst) == sizeof(Tsrc)) && std::is_trivially_copyable<Tsrc>::value && std::is_trivially_copyable<Tdst>::value, Tdst>::type bit_cast(const Tsrc & src) noexcept {
	Tdst dst{};
	std::atomic_signal_fence(std::memory_order_seq_cst);
	std::memcpy(&dst, &src, sizeof(Tdst));
	std::atomic_signal_fence(std::memory_order_seq_cst);
	return dst;
}
#elif MPT_CXX_AT_LEAST(20) && !MPT_LIBCXX_GNU_BEFORE(11) && !MPT_LIBCXX_LLVM_BEFORE(14000)
using std::bit_cast;
#else  // !C++20
// C++2a compatible bit_cast.
// Not implementing constexpr because this is not easily possible pre C++20.
template <typename Tdst, typename Tsrc>
MPT_FORCEINLINE typename std::enable_if<(sizeof(Tdst) == sizeof(Tsrc)) && std::is_trivially_copyable<Tsrc>::value && std::is_trivially_copyable<Tdst>::value, Tdst>::type bit_cast(const Tsrc & src) noexcept {
	Tdst dst{};
	std::memcpy(&dst, &src, sizeof(Tdst));
	return dst;
}
#endif // C++20



#if MPT_CXX_AT_LEAST(20)

using std::endian;

static_assert(mpt::endian::big != mpt::endian::little, "platform with all scalar types having size 1 is not supported");

constexpr mpt::endian get_endian() noexcept {
	return mpt::endian::native;
}

constexpr bool endian_is_little() noexcept {
	return get_endian() == mpt::endian::little;
}

constexpr bool endian_is_big() noexcept {
	return get_endian() == mpt::endian::big;
}

constexpr bool endian_is_weird() noexcept {
	return !endian_is_little() && !endian_is_big();
}

#else // !C++20

#if MPT_COMPILER_MSVC
// same definition as VS2022 C++20 in order to be compatible with debugvis
enum class endian {
	little = 0,
	big = 1,
	weird = -1,
	native = little,
};
#else // !MPT_COMPILER_MSVC
enum class endian {
	little = 0x78563412u,
	big = 0x12345678u,
	weird = 1u,
#if MPT_COMPILER_GENERIC
	native = 0u,
#elif defined(MPT_ARCH_LITTLE_ENDIAN)
	native = little,
#elif defined(MPT_ARCH_BIG_ENDIAN)
	native = big,
#else
	native = 0u,
#endif
};
#endif // MPT_COMPILER_MSVC

static_assert(mpt::endian::big != mpt::endian::little, "platform with all scalar types having size 1 is not supported");

MPT_FORCEINLINE mpt::endian endian_probe() noexcept {
	using endian_probe_type = uint32;
	static_assert(sizeof(endian_probe_type) == 4);
	constexpr endian_probe_type endian_probe_big = 0x12345678u;
	constexpr endian_probe_type endian_probe_little = 0x78563412u;
	const std::array<std::byte, sizeof(endian_probe_type)> probe{
		{std::byte{0x12}, std::byte{0x34}, std::byte{0x56}, std::byte{0x78}}
    };
	const endian_probe_type test = mpt::bit_cast<endian_probe_type>(probe);
	mpt::endian result = mpt::endian::native;
	switch (test) {
		case endian_probe_big:
			result = mpt::endian::big;
			break;
		case endian_probe_little:
			result = mpt::endian::little;
			break;
		default:
			result = mpt::endian::weird;
			break;
	}
	return result;
}

MPT_FORCEINLINE mpt::endian get_endian() noexcept {
#if MPT_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable : 6285) // false-positive: (<non-zero constant> || <non-zero constant>) is always a non-zero constant.
#endif                          // MPT_COMPILER_MSVC
	if constexpr ((mpt::endian::native == mpt::endian::little) || (mpt::endian::native == mpt::endian::big)) {
		return mpt::endian::native;
	} else {
		return mpt::endian_probe();
	}
#if MPT_COMPILER_MSVC
#pragma warning(pop)
#endif // MPT_COMPILER_MSVC
}

MPT_FORCEINLINE bool endian_is_little() noexcept {
	return get_endian() == mpt::endian::little;
}

MPT_FORCEINLINE bool endian_is_big() noexcept {
	return get_endian() == mpt::endian::big;
}

MPT_FORCEINLINE bool endian_is_weird() noexcept {
	return !endian_is_little() && !endian_is_big();
}

#endif // C++20



#if MPT_CXX_AT_LEAST(20) && MPT_MSVC_AT_LEAST(2022, 1) && !MPT_LIBCXX_GNU_BEFORE(10) && !MPT_LIBCXX_LLVM_BEFORE(12000)

// Disabled for VS2022.0 because of
// <https://developercommunity.visualstudio.com/t/vs2022-cl-193030705-generates-non-universally-avai/1578571>
// / <https://github.com/microsoft/STL/issues/2330> with fix already queued
// (<https://github.com/microsoft/STL/pull/2333>).

using std::bit_ceil;
using std::bit_floor;
using std::bit_width;
using std::countl_one;
using std::countl_zero;
using std::countr_one;
using std::countr_zero;
using std::has_single_bit;
using std::popcount;
using std::rotl;
using std::rotr;

#else // !C++20

// C++20 <bit> header.
// Note that we do not use SFINAE here but instead rely on static_assert.

template <typename T>
constexpr int popcount(T val) noexcept {
	static_assert(std::numeric_limits<T>::is_integer);
	static_assert(std::is_unsigned<T>::value);
	int result = 0;
	while (val > 0) {
		if (val & 0x1) {
			result++;
		}
		val >>= 1;
	}
	return result;
}

template <typename T>
constexpr bool has_single_bit(T x) noexcept {
	static_assert(std::numeric_limits<T>::is_integer);
	static_assert(std::is_unsigned<T>::value);
	return mpt::popcount(x) == 1;
}

template <typename T>
constexpr T bit_ceil(T x) noexcept {
	static_assert(std::numeric_limits<T>::is_integer);
	static_assert(std::is_unsigned<T>::value);
	T result = 1;
	while (result < x) {
		T newresult = result << 1;
		if (newresult < result) {
			return 0;
		}
		result = newresult;
	}
	return result;
}

template <typename T>
constexpr T bit_floor(T x) noexcept {
	static_assert(std::numeric_limits<T>::is_integer);
	static_assert(std::is_unsigned<T>::value);
	if (x == 0) {
		return 0;
	}
	T result = 1;
	do {
		T newresult = result << 1;
		if (newresult < result) {
			return result;
		}
		result = newresult;
	} while (result <= x);
	return result >> 1;
}

template <typename T>
constexpr int bit_width(T x) noexcept {
	static_assert(std::numeric_limits<T>::is_integer);
	static_assert(std::is_unsigned<T>::value);
	int result = 0;
	while (x > 0) {
		x >>= 1;
		result += 1;
	}
	return result;
}

template <typename T>
constexpr int countl_zero(T x) noexcept {
	static_assert(std::numeric_limits<T>::is_integer);
	static_assert(std::is_unsigned<T>::value);
	int count = 0;
	for (int bit = std::numeric_limits<T>::digits - 1; bit >= 0; --bit) {
		if ((x & (1u << bit)) == 0u) {
			count++;
		} else {
			break;
		}
	}
	return count;
}

template <typename T>
constexpr int countl_one(T x) noexcept {
	static_assert(std::numeric_limits<T>::is_integer);
	static_assert(std::is_unsigned<T>::value);
	int count = 0;
	for (int bit = std::numeric_limits<T>::digits - 1; bit >= 0; --bit) {
		if ((x & (1u << bit)) != 0u) {
			count++;
		} else {
			break;
		}
	}
	return count;
}

template <typename T>
constexpr int countr_zero(T x) noexcept {
	static_assert(std::numeric_limits<T>::is_integer);
	static_assert(std::is_unsigned<T>::value);
	int count = 0;
	for (int bit = 0; bit < std::numeric_limits<T>::digits; ++bit) {
		if ((x & (1u << bit)) == 0u) {
			count++;
		} else {
			break;
		}
	}
	return count;
}

template <typename T>
constexpr int countr_one(T x) noexcept {
	static_assert(std::numeric_limits<T>::is_integer);
	static_assert(std::is_unsigned<T>::value);
	int count = 0;
	for (int bit = 0; bit < std::numeric_limits<T>::digits; ++bit) {
		if ((x & (1u << bit)) != 0u) {
			count++;
		} else {
			break;
		}
	}
	return count;
}

template <typename T>
constexpr T rotl_impl(T x, int r) noexcept {
	auto N = std::numeric_limits<T>::digits;
	return (x >> (N - r)) | (x << r);
}

template <typename T>
constexpr T rotr_impl(T x, int r) noexcept {
	auto N = std::numeric_limits<T>::digits;
	return (x << (N - r)) | (x >> r);
}

template <typename T>
constexpr T rotl(T x, int s) noexcept {
	static_assert(std::numeric_limits<T>::is_integer);
	static_assert(std::is_unsigned<T>::value);
	auto N = std::numeric_limits<T>::digits;
	auto r = s % N;
	return (s < 0) ? mpt::rotr_impl(x, -s) : ((x >> (N - r)) | (x << r));
}

template <typename T>
constexpr T rotr(T x, int s) noexcept {
	static_assert(std::numeric_limits<T>::is_integer);
	static_assert(std::is_unsigned<T>::value);
	auto N = std::numeric_limits<T>::digits;
	auto r = s % N;
	return (s < 0) ? mpt::rotl_impl(x, -s) : ((x << (N - r)) | (x >> r));
}

#endif // C++20



#if MPT_CXX_AT_LEAST(23) && !MPT_LIBCXX_GNU_BEFORE(12) && !MPT_LIBCXX_LLVM_BEFORE(14000) && !MPT_MSVC_BEFORE(2022, 1)

using std::byteswap;

#else // !C++23

constexpr inline uint16 byteswap_impl_constexpr16(uint16 x) noexcept {
#if MPT_COMPILER_GCC
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#endif // MPT_COMPILER_GCC
	return uint16(0)
		 | ((x >> 8) & 0x00FFu)
		 | ((x << 8) & 0xFF00u);
#if MPT_COMPILER_GCC
#pragma GCC diagnostic pop
#endif // MPT_COMPILER_GCC
}

constexpr inline uint32 byteswap_impl_constexpr32(uint32 x) noexcept {
	return uint32(0)
		 | ((x & 0x000000FFu) << 24)
		 | ((x & 0x0000FF00u) << 8)
		 | ((x & 0x00FF0000u) >> 8)
		 | ((x & 0xFF000000u) >> 24);
}

constexpr inline uint64 byteswap_impl_constexpr64(uint64 x) noexcept {
	return uint64(0)
		 | (((x >> 0) & 0xffull) << 56)
		 | (((x >> 8) & 0xffull) << 48)
		 | (((x >> 16) & 0xffull) << 40)
		 | (((x >> 24) & 0xffull) << 32)
		 | (((x >> 32) & 0xffull) << 24)
		 | (((x >> 40) & 0xffull) << 16)
		 | (((x >> 48) & 0xffull) << 8)
		 | (((x >> 56) & 0xffull) << 0);
}

#if MPT_COMPILER_GCC
// Clang also supports these,
// however <https://github.com/llvm/llvm-project/issues/58470>.
#define MPT_byteswap_impl16 __builtin_bswap16
#define MPT_byteswap_impl32 __builtin_bswap32
#define MPT_byteswap_impl64 __builtin_bswap64
#elif MPT_COMPILER_MSVC
#define MPT_byteswap_impl16 _byteswap_ushort
#define MPT_byteswap_impl32 _byteswap_ulong
#define MPT_byteswap_impl64 _byteswap_uint64
#endif

// No intrinsics available
#ifndef MPT_byteswap_impl16
#define MPT_byteswap_impl16(x) byteswap_impl_constexpr16(x)
#endif
#ifndef MPT_byteswap_impl32
#define MPT_byteswap_impl32(x) byteswap_impl_constexpr32(x)
#endif
#ifndef MPT_byteswap_impl64
#define MPT_byteswap_impl64(x) byteswap_impl_constexpr64(x)
#endif

MPT_CONSTEXPR20_FUN uint64 byteswap_impl(uint64 value) noexcept {
	MPT_MAYBE_CONSTANT_IF (MPT_IS_CONSTANT_EVALUATED20()) {
		return byteswap_impl_constexpr64(value);
	} else {
		return MPT_byteswap_impl64(value);
	}
}

MPT_CONSTEXPR20_FUN uint32 byteswap_impl(uint32 value) noexcept {
	MPT_MAYBE_CONSTANT_IF (MPT_IS_CONSTANT_EVALUATED20()) {
		return byteswap_impl_constexpr32(value);
	} else {
		return MPT_byteswap_impl32(value);
	}
}

MPT_CONSTEXPR20_FUN uint16 byteswap_impl(uint16 value) noexcept {
	MPT_MAYBE_CONSTANT_IF (MPT_IS_CONSTANT_EVALUATED20()) {
		return byteswap_impl_constexpr16(value);
	} else {
		return MPT_byteswap_impl16(value);
	}
}

MPT_CONSTEXPR20_FUN int64 byteswap_impl(int64 value) noexcept {
	MPT_MAYBE_CONSTANT_IF (MPT_IS_CONSTANT_EVALUATED20()) {
		return byteswap_impl_constexpr64(value);
	} else {
		return MPT_byteswap_impl64(value);
	}
}

MPT_CONSTEXPR20_FUN int32 byteswap_impl(int32 value) noexcept {
	MPT_MAYBE_CONSTANT_IF (MPT_IS_CONSTANT_EVALUATED20()) {
		return byteswap_impl_constexpr32(value);
	} else {
		return MPT_byteswap_impl32(value);
	}
}

MPT_CONSTEXPR20_FUN int16 byteswap_impl(int16 value) noexcept {
	MPT_MAYBE_CONSTANT_IF (MPT_IS_CONSTANT_EVALUATED20()) {
		return byteswap_impl_constexpr16(value);
	} else {
		return MPT_byteswap_impl16(value);
	}
}

// Do NOT remove these overloads, even if they seem useless.
// We do not want risking to extend 8bit integers to int and then
// endian-converting and casting back to int.
// Thus these overloads.

constexpr inline uint8 byteswap_impl(uint8 value) noexcept {
	return value;
}

constexpr inline int8 byteswap_impl(int8 value) noexcept {
	return value;
}

constexpr inline char byteswap_impl(char value) noexcept {
	return value;
}

#undef MPT_byteswap_impl16
#undef MPT_byteswap_impl32
#undef MPT_byteswap_impl64

template <typename T>
constexpr T byteswap(T x) noexcept {
	static_assert(std::numeric_limits<T>::is_integer);
	return byteswap_impl(x);
}

#endif // C++23



template <typename T>
constexpr int lower_bound_entropy_bits(T x_) {
	typename std::make_unsigned<T>::type x = static_cast<typename std::make_unsigned<T>::type>(x_);
	return (static_cast<unsigned int>(mpt::bit_width(x)) == static_cast<typename std::make_unsigned<T>::type>(mpt::popcount(x))) ? mpt::bit_width(x) : mpt::bit_width(x) - 1;
}


template <typename T>
constexpr bool is_mask(T x) {
	static_assert(std::is_integral<T>::value);
	typedef typename std::make_unsigned<T>::type unsigned_T;
	unsigned_T ux = static_cast<unsigned_T>(x);
	unsigned_T mask = 0;
	for (std::size_t bits = 0; bits <= (sizeof(unsigned_T) * 8); ++bits) {
		mask = (mask << 1) | 1u;
		if (ux == mask) {
			return true;
		}
	}
	return false;
}



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_BASE_BIT_HPP
