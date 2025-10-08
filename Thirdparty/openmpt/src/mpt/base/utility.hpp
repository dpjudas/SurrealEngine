/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_BASE_UTILITY_HPP
#define MPT_BASE_UTILITY_HPP



#include "mpt/base/detect.hpp"
#include "mpt/base/namespace.hpp"

#if MPT_CXX_BEFORE(20) || MPT_LIBCXX_LLVM_BEFORE(13000)
#include "mpt/base/saturate_cast.hpp"
#include "mpt/base/saturate_round.hpp"
#endif

#if MPT_CXX_BEFORE(23) && !MPT_COMPILER_MSVC && !MPT_COMPILER_GCC && !MPT_COMPILER_CLANG
#include <exception>
#endif
#include <new>
#include <type_traits>
#include <utility>

#include <cstddef>



namespace mpt {
inline namespace MPT_INLINE_NS {



template <typename Tdst, typename Tsrc>
MPT_CONSTEXPRINLINE Tdst c_cast(Tsrc && x) {
	return (Tdst)std::forward<Tsrc>(x);
}



template <typename Tdst, typename Tsrc>
MPT_CONSTEXPRINLINE Tdst function_pointer_cast(Tsrc f) {
#if !defined(MPT_LIBCXX_QUIRK_INCOMPLETE_IS_FUNCTION)
	// MinGW64 std::is_function is always false for non __cdecl functions.
	// Issue is similar to <https://connect.microsoft.com/VisualStudio/feedback/details/774720/stl-is-function-bug>.
	static_assert(std::is_pointer<typename std::remove_cv<Tsrc>::type>::value);
	static_assert(std::is_pointer<typename std::remove_cv<Tdst>::type>::value);
	static_assert(std::is_function<typename std::remove_pointer<typename std::remove_cv<Tsrc>::type>::type>::value);
	static_assert(std::is_function<typename std::remove_pointer<typename std::remove_cv<Tdst>::type>::type>::value);
#endif
#if (MPT_CLANG_AT_LEAST(19, 0, 0) && !MPT_OS_ANDROID) || MPT_CLANG_AT_LEAST(20, 0, 0)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcast-function-type-mismatch"
#endif
	return reinterpret_cast<Tdst>(f);
#if (MPT_CLANG_AT_LEAST(19, 0, 0) && !MPT_OS_ANDROID) || MPT_CLANG_AT_LEAST(20, 0, 0)
#pragma clang diagnostic pop
#endif
}



#if MPT_CXX_AT_LEAST(20) && !MPT_LIBCXX_LLVM_BEFORE(13000)

using std::in_range;

#else

namespace detail {

template <typename Tdst, typename Tsrc>
constexpr Tdst saturate_cast(Tsrc src) noexcept {
	return mpt::saturate_cast<Tdst>(src);
}

template <typename Tdst>
constexpr Tdst saturate_cast(double src) {
	return mpt::saturate_trunc<Tdst>(src);
}

template <typename Tdst>
constexpr Tdst saturate_cast(float src) {
	return mpt::saturate_trunc<Tdst>(src);
}

} // namespace detail

// Returns true iff Tdst can represent the value val.
// Use as if(mpt::in_range<uint8>(-1)).
template <typename Tdst, typename Tsrc>
constexpr bool in_range(Tsrc val) {
	return (static_cast<Tsrc>(mpt::detail::saturate_cast<Tdst>(val)) == val);
}

#endif


#if MPT_CXX_AT_LEAST(23)

using std::to_underlying;

#else // !C++23

template <typename T>
constexpr std::underlying_type_t<T> to_underlying(T value) noexcept {
	return static_cast<typename std::underlying_type<T>::type>(value);
}

#endif // C++23



template <typename T>
struct value_initializer {
	inline void operator()(T & x) {
		x = T{};
	}
};

template <typename T, std::size_t N>
struct value_initializer<T[N]> {
	inline void operator()(T (&a)[N]) {
		for (auto & e : a) {
			value_initializer<T>{}(e);
		}
	}
};

template <typename T>
inline void reset(T & x) {
	value_initializer<T>{}(x);
}

template <typename T, typename... Targs>
void reset(T & x, Targs &&... args) {
	x = T{std::forward<Targs>(args)...};
}



template <typename T, typename... Targs>
void reconstruct(T & x, Targs &&... args) {
	x.~T();
	new (&x) T{std::forward<Targs>(args)...};
}



#if MPT_CXX_AT_LEAST(20) && !MPT_LIBCXX_LLVM_BEFORE(13000)

using std::cmp_equal;
using std::cmp_greater;
using std::cmp_greater_equal;
using std::cmp_less;
using std::cmp_less_equal;
using std::cmp_not_equal;

#else

template <typename Ta, typename Tb>
constexpr bool cmp_equal(Ta a, Tb b) noexcept {
	using UTa = typename std::make_unsigned<Ta>::type;
	using UTb = typename std::make_unsigned<Tb>::type;
	if constexpr (std::is_signed<Ta>::value == std::is_signed<Tb>::value) {
		return a == b;
	} else if constexpr (std::is_signed<Ta>::value) {
		return (a < 0) ? false : static_cast<UTa>(a) == b;
	} else {
		return (b < 0) ? false : a == static_cast<UTb>(b);
	}
}

template <typename Ta, typename Tb>
constexpr bool cmp_not_equal(Ta a, Tb b) noexcept {
	using UTa = typename std::make_unsigned<Ta>::type;
	using UTb = typename std::make_unsigned<Tb>::type;
	if constexpr (std::is_signed<Ta>::value == std::is_signed<Tb>::value) {
		return a != b;
	} else if constexpr (std::is_signed<Ta>::value) {
		return (a < 0) ? true : static_cast<UTa>(a) != b;
	} else {
		return (b < 0) ? true : a != static_cast<UTb>(b);
	}
}

template <typename Ta, typename Tb>
constexpr bool cmp_less(Ta a, Tb b) noexcept {
	using UTa = typename std::make_unsigned<Ta>::type;
	using UTb = typename std::make_unsigned<Tb>::type;
	if constexpr (std::is_signed<Ta>::value == std::is_signed<Tb>::value) {
		return a < b;
	} else if constexpr (std::is_signed<Ta>::value) {
		return (a < 0) ? true : static_cast<UTa>(a) < b;
	} else {
		return (b < 0) ? false : a < static_cast<UTb>(b);
	}
}

template <typename Ta, typename Tb>
constexpr bool cmp_greater(Ta a, Tb b) noexcept {
	using UTa = typename std::make_unsigned<Ta>::type;
	using UTb = typename std::make_unsigned<Tb>::type;
	if constexpr (std::is_signed<Ta>::value == std::is_signed<Tb>::value) {
		return a > b;
	} else if constexpr (std::is_signed<Ta>::value) {
		return (a < 0) ? false : static_cast<UTa>(a) > b;
	} else {
		return (b < 0) ? true : a > static_cast<UTb>(b);
	}
}

template <typename Ta, typename Tb>
constexpr bool cmp_less_equal(Ta a, Tb b) noexcept {
	using UTa = typename std::make_unsigned<Ta>::type;
	using UTb = typename std::make_unsigned<Tb>::type;
	if constexpr (std::is_signed<Ta>::value == std::is_signed<Tb>::value) {
		return a <= b;
	} else if constexpr (std::is_signed<Ta>::value) {
		return (a < 0) ? true : static_cast<UTa>(a) <= b;
	} else {
		return (b < 0) ? false : a <= static_cast<UTb>(b);
	}
}

template <typename Ta, typename Tb>
constexpr bool cmp_greater_equal(Ta a, Tb b) noexcept {
	using UTa = typename std::make_unsigned<Ta>::type;
	using UTb = typename std::make_unsigned<Tb>::type;
	if constexpr (std::is_signed<Ta>::value == std::is_signed<Tb>::value) {
		return a >= b;
	} else if constexpr (std::is_signed<Ta>::value) {
		return (a < 0) ? false : static_cast<UTa>(a) >= b;
	} else {
		return (b < 0) ? true : a >= static_cast<UTb>(b);
	}
}

#endif



#if MPT_CXX_AT_LEAST(23) && !MPT_LIBCXX_GNU_BEFORE(12)

using std::unreachable;

#else // !C++23

[[noreturn]] inline void unreachable() {
#if MPT_COMPILER_MSVC
	__assume(false);
#elif MPT_COMPILER_GCC || MPT_COMPILER_CLANG
	__builtin_unreachable();
#else
	std::terminate();
#endif
}

#endif // C++23



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_BASE_UTILITY_HPP
