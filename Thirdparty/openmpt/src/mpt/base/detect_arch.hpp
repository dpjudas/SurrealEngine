/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_BASE_DETECT_ARCH_HPP
#define MPT_BASE_DETECT_ARCH_HPP



#include "mpt/base/detect_compiler.hpp"



// The order of the checks matters!



#if MPT_COMPILER_GENERIC



#define MPT_ARCH_GENERIC 1



#elif MPT_COMPILER_MSVC



#if defined(_M_ARM64) || defined(_M_ARM64EC)
#define MPT_ARCH_AARCH64 1

#elif defined(_M_ARM)
#define MPT_ARCH_ARM 1

#elif defined(_M_AMD64) || defined(_M_X64)
#define MPT_ARCH_AMD64 1

#elif defined(_M_IX86)
#define MPT_ARCH_X86 1

#endif



#elif MPT_COMPILER_GCC || MPT_COMPILER_CLANG



#if defined(__wasm64__) || defined(__wasm64)
#define MPT_ARCH_WASM64 1

#elif defined(__wasm32__) || defined(__wasm32)
#define MPT_ARCH_WASM32 1

#elif defined(__aarch64__)
#define MPT_ARCH_AARCH64 1

#elif defined(__arm__)
#define MPT_ARCH_ARM 1

#elif defined(__amd64__) || defined(__x86_64__)
#define MPT_ARCH_AMD64 1

#elif defined(__i386__) || defined(_X86_)
#define MPT_ARCH_X86 1

#endif



#else // MPT_COMPILER



#if defined(__wasm64__) || defined(__wasm64)
#define MPT_ARCH_WASM64 1

#elif defined(__wasm32__) || defined(__wasm32)
#define MPT_ARCH_WASM32 1

#elif defined(__aarch64__) || defined(_M_ARM64)
#define MPT_ARCH_AARCH64 1

#elif defined(__arm__) || defined(_M_ARM)
#define MPT_ARCH_ARM 1

#elif defined(__amd64__) || defined(__x86_64__) || defined(_M_AMD64) || defined(_M_X64) || defined(__amd64) || defined(__x86_64)
#define MPT_ARCH_AMD64 1

#elif defined(__i386__) || defined(_X86_) || defined(_M_IX86) || defined(__i386) || defined(__X86__)
#define MPT_ARCH_X86 1

#endif



#endif // MPT_COMPILER



#ifndef MPT_ARCH_GENERIC
#define MPT_ARCH_GENERIC 0
#endif
#ifndef MPT_ARCH_WASM64
#define MPT_ARCH_WASM64 0
#endif
#ifndef MPT_ARCH_WASM32
#define MPT_ARCH_WASM32 0
#endif
#ifndef MPT_ARCH_AARCH64
#define MPT_ARCH_AARCH64 0
#endif
#ifndef MPT_ARCH_ARM
#define MPT_ARCH_ARM 0
#endif
#ifndef MPT_ARCH_AMD64
#define MPT_ARCH_AMD64 0
#endif
#ifndef MPT_ARCH_X86
#define MPT_ARCH_X86 0
#endif



#if !MPT_COMPILER_GENERIC

#if MPT_COMPILER_MSVC
#define MPT_ARCH_LITTLE_ENDIAN
#elif MPT_COMPILER_GCC || MPT_COMPILER_CLANG
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define MPT_ARCH_BIG_ENDIAN
#elif __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define MPT_ARCH_LITTLE_ENDIAN
#endif
#elif defined(__BYTE_ORDER__) && defined(__ORDER_BIG_ENDIAN__) && defined(__ORDER_LITTLE_ENDIAN__)
#if __ORDER_BIG_ENDIAN__ != __ORDER_LITTLE_ENDIAN__
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define MPT_ARCH_BIG_ENDIAN
#elif __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define MPT_ARCH_LITTLE_ENDIAN
#endif
#endif
#endif

