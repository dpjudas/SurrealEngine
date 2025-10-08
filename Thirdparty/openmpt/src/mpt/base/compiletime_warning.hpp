/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_BASE_COMPILETIME_WARNING_HPP
#define MPT_BASE_COMPILETIME_WARNING_HPP



#include "mpt/base/detect.hpp"
#include "mpt/base/preprocessor.hpp"



#if MPT_COMPILER_MSVC

#define MPT_WARNING(text)           __pragma(message(__FILE__ "(" MPT_PP_DEFER(MPT_PP_STRINGIFY, __LINE__) "): Warning: " text))
#define MPT_WARNING_STATEMENT(text) __pragma(message(__FILE__ "(" MPT_PP_DEFER(MPT_PP_STRINGIFY, __LINE__) "): Warning: " text))

#elif MPT_COMPILER_GCC || MPT_COMPILER_CLANG

#define MPT_WARNING(text)           _Pragma(MPT_PP_STRINGIFY(GCC warning text))
#define MPT_WARNING_STATEMENT(text) _Pragma(MPT_PP_STRINGIFY(GCC warning text))

#else

// portable #pragma message or #warning replacement
#define MPT_WARNING(text) \
	static inline int MPT_PP_UNIQUE_IDENTIFIER(MPT_WARNING_NAME)() noexcept { \
		int warning [[deprecated("Warning: " text)]] = 0; \
		return warning; \
	} \
/**/
#define MPT_WARNING_STATEMENT(text) \
	int MPT_PP_UNIQUE_IDENTIFIER(MPT_WARNING_NAME) = []() { \
		int warning [[deprecated("Warning: " text)]] = 0; \
		return warning; \
	}() /**/

#endif



#endif // MPT_BASE_COMPILETIME_WARNING_HPP
