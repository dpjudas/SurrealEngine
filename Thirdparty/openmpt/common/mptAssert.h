/*
 * mptAssert.h
 * -----------
 * Purpose: assert and static_assert
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#pragma once

#include "openmpt/all/BuildSettings.hpp"


#include "mpt/base/source_location.hpp"
#include "mptBaseMacros.h"



OPENMPT_NAMESPACE_BEGIN



// Static code checkers might need to get the knowledge of our assertions transferred to them.
#define MPT_CHECKER_ASSUME_ASSERTIONS 1
//#define MPT_CHECKER_ASSUME_ASSERTIONS 0

#ifdef MPT_BUILD_ANALYZED

#if MPT_COMPILER_MSVC

#if MPT_CHECKER_ASSUME_ASSERTIONS
#define MPT_CHECKER_ASSUME(x) __analysis_assume(!!(x))
#endif

#elif MPT_COMPILER_CLANG

#if MPT_CHECKER_ASSUME_ASSERTIONS
#ifdef NDEBUG
#error "Builds for static analyzers depend on assert() being enabled, but the current build has #define NDEBUG. This makes no sense."
#endif
OPENMPT_NAMESPACE_END
#include <cassert>
OPENMPT_NAMESPACE_BEGIN
#define MPT_CHECKER_ASSUME(x) assert(!!(x))
#endif

#endif // MPT_COMPILER

#endif // MPT_BUILD_ANALYZED

#ifndef MPT_CHECKER_ASSUME
#define MPT_CHECKER_ASSUME(x) do { } while(0)
#endif



#if defined(MPT_WITH_MFC) && !defined(MPT_CPPCHECK_CUSTOM)

#if !defined(ASSERT)
#error "MFC is expected to #define ASSERT"
#endif // !defined(ASERRT)
#define MPT_FRAMEWORK_ASSERT_IS_DEFINED

#if defined(_DEBUG)
 #define MPT_FRAMEWORK_ASSERT_IS_ACTIVE 1
#else // !_DEBUG
 #define MPT_FRAMEWORK_ASSERT_IS_ACTIVE 0
#endif // _DEBUG

// let MFC handle our asserts
#define MPT_ASSERT_USE_FRAMEWORK 1

#else // !MPT_WITH_MFC

#if defined(ASSERT)
#define MPT_FRAMEWORK_ASSERT_IS_DEFINED
#if defined(_DEBUG)
 #define MPT_FRAMEWORK_ASSERT_IS_ACTIVE 1
#else // !_DEBUG
 #define MPT_FRAMEWORK_ASSERT_IS_ACTIVE 0
#endif // _DEBUG
#endif // !defined(ASERRT)

// handle assert in our own way without relying on some platform-/framework-specific assert implementation
#define MPT_ASSERT_USE_FRAMEWORK 0

#endif // MPT_WITH_MFC

#if defined(MPT_FRAMEWORK_ASSERT_IS_DEFINED) && (MPT_ASSERT_USE_FRAMEWORK == 1)

#define MPT_ASSERT_NOTREACHED()          ASSERT(0)
#define MPT_ASSERT(expr)                 ASSERT((expr))
#define MPT_ASSERT_MSG(expr, msg)        ASSERT((expr) && (msg))
#if (MPT_FRAMEWORK_ASSERT_IS_ACTIVE == 1)
#define MPT_ASSERT_ALWAYS(expr)          ASSERT((expr))
#define MPT_ASSERT_ALWAYS_MSG(expr, msg) ASSERT((expr) && (msg))
#else
#define MPT_ASSERT_ALWAYS(expr)          do { if(!(expr)) { OPENMPT_NAMESPACE::AssertHandler(MPT_SOURCE_LOCATION_CURRENT(), #expr); } MPT_CHECKER_ASSUME(expr); } while(0)
#define MPT_ASSERT_ALWAYS_MSG(expr, msg) do { if(!(expr)) { OPENMPT_NAMESPACE::AssertHandler(MPT_SOURCE_LOCATION_CURRENT(), #expr, msg); } MPT_CHECKER_ASSUME(expr); } while(0)
#ifndef MPT_ASSERT_HANDLER_NEEDED
#define MPT_ASSERT_HANDLER_NEEDED
#endif
#endif

#elif defined(NO_ASSERTS)

#define MPT_ASSERT_NOTREACHED()          MPT_CHECKER_ASSUME(0)
#define MPT_ASSERT(expr)                 MPT_CHECKER_ASSUME(expr)
#define MPT_ASSERT_MSG(expr, msg)        MPT_CHECKER_ASSUME(expr)
#define MPT_ASSERT_ALWAYS(expr)          do { if(!(expr)) { OPENMPT_NAMESPACE::AssertHandler(MPT_SOURCE_LOCATION_CURRENT(), #expr); } MPT_CHECKER_ASSUME(expr); } while(0)
#define MPT_ASSERT_ALWAYS_MSG(expr, msg) do { if(!(expr)) { OPENMPT_NAMESPACE::AssertHandler(MPT_SOURCE_LOCATION_CURRENT(), #expr, msg); } MPT_CHECKER_ASSUME(expr); } while(0)
#ifndef MPT_ASSERT_HANDLER_NEEDED
#define MPT_ASSERT_HANDLER_NEEDED
#endif

#else // !NO_ASSERTS

#define MPT_ASSERT_NOTREACHED()          do { if constexpr(!(0)) { OPENMPT_NAMESPACE::AssertHandler(MPT_SOURCE_LOCATION_CURRENT(), "0"); } MPT_CHECKER_ASSUME(0); } while(0)
#define MPT_ASSERT(expr)                 do { if(!(expr)) { OPENMPT_NAMESPACE::AssertHandler(MPT_SOURCE_LOCATION_CURRENT(), #expr); } MPT_CHECKER_ASSUME(expr); } while(0)
#define MPT_ASSERT_MSG(expr, msg)        do { if(!(expr)) { OPENMPT_NAMESPACE::AssertHandler(MPT_SOURCE_LOCATION_CURRENT(), #expr, msg); } MPT_CHECKER_ASSUME(expr); } while(0)
#define MPT_ASSERT_ALWAYS(expr)          do { if(!(expr)) { OPENMPT_NAMESPACE::AssertHandler(MPT_SOURCE_LOCATION_CURRENT(), #expr); } MPT_CHECKER_ASSUME(expr); } while(0)
#define MPT_ASSERT_ALWAYS_MSG(expr, msg) do { if(!(expr)) { OPENMPT_NAMESPACE::AssertHandler(MPT_SOURCE_LOCATION_CURRENT(), #expr, msg); } MPT_CHECKER_ASSUME(expr); } while(0)
#ifndef MPT_ASSERT_HANDLER_NEEDED
#define MPT_ASSERT_HANDLER_NEEDED
#endif

#endif // NO_ASSERTS


#if defined(MPT_ASSERT_HANDLER_NEEDED)
// custom assert handler needed
MPT_NOINLINE void AssertHandler(const mpt::source_location &loc, const char *expr, const char *msg=nullptr);
#endif // MPT_ASSERT_HANDLER_NEEDED



OPENMPT_NAMESPACE_END
