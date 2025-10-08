/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_BASE_NUMBRES_HPP
#define MPT_BASE_NUMBRES_HPP



#include "mpt/base/detect_compiler.hpp"
#include "mpt/base/detect_quirks.hpp"
#include "mpt/base/namespace.hpp"

#if MPT_CXX_AT_LEAST(20) && !defined(MPT_LIBCXX_QUIRK_NO_NUMBERS)
#include <numbers>
#else
#include <type_traits>
#include <cmath>
#include <math.h>
#endif



namespace mpt {
inline namespace MPT_INLINE_NS {



namespace numbers {

#if MPT_CXX_AT_LEAST(20) && !defined(MPT_LIBCXX_QUIRK_NO_NUMBERS)

template <typename T>
inline constexpr T e_v = std::numbers::e_v<T>;
template <typename T>
inline constexpr T log2e_v = std::numbers::log2e_v<T>;
template <typename T>
inline constexpr T log10e_v = std::numbers::log10e_v<T>;
template <typename T>
inline constexpr T pi_v = std::numbers::pi_v<T>;
template <typename T>
inline constexpr T inv_pi_v = std::numbers::inv_pi_v<T>;
template <typename T>
inline constexpr T inv_sqrtpi_v = std::numbers::inv_sqrtpi_v<T>;
template <typename T>
inline constexpr T ln2_v = std::numbers::ln2_v<T>;
template <typename T>
inline constexpr T ln10_v = std::numbers::ln10_v<T>;
template <typename T>
inline constexpr T sqrt2_v = std::numbers::sqrt2_v<T>;
template <typename T>
inline constexpr T sqrt3_v = std::numbers::sqrt3_v<T>;
template <typename T>
inline constexpr T inv_sqrt3_v = std::numbers::inv_sqrt3_v<T>;
template <typename T>
inline constexpr T egamma_v = std::numbers::egamma_v<T>;
template <typename T>
inline constexpr T phi_v = std::numbers::phi_v<T>;

inline constexpr double e = e_v<double>;
inline constexpr double log2e = log2e_v<double>;
inline constexpr double log10e = log10e_v<double>;
inline constexpr double pi = pi_v<double>;
inline constexpr double inv_pi = inv_pi_v<double>;
inline constexpr double inv_sqrtpi = inv_sqrtpi_v<double>;
inline constexpr double ln2 = ln2_v<double>;
inline constexpr double ln10 = ln10_v<double>;
inline constexpr double sqrt2 = sqrt2_v<double>;
inline constexpr double sqrt3 = sqrt3_v<double>;
inline constexpr double inv_sqrt3 = inv_sqrt3_v<double>;
inline constexpr double egamma = egamma_v<double>;
inline constexpr double phi = phi_v<double>;

#else

#ifdef M_E
template <typename T, typename std::enable_if<std::is_floating_point<T>::value, bool>::type = true>
inline constexpr T e_v = static_cast<T>(M_E);
#else
template <typename T, typename std::enable_if<std::is_floating_point<T>::value, bool>::type = true>
inline constexpr T e_v = static_cast<T>(2.71828182845904523536);
#endif

#ifdef M_LOG2E
template <typename T, typename std::enable_if<std::is_floating_point<T>::value, bool>::type = true>
inline constexpr T log2e_v = static_cast<T>(M_LOG2E);
#else
template <typename T, typename std::enable_if<std::is_floating_point<T>::value, bool>::type = true>
inline constexpr T log2e_v = static_cast<T>(1.44269504088896340736);
#endif

#ifdef M_LOG10E
template <typename T, typename std::enable_if<std::is_floating_point<T>::value, bool>::type = true>
inline constexpr T log10e_v = static_cast<T>(M_LOG10E);
#else
template <typename T, typename std::enable_if<std::is_floating_point<T>::value, bool>::type = true>
inline constexpr T log10e_v = static_cast<T>(0.434294481903251827651);
#endif

#ifdef M_PI
template <typename T, typename std::enable_if<std::is_floating_point<T>::value, bool>::type = true>
inline constexpr T pi_v = static_cast<T>(M_PI);
#else
template <typename T, typename std::enable_if<std::is_floating_point<T>::value, bool>::type = true>
inline constexpr T pi_v = static_cast<T>(3.14159265358979323846);
#endif

#ifdef M_1_PI
template <typename T, typename std::enable_if<std::is_floating_point<T>::value, bool>::type = true>
inline constexpr T inv_pi_v = static_cast<T>(M_1_PI);
#else
template <typename T, typename std::enable_if<std::is_floating_point<T>::value, bool>::type = true>
inline constexpr T inv_pi_v = static_cast<T>(0.318309886183790671538);
#endif

template <typename T, typename std::enable_if<std::is_floating_point<T>::value, bool>::type = true>
inline constexpr T inv_sqrtpi_v = static_cast<T>(0.564189583547756286948079451560772586);

#ifdef M_LN2
template <typename T, typename std::enable_if<std::is_floating_point<T>::value, bool>::type = true>
inline constexpr T ln2_v = static_cast<T>(M_LN2);
#else
template <typename T, typename std::enable_if<std::is_floating_point<T>::value, bool>::type = true>
inline constexpr T ln2_v = static_cast<T>(0.693147180559945309417);
#endif

#ifdef M_LN10
template <typename T, typename std::enable_if<std::is_floating_point<T>::value, bool>::type = true>
inline constexpr T ln10_v = static_cast<T>(M_LN10);
#else
template <typename T, typename std::enable_if<std::is_floating_point<T>::value, bool>::type = true>
inline constexpr T ln10_v = static_cast<T>(2.30258509299404568402);
#endif

#ifdef M_SQRT2
template <typename T, typename std::enable_if<std::is_floating_point<T>::value, bool>::type = true>
inline constexpr T sqrt2_v = static_cast<T>(M_SQRT2);
#else
template <typename T, typename std::enable_if<std::is_floating_point<T>::value, bool>::type = true>
inline constexpr T sqrt2_v = static_cast<T>(1.41421356237309504880);
#endif

template <typename T, typename std::enable_if<std::is_floating_point<T>::value, bool>::type = true>
inline constexpr T sqrt3_v = static_cast<T>(1.732050807568877293527446341505872367);

template <typename T, typename std::enable_if<std::is_floating_point<T>::value, bool>::type = true>
inline constexpr T inv_sqrt3_v = static_cast<T>(0.577350269189625764509148780501957456);

template <typename T, typename std::enable_if<std::is_floating_point<T>::value, bool>::type = true>
inline constexpr T egamma_v = static_cast<T>(0.577215664901532860606512090082402431);

template <typename T, typename std::enable_if<std::is_floating_point<T>::value, bool>::type = true>
inline constexpr T phi_v = static_cast<T>(1.618033988749894848204586834365638118);

inline constexpr double e = e_v<double>;
inline constexpr double log2e = log2e_v<double>;
inline constexpr double log10e = log10e_v<double>;
inline constexpr double pi = pi_v<double>;
inline constexpr double inv_pi = inv_pi_v<double>;
inline constexpr double inv_sqrtpi = inv_sqrtpi_v<double>;
inline constexpr double ln2 = ln2_v<double>;
inline constexpr double ln10 = ln10_v<double>;
inline constexpr double sqrt2 = sqrt2_v<double>;
inline constexpr double sqrt3 = sqrt3_v<double>;
inline constexpr double inv_sqrt3 = inv_sqrt3_v<double>;
inline constexpr double egamma = egamma_v<double>;
inline constexpr double phi = phi_v<double>;

#endif

} // namespace numbers



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_BASE_NUMBRES_HPP
