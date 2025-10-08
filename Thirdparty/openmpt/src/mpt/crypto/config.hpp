/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_CRYPTO_CONFIG_HPP
#define MPT_CRYPTO_CONFIG_HPP



#include "mpt/base/detect.hpp"
#include "mpt/base/namespace.hpp"



#if MPT_OS_WINDOWS && MPT_OS_WINDOWS_WINNT
//#if MPT_WINNT_AT_LEAST(MPT_WIN_VISTA)
#define MPT_CRYPTO_WINDOWS
//#endif // >= Windows Vista
#endif // MPT_OS_WINDOWS

#if defined(MPT_WITH_CRYPTOPP)
#define MPT_CRYPTO_CRYPTOPP
#endif // MPT_WITH_CRYPTOPP



namespace mpt {
inline namespace MPT_INLINE_NS {


namespace crypto {



#if defined(MPT_CRYPTO_CRYPTOPP)
inline namespace cryptopp {
} // namespace cryptopp
#elif defined(MPT_CRYPTO_WINDOWS)
inline namespace windows {
} // namespace windows
#endif // MPT_CRYPTO



} // namespace crypto


} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_CRYPTO_CONFIG_HPP