// fallback:
#if !defined(MPT_ARCH_BIG_ENDIAN) && !defined(MPT_ARCH_LITTLE_ENDIAN)
#if (defined(_BIG_ENDIAN) && !defined(_LITTLE_ENDIAN)) \
	|| (defined(__BIG_ENDIAN__) && !defined(__LITTLE_ENDIAN__)) \
	|| (defined(_STLP_BIG_ENDIAN) && !defined(_STLP_LITTLE_ENDIAN))
#define MPT_ARCH_BIG_ENDIAN
#elif (defined(_LITTLE_ENDIAN) && !defined(_BIG_ENDIAN)) \
	|| (defined(__LITTLE_ENDIAN__) && !defined(__BIG_ENDIAN__)) \
	|| (defined(_STLP_LITTLE_ENDIAN) && !defined(_STLP_BIG_ENDIAN))
#define MPT_ARCH_LITTLE_ENDIAN
#elif defined(__hpux) || defined(__hppa) \
	|| defined(_MIPSEB) \
	|| defined(__s390__)
#define MPT_ARCH_BIG_ENDIAN
#elif defined(__i386__) || defined(_M_IX86) \
	|| defined(__amd64) || defined(__amd64__) || defined(_M_AMD64) || defined(__x86_64) || defined(__x86_64__) || defined(_M_X64) \
	|| defined(__bfin__)
#define MPT_ARCH_LITTLE_ENDIAN
#endif
#endif

#endif // !MPT_COMPILER_GENERIC



// compiler assumed instruction set support

// clang-format off

#if MPT_ARCH_X86 || MPT_ARCH_AMD64

#if MPT_COMPILER_MSVC

#if defined(_M_X64)
	#define MPT_ARCH_X86_I386
	#define MPT_ARCH_X86_FPU
	#define MPT_ARCH_X86_FSIN
	#define MPT_ARCH_X86_I486
	#define MPT_ARCH_X86_CPUID
	#define MPT_ARCH_X86_TSC
	#define MPT_ARCH_X86_CX8
	#define MPT_ARCH_X86_CMOV
	#define MPT_ARCH_X86_MMX
	#define MPT_ARCH_X86_MMXEXT
	#define MPT_ARCH_X86_FXSR
	#define MPT_ARCH_X86_SSE
	#define MPT_ARCH_X86_SSE2
#elif defined(_M_IX86) && defined(_M_IX86_FP)
	#if (_M_IX86_FP >= 2)
		#define MPT_ARCH_X86_I386
		#define MPT_ARCH_X86_FPU
		#define MPT_ARCH_X86_FSIN
		#define MPT_ARCH_X86_I486
		#define MPT_ARCH_X86_CPUID
		#define MPT_ARCH_X86_TSC
		#define MPT_ARCH_X86_CX8
		#define MPT_ARCH_X86_CMOV
		#define MPT_ARCH_X86_MMX
		#define MPT_ARCH_X86_MMXEXT
		#define MPT_ARCH_X86_FXSR
		#define MPT_ARCH_X86_SSE
		#define MPT_ARCH_X86_SSE2
	#elif (_M_IX86_FP == 1)
		#define MPT_ARCH_X86_I386
		#define MPT_ARCH_X86_FPU
		#define MPT_ARCH_X86_FSIN
		#define MPT_ARCH_X86_I486
		#define MPT_ARCH_X86_CPUID
		#define MPT_ARCH_X86_TSC
		#define MPT_ARCH_X86_CX8
		#define MPT_ARCH_X86_CMOV
		#define MPT_ARCH_X86_MMX
		#define MPT_ARCH_X86_MMXEXT
		#define MPT_ARCH_X86_FXSR
		#define MPT_ARCH_X86_SSE
	#elif MPT_MSVC_AT_LEAST(2008, 0)
		#define MPT_ARCH_X86_I386
		#define MPT_ARCH_X86_FPU
		#define MPT_ARCH_X86_FSIN
		#define MPT_ARCH_X86_I486
		#define MPT_ARCH_X86_CPUID
		#define MPT_ARCH_X86_TSC
		#define MPT_ARCH_X86_CX8
	#elif MPT_MSVC_AT_LEAST(2005, 0)
		#define MPT_ARCH_X86_I386
		#define MPT_ARCH_X86_FPU
		#define MPT_ARCH_X86_FSIN
		#define MPT_ARCH_X86_I486
	#elif MPT_MSVC_AT_LEAST(1998, 0)
		#define MPT_ARCH_X86_I386
		#define MPT_ARCH_X86_FPU
		#define MPT_ARCH_X86_FSIN
	#else
		#define MPT_ARCH_X86_I386
	#endif
