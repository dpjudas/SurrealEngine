/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_BASE_MATH_HPP
#define MPT_BASE_MATH_HPP



#include "mpt/base/detect.hpp"
#include "mpt/base/namespace.hpp"

#include <algorithm>
#include <type_traits>

#include <cmath>



namespace mpt {
inline namespace MPT_INLINE_NS {


#if MPT_OS_DJGPP

inline long double log2(const long double val) {
	return static_cast<long double>(::log2(static_cast<double>(val)));
}

inline double log2(const double val) {
	return ::log2(val);
}

inline float log2(const float val) {
	return ::log2f(val);
}

#else // !MPT_OS_DJGPP

// C++11 std::log2
using std::log2;

#endif // MPT_OS_DJGPP


#if MPT_OS_DJGPP

inline long double round(const long double val) {
	return ::roundl(val);
}

inline double round(const double val) {
	return ::round(val);
}

inline float round(const float val) {
	return ::roundf(val);
}

#else // !MPT_OS_DJGPP

// C++11 std::round
using std::round;

#endif // MPT_OS_DJGPP


#if MPT_OS_DJGPP

inline long double trunc(const long double val) {
	return ::truncl(val);
}

inline double trunc(const double val) {
	return ::trunc(val);
}

inline float trunc(const float val) {
	return ::truncf(val);
}

#else // !MPT_OS_DJGPP

// C++11 std::trunc
using std::trunc;

#endif // MPT_OS_DJGPP


template <typename T>
inline T sanitize_nan(T val) {
	static_assert(std::is_floating_point<T>::value);
	if (std::isnan(val)) {
		return T(0.0);
	}
	return val;
}


template <typename T>
inline T safe_clamp(T v, T lo, T hi) {
	static_assert(std::is_floating_point<T>::value);
	return std::clamp(mpt::sanitize_nan(v), lo, hi);
}


} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_BASE_MATH_HPP
