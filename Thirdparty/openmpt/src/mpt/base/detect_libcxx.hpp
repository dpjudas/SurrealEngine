/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_BASE_DETECT_LIBCXX_HPP
#define MPT_BASE_DETECT_LIBCXX_HPP



#include "mpt/base/detect_compiler.hpp"
#include "mpt/base/detect_os.hpp"

#if MPT_CXX_AT_LEAST(20)
#include <version>
#else // !C++20
#include <array>
#endif // C++20



// order of checks is important!
#if MPT_COMPILER_GENERIC
#define MPT_LIBCXX_GENERIC 1
#elif defined(_LIBCPP_VERSION)
#define MPT_LIBCXX_LLVM               1
#define MPT_LIBCXX_LLVM_VERSION       _LIBCPP_VERSION
#define MPT_LIBCXX_LLVM_AT_LEAST(ver) (MPT_LIBCXX_LLVM_VERSION >= (ver))
#define MPT_LIBCXX_LLVM_BEFORE(ver)   (MPT_LIBCXX_LLVM_VERSION < (ver))
#elif defined(__GLIBCXX__) || defined(__GLIBCPP__)
#define MPT_LIBCXX_GNU               1
#define MPT_LIBCXX_GNU_VERSION       _GLIBCXX_RELEASE
#define MPT_LIBCXX_GNU_AT_LEAST(ver) (MPT_LIBCXX_GNU_VERSION >= (ver))
#define MPT_LIBCXX_GNU_BEFORE(ver)   (MPT_LIBCXX_GNU_VERSION < (ver))
#elif MPT_COMPILER_MSVC
#define MPT_LIBCXX_MS 1
#elif MPT_COMPILER_CLANG && MPT_OS_WINDOWS
#define MPT_LIBCXX_MS 1
#else
#define MPT_LIBCXX_GENERIC 1
#endif

#ifndef MPT_LIBCXX_GENERIC
#define MPT_LIBCXX_GENERIC 0
#endif
#ifndef MPT_LIBCXX_LLVM
#define MPT_LIBCXX_LLVM               0
#define MPT_LIBCXX_LLVM_VERSION       0
#define MPT_LIBCXX_LLVM_AT_LEAST(ver) 0
#define MPT_LIBCXX_LLVM_BEFORE(ver)   0
#endif
#ifndef MPT_LIBCXX_GNU
#define MPT_LIBCXX_GNU               0
#define MPT_LIBCXX_GNU_VERSION       0
#define MPT_LIBCXX_GNU_AT_LEAST(ver) 0
#define MPT_LIBCXX_GNU_BEFORE(ver)   0
#endif
#ifndef MPT_LIBCXX_MS
#define MPT_LIBCXX_MS 0
#endif



#endif // MPT_BASE_DETECT_LIBCXX_HPP