#endif
#if defined(__AVX__)
	#define MPT_ARCH_X86_3DNOWPREFETCH
	#ifndef MPT_ARCH_X86_XSAVE
	#define MPT_ARCH_X86_XSAVE
	#endif
	#define MPT_ARCH_X86_AVX
#endif
#if defined(__AVX2__)
	#ifndef MPT_ARCH_X86_XSAVE
	#define MPT_ARCH_X86_XSAVE
	#endif
	#define MPT_ARCH_X86_AVX2
	#define MPT_ARCH_X86_FMA
	#define MPT_ARCH_X86_BMI1
#endif

#elif MPT_COMPILER_GCC || MPT_COMPILER_CLANG

#define MPT_ARCH_X86_I386
#if !defined(_SOFT_FLOAT)
	#define MPT_ARCH_X86_FPU
	// GCC does not provide a macro for FSIN. Deduce it from 486 later.
#endif
#if defined(__i486__)
	// GCC does not consistently provide i486, deduce it later from cpuid.
	#define MPT_ARCH_X86_I486
#endif
#ifdef __GCC_HAVE_SYNC_COMPARE_AND_SWAP_8
	// GCC does not provide TSC or CPUID.
	// Imply it by CX8.
	#define MPT_ARCH_X86_CX8
	#define MPT_ARCH_X86_TSC
	#define MPT_ARCH_X86_CPUID
#endif
#if defined(__i686__) || defined(__athlon__)
	// GCC is broken here and does not set __i686__ for various non-Intel and even modern Intel CPUs
	// Imply __i686__ by __SSE__ as a work-around.
	#define MPT_ARCH_X86_CMOV
#endif
#if defined(MPT_ARCH_X86_CPUID)
	#ifndef MPT_ARCH_X86_I486
	#define MPT_ARCH_X86_I486
	#endif
#endif
#if defined(MPT_ARCH_X86_I486) && defined(MPT_ARCH_X86_FPU)
	#define MPT_ARCH_X86_FSIN
#endif
#ifdef __MMX__
	#define MPT_ARCH_X86_MMX
#endif
#ifdef __3dNOW__
	#define MPT_ARCH_X86_3DNOW
#endif
#ifdef __3dNOW_A__
	#define MPT_ARCH_X86_MMXEXT
	#define MPT_ARCH_X86_3DNOWEXT
#endif
#ifdef __PRFCHW__
	#define MPT_ARCH_X86_3DNOWPREFETCH
#endif
#ifdef __FXSR__
	#define MPT_ARCH_X86_FXSR
#endif
#ifdef __SSE__
	#ifndef MPT_ARCH_X86_MMXEXT
	#define MPT_ARCH_X86_MMXEXT
	#endif
	#define MPT_ARCH_X86_SSE
	#ifndef MPT_ARCH_X86_CMOV
	#define MPT_ARCH_X86_CMOV
	#endif
#endif
#ifdef __SSE2__
	#define MPT_ARCH_X86_SSE2
#endif
#ifdef __SSE3__
	#define MPT_ARCH_X86_SSE3
#endif
#ifdef __SSSE3__
	#define MPT_ARCH_X86_SSSE3
#endif
#ifdef __SSE4_1__
	#define MPT_ARCH_X86_SSE4_1
#endif
#ifdef __SSE4_2__
	#define MPT_ARCH_X86_SSE4_2
