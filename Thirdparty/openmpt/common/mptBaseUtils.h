/*
 * mptBaseUtils.h
 * --------------
 * Purpose: Various useful utility functions.
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#pragma once

#include "openmpt/all/BuildSettings.hpp"


#include "mpt/base/algorithm.hpp"
#include "mpt/base/arithmetic_shift.hpp"
#include "mpt/base/array.hpp"
#include "mpt/base/bit.hpp"
#include "mpt/base/constexpr_throw.hpp"
#include "mpt/base/detect_arch.hpp"
#include "mpt/base/detect_compiler.hpp"
#include "mpt/base/math.hpp"
#include "mpt/base/memory.hpp"
#include "mpt/base/numeric.hpp"
#include "mpt/base/saturate_cast.hpp"
#include "mpt/base/saturate_round.hpp"
#include "mpt/base/utility.hpp"
#include "mpt/base/wrapping_divide.hpp"

#include "mptBaseMacros.h"
#include "mptBaseTypes.h"

#include <algorithm>
#include <limits>
#include <numeric>
#include <utility>

#include <cmath>
#include <cstdlib>

#include <math.h>
#include <stdlib.h>

#if MPT_COMPILER_MSVC
#include <intrin.h>
#endif



OPENMPT_NAMESPACE_BEGIN



template <typename T>
inline void Clear(T& x)
{
	static_assert(!std::is_pointer<T>::value);
	mpt::reset(x);
}


// Memset given object to zero.
template <class T>
inline void MemsetZero(T& a)
{
	static_assert(std::is_pointer<T>::value == false, "Won't memset pointers.");
	mpt::memclear(a);
}



// Limits 'val' to given range. If 'val' is less than 'lowerLimit', 'val' is set to value 'lowerLimit'.
// Similarly if 'val' is greater than 'upperLimit', 'val' is set to value 'upperLimit'.
// If 'lowerLimit' > 'upperLimit', 'val' won't be modified.
template<class T, class C>
inline void Limit(T& val, const C lowerLimit, const C upperLimit)
{
	if(lowerLimit > upperLimit) return;
	if(val < lowerLimit) val = lowerLimit;
	else if(val > upperLimit) val = upperLimit;
}


// Like Limit, but returns value
template<class T, class C>
inline T Clamp(T val, const C lowerLimit, const C upperLimit)
{
	if(val < lowerLimit) return lowerLimit;
	else if(val > upperLimit) return upperLimit;
	else return val;
}

// Like Limit, but with upperlimit only.
template<class T, class C>
inline void LimitMax(T& val, const C upperLimit)
{
	if(val > upperLimit)
		val = upperLimit;
}



namespace Util
{

	// Returns maximum value of given integer type.
	template <class T> constexpr T MaxValueOfType(const T&) {static_assert(std::numeric_limits<T>::is_integer == true, "Only integer types are allowed."); return (std::numeric_limits<T>::max)();}

}  // namespace Util



namespace Util {

	// Multiply two 32-bit integers, receive 64-bit result.
	// MSVC generates unnecessarily complicated code for the unoptimized variant using _allmul.
	MPT_CONSTEXPR20_FUN int64 mul32to64(int32 a, int32 b)
	{
		#if MPT_COMPILER_MSVC && (MPT_ARCH_X86 || MPT_ARCH_AMD64)
			MPT_MAYBE_CONSTANT_IF(MPT_IS_CONSTANT_EVALUATED20())
			{
				return static_cast<int64>(a) * b;
			} else
			{
				return __emul(a, b);
			}
		#else
			return static_cast<int64>(a) * b;
		#endif
	}

	MPT_CONSTEXPR20_FUN uint64 mul32to64_unsigned(uint32 a, uint32 b)
	{
		#if MPT_COMPILER_MSVC && (MPT_ARCH_X86 || MPT_ARCH_AMD64)
			MPT_MAYBE_CONSTANT_IF(MPT_IS_CONSTANT_EVALUATED20())
			{
				return static_cast<uint64>(a) * b;
			} else
			{
				return __emulu(a, b);
			}
		#else
			return static_cast<uint64>(a) * b;
		#endif
	}

	MPT_CONSTEXPR20_FUN int32 muldiv(int32 a, int32 b, int32 c)
	{
		return mpt::saturate_cast<int32>( mul32to64( a, b ) / c );
	}

	MPT_CONSTEXPR20_FUN int32 muldivr(int32 a, int32 b, int32 c)
	{
		return mpt::saturate_cast<int32>( ( mul32to64( a, b ) + ( c / 2 ) ) / c );
	}

	// Do not use overloading because catching unsigned version by accident results in slower X86 code.
	MPT_CONSTEXPR20_FUN uint32 muldiv_unsigned(uint32 a, uint32 b, uint32 c)
	{
		return mpt::saturate_cast<uint32>( mul32to64_unsigned( a, b ) / c );
	}
	MPT_CONSTEXPR20_FUN uint32 muldivr_unsigned(uint32 a, uint32 b, uint32 c)
	{
		return mpt::saturate_cast<uint32>( ( mul32to64_unsigned( a, b ) + ( c / 2u ) ) / c );
	}

	constexpr MPT_FORCEINLINE int32 muldivrfloor(int64 a, uint32 b, uint32 c)
	{
		a *= b;
		a += c / 2u;
		return (a >= 0) ? mpt::saturate_cast<int32>(a / c) : mpt::saturate_cast<int32>((a - (c - 1)) / c);
	}

} // namespace Util



OPENMPT_NAMESPACE_END
