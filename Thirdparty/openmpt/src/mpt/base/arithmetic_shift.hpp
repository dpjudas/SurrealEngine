/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_BASE_ARITHMETIC_SHIFT_HPP
#define MPT_BASE_ARITHMETIC_SHIFT_HPP



#include "mpt/base/detect.hpp"
#include "mpt/base/namespace.hpp"


namespace mpt {
inline namespace MPT_INLINE_NS {


// mpt::rshift_signed
// mpt::lshift_signed
// Shift a signed integer value in a well-defined manner.
// Does the same thing as MSVC would do. This is verified by the test suite.

template <typename T>
constexpr auto rshift_signed_portable(T x, int y) noexcept -> decltype(x >> y) {
	static_assert(std::numeric_limits<T>::is_integer);
	static_assert(std::numeric_limits<T>::is_signed);
	using result_type = decltype(x >> y);
	using unsigned_result_type = typename std::make_unsigned<result_type>::type;
	const unsigned_result_type roffset = static_cast<unsigned_result_type>(1) << ((sizeof(result_type) * 8) - 1);
	result_type rx = x;
	unsigned_result_type urx = static_cast<unsigned_result_type>(rx);
	urx += roffset;
	urx >>= y;
	urx -= roffset >> y;
	return static_cast<result_type>(urx);
}

template <typename T>
constexpr auto lshift_signed_portable(T x, int y) noexcept -> decltype(x << y) {
	static_assert(std::numeric_limits<T>::is_integer);
	static_assert(std::numeric_limits<T>::is_signed);
	using result_type = decltype(x << y);
	using unsigned_result_type = typename std::make_unsigned<result_type>::type;
	const unsigned_result_type roffset = static_cast<unsigned_result_type>(1) << ((sizeof(result_type) * 8) - 1);
	result_type rx = x;
	unsigned_result_type urx = static_cast<unsigned_result_type>(rx);
	urx += roffset;
	urx <<= y;
	urx -= roffset << y;
	return static_cast<result_type>(urx);
}

#if MPT_CXX_AT_LEAST(20) || MPT_COMPILER_SHIFT_SIGNED

template <typename T>
constexpr auto rshift_signed_cxx20(T x, int y) noexcept -> decltype(x >> y) {
	static_assert(std::numeric_limits<T>::is_integer);
	static_assert(std::numeric_limits<T>::is_signed);
#if MPT_COMPILER_GCC
// <https://gcc.gnu.org/bugzilla/show_bug.cgi?id=103826>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshift-negative-value"
#endif // MPT_COMPILER_GCC
	return x >> y;
#if MPT_COMPILER_GCC
#pragma GCC diagnostic pop
#endif // MPT_COMPILER_GCC
}

template <typename T>
constexpr auto lshift_signed_cxx20(T x, int y) noexcept -> decltype(x << y) {
	static_assert(std::numeric_limits<T>::is_integer);
	static_assert(std::numeric_limits<T>::is_signed);
#if MPT_COMPILER_GCC
// <https://gcc.gnu.org/bugzilla/show_bug.cgi?id=103826>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshift-negative-value"
#endif // MPT_COMPILER_GCC
	return x << y;
#if MPT_COMPILER_GCC
#pragma GCC diagnostic pop
#endif // MPT_COMPILER_GCC
}

template <typename T>
constexpr auto rshift_signed(T x, int y) noexcept -> decltype(x >> y) {
	return mpt::rshift_signed_cxx20(x, y);
}

template <typename T>
constexpr auto lshift_signed(T x, int y) noexcept -> decltype(x << y) {
	return mpt::lshift_signed_cxx20(x, y);
}

#else

template <typename T>
constexpr auto rshift_signed(T x, int y) noexcept -> decltype(x >> y) {
	return mpt::rshift_signed_portable(x, y);
}

template <typename T>
constexpr auto lshift_signed(T x, int y) noexcept -> decltype(x << y) {
	return mpt::lshift_signed_portable(x, y);
}

#endif

template <typename T>
constexpr auto arithmetic_shift_right(T x, int y) noexcept -> decltype(x >> y) {
	return mpt::rshift_signed(x, y);
}

template <typename T>
constexpr auto arithmetic_shift_right(T x, int y) noexcept -> decltype(x << y) {
	return mpt::lshift_signed(x, y);
}

template <typename T>
constexpr auto sar(T x, int y) noexcept -> decltype(x >> y) {
	return mpt::rshift_signed(x, y);
}

template <typename T>
constexpr auto sal(T x, int y) noexcept -> decltype(x << y) {
	return mpt::lshift_signed(x, y);
}


} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_BASE_ARITHMETIC_SHIFT_HPP