#endif
#ifdef __XSAVE__
	#define MPT_ARCH_X86_XSAVE
#endif
#ifdef __AVX__
	#define MPT_ARCH_X86_AVX
#endif
#ifdef __AVX2__
	#define MPT_ARCH_X86_AVX2
#endif
#ifdef __GCC_HAVE_SYNC_COMPARE_AND_SWAP_16
	#define MPT_ARCH_X86_CX16
#endif
#ifdef __LAHF_SAHF__
	#define MPT_ARCH_X86_LAHF
#endif
#ifdef __POPCNT__
	#define MPT_ARCH_X86_POPCNT
#endif
#ifdef __BMI__
	#define MPT_ARCH_X86_BMI1
#endif
#ifdef __BMI2__
	#define MPT_ARCH_X86_BMI2
#endif
#ifdef __F16C__
	#define MPT_ARCH_X86_F16C
#endif
#ifdef __FMA__
	#define MPT_ARCH_X86_FMA
#endif
#ifdef __LZCNT__
	#define MPT_ARCH_X86_LZCNT
#endif
#ifdef __MOVBE__
	#define MPT_ARCH_X86_MOVBE
#endif

#endif // MPT_COMPILER

#endif // MPT_ARCH

// clang-format on



// compiler supported instrinsics

// clang-format off

#if MPT_ARCH_X86 || MPT_ARCH_AMD64

#if MPT_COMPILER_MSVC

#define MPT_ARCH_INTRINSICS_X86_I386
#define MPT_ARCH_INTRINSICS_X86_FPU
#define MPT_ARCH_INTRINSICS_X86_FSIN
#define MPT_ARCH_INTRINSICS_X86_CPUID
#define MPT_ARCH_INTRINSICS_X86_TSC
#define MPT_ARCH_INTRINSICS_X86_CX8
#define MPT_ARCH_INTRINSICS_X86_CMOV
#define MPT_ARCH_INTRINSICS_X86_MMX
#define MPT_ARCH_INTRINSICS_X86_MMXEXT
#define MPT_ARCH_INTRINSICS_X86_3DNOW
#define MPT_ARCH_INTRINSICS_X86_3DNOWEXT
#define MPT_ARCH_INTRINSICS_X86_3DNOWPREFETCH
#if MPT_MSVC_AT_LEAST(2003, 0)
#define MPT_ARCH_INTRINSICS_X86_FXSR
#define MPT_ARCH_INTRINSICS_X86_SSE
#define MPT_ARCH_INTRINSICS_X86_SSE2
#endif
#if MPT_MSVC_AT_LEAST(2008, 0)
#define MPT_ARCH_INTRINSICS_X86_SSE3
#define MPT_ARCH_INTRINSICS_X86_SSSE3
#define MPT_ARCH_INTRINSICS_X86_SSE4_1
#define MPT_ARCH_INTRINSICS_X86_SSE4_2
#endif
#if MPT_MSVC_AT_LEAST(2010, 1)
#define MPT_ARCH_INTRINSICS_X86_XSAVE
#define MPT_ARCH_INTRINSICS_X86_AVX
#endif
#if MPT_MSVC_AT_LEAST(2012, 0)
#define MPT_ARCH_INTRINSICS_X86_AVX2
#define MPT_ARCH_INTRINSICS_X86_FMA
#define MPT_ARCH_INTRINSICS_X86_BMI1
#endif

#elif MPT_COMPILER_GCC || MPT_COMPILER_CLANG

