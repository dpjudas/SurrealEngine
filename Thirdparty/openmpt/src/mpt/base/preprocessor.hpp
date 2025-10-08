/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_BASE_PREPROCESSOR_HPP
#define MPT_BASE_PREPROCESSOR_HPP



#define MPT_PP_DEFER(m, ...) m(__VA_ARGS__)

#define MPT_PP_STRINGIFY(x) #x

#define MPT_PP_JOIN_HELPER(a, b) a##b
#define MPT_PP_JOIN(a, b)        MPT_PP_JOIN_HELPER(a, b)

#define MPT_PP_UNIQUE_IDENTIFIER(prefix) MPT_PP_JOIN(prefix, __LINE__)



#endif // MPT_BASE_PREPROCESSOR_HPP
