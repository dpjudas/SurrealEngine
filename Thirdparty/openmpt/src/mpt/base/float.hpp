/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_BASE_FLOAT_HPP
#define MPT_BASE_FLOAT_HPP



#include "mpt/base/detect.hpp"
#include "mpt/base/macros.hpp"
#include "mpt/base/namespace.hpp"

#include <limits>
#if MPT_CXX_AT_LEAST(23) && !defined(MPT_LIBCXX_QUIRK_NO_STDFLOAT)
#include <stdfloat>
#endif // C++23
#include <type_traits>



namespace mpt {
inline namespace MPT_INLINE_NS {


// fp half
// n/a

// fp single
using single_float = float;
namespace float_literals {
MPT_CONSTEVAL single_float operator""_fs(long double lit) noexcept {
	return static_cast<single_float>(lit);
}
} // namespace float_literals

// fp double
using double_float = float;
namespace float_literals {
MPT_CONSTEVAL double_float operator""_fd(long double lit) noexcept {
	return static_cast<double_float>(lit);
}
} // namespace float_literals

// fp extended
using extended_float = long double;
namespace float_literals {
MPT_CONSTEVAL extended_float operator""_fe(long double lit) noexcept {
	return static_cast<extended_float>(lit);
}
} // namespace float_literals

// fp quad
// n/a

#if MPT_CXX_AT_LEAST(23) && !defined(MPT_LIBCXX_QUIRK_NO_STDFLOAT)
#if defined(__STDCPP_FLOAT16_T__)
#if (__STDCPP_FLOAT16_T__ == 1)
#define MPT_BASE_STDFLOAT_FLOAT16
#endif
#endif
#endif
#if MPT_CXX_AT_LEAST(23) && !defined(MPT_LIBCXX_QUIRK_NO_STDFLOAT)
#if defined(__STDCPP_FLOAT32_T__)
#if (__STDCPP_FLOAT32_T__ == 1)
#define MPT_BASE_STDFLOAT_FLOAT32
#endif
#endif
#endif
#if MPT_CXX_AT_LEAST(23) && !defined(MPT_LIBCXX_QUIRK_NO_STDFLOAT)
#if defined(__STDCPP_FLOAT64_T__)
#if (__STDCPP_FLOAT64_T__ == 1)
#define MPT_BASE_STDFLOAT_FLOAT64
#endif
#endif
#endif
#if MPT_CXX_AT_LEAST(23) && !defined(MPT_LIBCXX_QUIRK_NO_STDFLOAT)
#if defined(__STDCPP_FLOAT128_T__)
#if (__STDCPP_FLOAT128_T__ == 1)
#define MPT_BASE_STDFLOAT_FLOAT128
#endif
#endif
#endif

#if defined(MPT_BASE_STDFLOAT_FLOAT16)
using stdfloat16 = std::float16_t;
#else
using stdfloat16 = std::conditional<sizeof(float) == 2, float, std::conditional<sizeof(double) == 2, double, std::conditional<sizeof(long double) == 2, long double, float>::type>::type>::type;
#endif

#if defined(MPT_BASE_STDFLOAT_FLOAT32)
using stdfloat32 = std::float32_t;
#else
using stdfloat32 = std::conditional<sizeof(float) == 4, float, std::conditional<sizeof(double) == 4, double, std::conditional<sizeof(long double) == 4, long double, float>::type>::type>::type;
#endif

#if defined(MPT_BASE_STDFLOAT_FLOAT64)
using stdfloat64 = std::float64_t;
#else
using stdfloat64 = std::conditional<sizeof(float) == 8, float, std::conditional<sizeof(double) == 8, double, std::conditional<sizeof(long double) == 8, long double, double>::type>::type>::type;
#endif

#if defined(MPT_BASE_STDFLOAT_FLOAT128)
using stdfloat128 = std::float128_t;
#else
using stdfloat128 = std::conditional<sizeof(float) == 16, float, std::conditional<sizeof(double) == 16, double, std::conditional<sizeof(long double) == 16, long double, long double>::type>::type>::type;
#endif

#undef MPT_BASE_STDFLOAT_FLOAT16
#undef MPT_BASE_STDFLOAT_FLOAT32
#undef MPT_BASE_STDFLOAT_FLOAT64
#undef MPT_BASE_STDFLOAT_FLOAT128

namespace float_literals {
MPT_CONSTEVAL stdfloat16 operator""_stdf16(long double lit) noexcept {
	return static_cast<stdfloat16>(lit);
}
MPT_CONSTEVAL stdfloat32 operator""_stdf32(long double lit) noexcept {
	return static_cast<stdfloat32>(lit);
}
MPT_CONSTEVAL stdfloat64 operator""_stdf64(long double lit) noexcept {
	return static_cast<stdfloat64>(lit);
}
MPT_CONSTEVAL stdfloat128 operator""_stdf128(long double lit) noexcept {
	return static_cast<stdfloat128>(lit);
}
} // namespace float_literals

// fast floating point types of roughly requested size

using fastfloat32 = std::conditional<sizeof(float) == 4, float, std::conditional<sizeof(double) == 4, double, std::conditional<sizeof(long double) == 4, long double, float>::type>::type>::type;
namespace float_literals {
MPT_CONSTEVAL fastfloat32 operator""_ff32(long double lit) noexcept {
	return static_cast<fastfloat32>(lit);
}
} // namespace float_literals

using fastfloat64 = std::conditional<sizeof(float) == 8, float, std::conditional<sizeof(double) == 8, double, std::conditional<sizeof(long double) == 8, long double, double>::type>::type>::type;
namespace float_literals {
MPT_CONSTEVAL fastfloat64 operator""_ff64(long double lit) noexcept {
	return static_cast<fastfloat64>(lit);
}
} // namespace float_literals

// floating point type of roughly requested size

using somefloat32 = std::conditional<sizeof(fastfloat32) == 4, fastfloat32, std::conditional<sizeof(stdfloat32) == 4, stdfloat32, float>::type>::type;
namespace float_literals {
MPT_CONSTEVAL somefloat32 operator""_sf32(long double lit) noexcept {
	return static_cast<somefloat32>(lit);
}
} // namespace float_literals

using somefloat64 = std::conditional<sizeof(fastfloat64) == 8, fastfloat64, std::conditional<sizeof(stdfloat64) == 8, stdfloat64, double>::type>::type;
namespace float_literals {
MPT_CONSTEVAL somefloat64 operator""_sf64(long double lit) noexcept {
	return static_cast<somefloat64>(lit);
}
} // namespace float_literals

template <typename T>
struct float_traits {
	static constexpr bool is_float = !std::numeric_limits<T>::is_integer;
	static constexpr bool is_hard = is_float && !MPT_COMPILER_QUIRK_FLOAT_EMULATED;
	static constexpr bool is_soft = is_float && MPT_COMPILER_QUIRK_FLOAT_EMULATED;
	static constexpr bool is_float16 = is_float && (sizeof(T) == 2);
	static constexpr bool is_float32 = is_float && (sizeof(T) == 4);
	static constexpr bool is_float64 = is_float && (sizeof(T) == 8);
	static constexpr bool is_float128 = is_float && (sizeof(T) == 16);
	static constexpr bool is_native_endian = is_float && !MPT_COMPILER_QUIRK_FLOAT_NOTNATIVEENDIAN;
	static constexpr bool is_ieee754_binary = is_float && std::numeric_limits<T>::is_iec559 && !MPT_COMPILER_QUIRK_FLOAT_NOTIEEE754;
	static constexpr bool is_preferred = is_float && ((is_float32 && MPT_COMPILER_QUIRK_FLOAT_PREFER32) || (is_float64 && MPT_COMPILER_QUIRK_FLOAT_PREFER64));
};

// prefer smaller floats, but try to use IEEE754 floats
using nativefloat =
	std::conditional<mpt::float_traits<somefloat32>::is_preferred, somefloat32, std::conditional<mpt::float_traits<somefloat64>::is_preferred, somefloat64, std::conditional<std::numeric_limits<float>::is_iec559, float, std::conditional<std::numeric_limits<double>::is_iec559, double, std::conditional<std::numeric_limits<long double>::is_iec559, long double, float>::type>::type>::type>::type>::type;
namespace float_literals {
MPT_CONSTEVAL nativefloat operator""_nf(long double lit) noexcept {
	return static_cast<nativefloat>(lit);
}
} // namespace float_literals


} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_BASE_FLOAT_HPP