#ifdef MPT_ARCH_X86_I386
#define MPT_ARCH_INTRINSICS_X86_I386
#endif
#ifdef MPT_ARCH_X86_FPU
#define MPT_ARCH_INTRINSICS_X86_FPU
#endif
#ifdef MPT_ARCH_X86_FSIN
#define MPT_ARCH_INTRINSICS_X86_FSIN
#endif
#ifdef MPT_ARCH_X86_I486
#define MPT_ARCH_INTRINSICS_X86_I486
#endif
#ifdef MPT_ARCH_X86_CPUID
#define MPT_ARCH_INTRINSICS_X86_CPUID
#endif
#ifdef MPT_ARCH_X86_TSC
#define MPT_ARCH_INTRINSICS_X86_TSC
#endif
#ifdef MPT_ARCH_X86_CX8
#define MPT_ARCH_INTRINSICS_X86_CX8
#endif
#ifdef MPT_ARCH_X86_CMOV
#define MPT_ARCH_INTRINSICS_X86_CMOV
#endif
#ifdef MPT_ARCH_X86_MMX
#define MPT_ARCH_INTRINSICS_X86_MMX
#endif
#ifdef MPT_ARCH_X86_MMXEXT
#define MPT_ARCH_INTRINSICS_X86_MMXEXT
#endif
#ifdef MPT_ARCH_X86_3DNOW
#define MPT_ARCH_INTRINSICS_X86_3DNOW
#endif
#ifdef MPT_ARCH_X86_3DNOWEXT
#define MPT_ARCH_INTRINSICS_X86_3DNOWEXT
#endif
#ifdef MPT_ARCH_X86_3DNOWPREFETCH
#define MPT_ARCH_INTRINSICS_X86_3DNOWPREFETCH
#endif
#ifdef MPT_ARCH_X86_FXSR
#define MPT_ARCH_INTRINSICS_X86_FXSR
#endif
#ifdef MPT_ARCH_X86_SSE
#define MPT_ARCH_INTRINSICS_X86_SSE
#endif
#ifdef MPT_ARCH_X86_SSE2
#define MPT_ARCH_INTRINSICS_X86_SSE2
#endif
#ifdef MPT_ARCH_X86_SSE3
#define MPT_ARCH_INTRINSICS_X86_SSE3
#endif
#ifdef MPT_ARCH_X86_SSSE3
#define MPT_ARCH_INTRINSICS_X86_SSSE3
#endif
#ifdef MPT_ARCH_X86_SSE4_1
#define MPT_ARCH_INTRINSICS_X86_SSE4_1
#endif
#ifdef MPT_ARCH_X86_SSE4_2
#define MPT_ARCH_INTRINSICS_X86_SSE4_2
#endif
#ifdef MPT_ARCH_X86_XSAVE
#define MPT_ARCH_INTRINSICS_X86_XSAVE
#endif
#ifdef MPT_ARCH_X86_AVX
#define MPT_ARCH_INTRINSICS_X86_AVX
#endif
#ifdef MPT_ARCH_X86_AVX2
#define MPT_ARCH_INTRINSICS_X86_AVX2
#endif
#ifdef MPT_ARCH_X86_CX16
#define MPT_ARCH_INTRINSICS_X86_CX16
#endif
#ifdef MPT_ARCH_X86_LAHF
#define MPT_ARCH_INTRINSICS_X86_LAHF
#endif
#ifdef MPT_ARCH_X86_POPCNT
#define MPT_ARCH_INTRINSICS_X86_POPCNT
#endif
#ifdef MPT_ARCH_X86_BMI1
#define MPT_ARCH_INTRINSICS_X86_BMI1
#endif
#ifdef MPT_ARCH_X86_BMI2
#define MPT_ARCH_INTRINSICS_X86_BMI2
#endif
#ifdef MPT_ARCH_X86_F16C
#define MPT_ARCH_INTRINSICS_X86_F16C
#endif
#ifdef MPT_ARCH_X86_FMA
#define MPT_ARCH_INTRINSICS_X86_FMA
#endif
#ifdef MPT_ARCH_X86_LZCNT
#define MPT_ARCH_INTRINSICS_X86_LZCNT
#endif
#ifdef MPT_ARCH_X86_MOVBE
#define MPT_ARCH_INTRINSICS_X86_MOVBE
#endif

#endif // MPT_COMPILER

#endif // MPT_ARCH

// clang-format on



#endif // MPT_BASE_DETECT_ARCH_HPP
