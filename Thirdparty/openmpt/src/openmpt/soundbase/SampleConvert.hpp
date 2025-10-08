/* SPDX-License-Identifier: BSD-3-Clause */
/* SPDX-FileCopyrightText: OpenMPT Project Developers and Contributors */


#pragma once

#include "openmpt/all/BuildSettings.hpp"

#include "mpt/base/arithmetic_shift.hpp"
#include "mpt/base/macros.hpp"
#include "mpt/base/math.hpp"
#include "mpt/base/saturate_cast.hpp"
#include "mpt/base/saturate_round.hpp"
#include "openmpt/base/Int24.hpp"
#include "openmpt/base/Types.hpp"
#include "openmpt/soundbase/SampleConvert.hpp"

#include <algorithm>
#include <limits>
#include <type_traits>

#include <cmath>



OPENMPT_NAMESPACE_BEGIN



namespace SC
{  // SC = _S_ample_C_onversion



#if MPT_COMPILER_MSVC
template <typename Tfloat>
MPT_FORCEINLINE Tfloat fastround(Tfloat x)
{
	static_assert(std::is_floating_point<Tfloat>::value);
	return std::floor(x + static_cast<Tfloat>(0.5));
}
#else
template <typename Tfloat>
MPT_FORCEINLINE Tfloat fastround(Tfloat x)
{
	static_assert(std::is_floating_point<Tfloat>::value);
	return mpt::round(x);
}
#endif



// Shift input_t down by shift and saturate to output_t.
template <typename Tdst, typename Tsrc, int shift>
struct ConvertShift
{
	using input_t = Tsrc;
	using output_t = Tdst;
	MPT_FORCEINLINE output_t operator()(input_t val)
	{
		return mpt::saturate_cast<output_t>(mpt::rshift_signed(val, shift));
	}
};



// Shift input_t up by shift and saturate to output_t.
template <typename Tdst, typename Tsrc, int shift>
struct ConvertShiftUp
{
	using input_t = Tsrc;
	using output_t = Tdst;
	MPT_FORCEINLINE output_t operator()(input_t val)
	{
		return mpt::saturate_cast<output_t>(mpt::lshift_signed(val, shift));
	}
};




// Every sample conversion functor has to typedef its input_t and output_t.
// The input_t argument is taken by value because we only deal with per-single-sample conversions here.


// straight forward type conversions, clamping when converting from floating point.
template <typename Tdst, typename Tsrc>
struct Convert;

template <typename Tid>
struct Convert<Tid, Tid>
{
	using input_t = Tid;
	using output_t = Tid;
	MPT_FORCEINLINE output_t operator()(input_t val)
	{
		return val;
	}
};

template <>
struct Convert<uint8, int8>
{
	using input_t = int8;
	using output_t = uint8;
	MPT_FORCEINLINE output_t operator()(input_t val)
	{
		return static_cast<uint8>(val + 0x80);
	}
};

template <>
struct Convert<uint8, int16>
{
	using input_t = int16;
	using output_t = uint8;
	MPT_FORCEINLINE output_t operator()(input_t val)
	{
		return static_cast<uint8>(static_cast<int8>(mpt::rshift_signed(val, 8)) + 0x80);
	}
};

template <>
struct Convert<uint8, int24>
{
	using input_t = int24;
	using output_t = uint8;
	MPT_FORCEINLINE output_t operator()(input_t val)
	{
		return static_cast<uint8>(static_cast<int8>(mpt::rshift_signed(static_cast<int>(val), 16)) + 0x80);
	}
};

template <>
struct Convert<uint8, int32>
{
	using input_t = int32;
	using output_t = uint8;
	MPT_FORCEINLINE output_t operator()(input_t val)
	{
		return static_cast<uint8>(static_cast<int8>(mpt::rshift_signed(val, 24)) + 0x80);
	}
};

template <>
struct Convert<uint8, int64>
{
	using input_t = int64;
	using output_t = uint8;
	MPT_FORCEINLINE output_t operator()(input_t val)
	{
		return static_cast<uint8>(static_cast<int8>(mpt::rshift_signed(val, 56)) + 0x80);
	}
};

template <>
struct Convert<uint8, somefloat32>
{
	using input_t = somefloat32;
	using output_t = uint8;
	MPT_FORCEINLINE output_t operator()(input_t val)
	{
		val = mpt::safe_clamp(val, -1.0f, 1.0f);
		val *= 128.0f;
		return static_cast<uint8>(mpt::saturate_cast<int8>(static_cast<int>(SC::fastround(val))) + 0x80);
	}
};

template <>
struct Convert<uint8, double>
{
	using input_t = double;
	using output_t = uint8;
	MPT_FORCEINLINE output_t operator()(input_t val)
	{
		val = std::clamp(val, -1.0, 1.0);
		val *= 128.0;
		return static_cast<uint8>(mpt::saturate_cast<int8>(static_cast<int>(SC::fastround(val))) + 0x80);
	}
};

template <>
struct Convert<int8, uint8>
{
	using input_t = uint8;
	using output_t = int8;
	MPT_FORCEINLINE output_t operator()(input_t val)
	{
		return static_cast<int8>(static_cast<int>(val) - 0x80);
	}
};

template <>
struct Convert<int8, int16>
{
	using input_t = int16;
	using output_t = int8;
	MPT_FORCEINLINE output_t operator()(input_t val)
	{
		return static_cast<int8>(mpt::rshift_signed(val, 8));
	}
};

template <>
struct Convert<int8, int24>
{
	using input_t = int24;
	using output_t = int8;
	MPT_FORCEINLINE output_t operator()(input_t val)
	{
		return static_cast<int8>(mpt::rshift_signed(static_cast<int>(val), 16));
	}
};

template <>
struct Convert<int8, int32>
{
	using input_t = int32;
	using output_t = int8;
	MPT_FORCEINLINE output_t operator()(input_t val)
	{
		return static_cast<int8>(mpt::rshift_signed(val, 24));
	}
};

template <>
struct Convert<int8, int64>
{
	using input_t = int64;
	using output_t = int8;
	MPT_FORCEINLINE output_t operator()(input_t val)
	{
		return static_cast<int8>(mpt::rshift_signed(val, 56));
	}
};

template <>
struct Convert<int8, somefloat32>
{
	using input_t = somefloat32;
	using output_t = int8;
	MPT_FORCEINLINE output_t operator()(input_t val)
	{
		val = mpt::safe_clamp(val, -1.0f, 1.0f);
		val *= 128.0f;
		return mpt::saturate_cast<int8>(static_cast<int>(SC::fastround(val)));
	}
};

template <>
struct Convert<int8, double>
{
	using input_t = double;
	using output_t = int8;
	MPT_FORCEINLINE output_t operator()(input_t val)
	{
		val = std::clamp(val, -1.0, 1.0);
		val *= 128.0;
		return mpt::saturate_cast<int8>(static_cast<int>(SC::fastround(val)));
	}
};

template <>
struct Convert<int16, uint8>
{
	using input_t = uint8;
	using output_t = int16;
	MPT_FORCEINLINE output_t operator()(input_t val)
	{
		return static_cast<int16>(mpt::lshift_signed(static_cast<int>(val) - 0x80, 8));
	}
};

template <>
struct Convert<int16, int8>
{
	using input_t = int8;
	using output_t = int16;
	MPT_FORCEINLINE output_t operator()(input_t val)
	{
		return static_cast<int16>(mpt::lshift_signed(val, 8));
	}
};

template <>
struct Convert<int16, int24>
{
	using input_t = int24;
	using output_t = int16;
	MPT_FORCEINLINE output_t operator()(input_t val)
	{
		return static_cast<int16>(mpt::rshift_signed(static_cast<int>(val), 8));
	}
};

template <>
struct Convert<int16, int32>
{
	using input_t = int32;
	using output_t = int16;
	MPT_FORCEINLINE output_t operator()(input_t val)
	{
		return static_cast<int16>(mpt::rshift_signed(val, 16));
	}
};

template <>
struct Convert<int16, int64>
{
	using input_t = int64;
	using output_t = int16;
	MPT_FORCEINLINE output_t operator()(input_t val)
	{
		return static_cast<int16>(mpt::rshift_signed(val, 48));
	}
};

template <>
struct Convert<int16, somefloat32>
{
	using input_t = somefloat32;
	using output_t = int16;
	MPT_FORCEINLINE output_t operator()(input_t val)
	{
		val = mpt::safe_clamp(val, -1.0f, 1.0f);
		val *= 32768.0f;
		return mpt::saturate_cast<int16>(static_cast<int>(SC::fastround(val)));
	}
};

template <>
struct Convert<int16, double>
{
	using input_t = double;
	using output_t = int16;
	MPT_FORCEINLINE output_t operator()(input_t val)
	{
		val = std::clamp(val, -1.0, 1.0);
		val *= 32768.0;
		return mpt::saturate_cast<int16>(static_cast<int>(SC::fastround(val)));
	}
};

template <>
struct Convert<int24, uint8>
{
	using input_t = uint8;
	using output_t = int24;
	MPT_FORCEINLINE output_t operator()(input_t val)
	{
		return static_cast<int24>(mpt::lshift_signed(static_cast<int>(val) - 0x80, 16));
	}
};

template <>
struct Convert<int24, int8>
{
	using input_t = int8;
	using output_t = int24;
	MPT_FORCEINLINE output_t operator()(input_t val)
	{
		return static_cast<int24>(mpt::lshift_signed(val, 16));
	}
};

template <>
struct Convert<int24, int16>
{
	using input_t = int16;
	using output_t = int24;
	MPT_FORCEINLINE output_t operator()(input_t val)
	{
		return static_cast<int24>(mpt::lshift_signed(val, 8));
	}
};

template <>
struct Convert<int24, int32>
{
	using input_t = int32;
	using output_t = int24;
	MPT_FORCEINLINE output_t operator()(input_t val)
	{
		return static_cast<int24>(mpt::rshift_signed(val, 8));
	}
};

template <>
struct Convert<int24, int64>
{
	using input_t = int64;
	using output_t = int24;
	MPT_FORCEINLINE output_t operator()(input_t val)
	{
		return static_cast<int24>(mpt::rshift_signed(val, 40));
	}
};

template <>
struct Convert<int24, somefloat32>
{
	using input_t = somefloat32;
	using output_t = int24;
	MPT_FORCEINLINE output_t operator()(input_t val)
	{
		val = mpt::safe_clamp(val, -1.0f, 1.0f);
		val *= 2147483648.0f;
		return static_cast<int24>(mpt::rshift_signed(mpt::saturate_cast<int32>(static_cast<int64>(SC::fastround(val))), 8));
	}
};

template <>
struct Convert<int24, double>
{
	using input_t = double;
	using output_t = int24;
	MPT_FORCEINLINE output_t operator()(input_t val)
	{
		val = std::clamp(val, -1.0, 1.0);
		val *= 2147483648.0;
		return static_cast<int24>(mpt::rshift_signed(mpt::saturate_cast<int32>(static_cast<int64>(SC::fastround(val))), 8));
	}
};

template <>
struct Convert<int32, uint8>
{
	using input_t = uint8;
	using output_t = int32;
	MPT_FORCEINLINE output_t operator()(input_t val)
	{
		return static_cast<int32>(mpt::lshift_signed(static_cast<int>(val) - 0x80, 24));
	}
};

template <>
struct Convert<int32, int8>
{
	using input_t = int8;
	using output_t = int32;
	MPT_FORCEINLINE output_t operator()(input_t val)
	{
		return static_cast<int32>(mpt::lshift_signed(val, 24));
	}
};

template <>
struct Convert<int32, int16>
{
	using input_t = int16;
	using output_t = int32;
	MPT_FORCEINLINE output_t operator()(input_t val)
	{
		return static_cast<int32>(mpt::lshift_signed(val, 16));
	}
};

template <>
struct Convert<int32, int24>
{
	using input_t = int24;
	using output_t = int32;
	MPT_FORCEINLINE output_t operator()(input_t val)
	{
		return static_cast<int32>(mpt::lshift_signed(static_cast<int>(val), 8));
	}
};

template <>
struct Convert<int32, int64>
{
	using input_t = int64;
	using output_t = int32;
	MPT_FORCEINLINE output_t operator()(input_t val)
	{
		return static_cast<int32>(mpt::rshift_signed(val, 32));
	}
};

template <>
struct Convert<int32, somefloat32>
{
	using input_t = somefloat32;
	using output_t = int32;
	MPT_FORCEINLINE output_t operator()(input_t val)
	{
		val = mpt::safe_clamp(val, -1.0f, 1.0f);
		val *= 2147483648.0f;
		return mpt::saturate_cast<int32>(static_cast<int64>(SC::fastround(val)));
	}
};

template <>
struct Convert<int32, double>
{
	using input_t = double;
	using output_t = int32;
	MPT_FORCEINLINE output_t operator()(input_t val)
	{
		val = std::clamp(val, -1.0, 1.0);
		val *= 2147483648.0;
		return mpt::saturate_cast<int32>(static_cast<int64>(SC::fastround(val)));
	}
};

template <>
struct Convert<int64, uint8>
{
	using input_t = uint8;
	using output_t = int64;
	MPT_FORCEINLINE output_t operator()(input_t val)
	{
		return mpt::lshift_signed(static_cast<int64>(val) - 0x80, 56);
	}
};

template <>
struct Convert<int64, int8>
{
	using input_t = int8;
	using output_t = int64;
	MPT_FORCEINLINE output_t operator()(input_t val)
	{
		return mpt::lshift_signed(static_cast<int64>(val), 56);
	}
};

template <>
struct Convert<int64, int16>
{
	using input_t = int16;
	using output_t = int64;
	MPT_FORCEINLINE output_t operator()(input_t val)
	{
		return mpt::lshift_signed(static_cast<int64>(val), 48);
	}
};

template <>
struct Convert<int64, int24>
{
	using input_t = int24;
	using output_t = int64;
	MPT_FORCEINLINE output_t operator()(input_t val)
	{
		return mpt::lshift_signed(static_cast<int64>(val), 40);
	}
};

template <>
struct Convert<int64, int32>
{
	using input_t = int32;
	using output_t = int64;
	MPT_FORCEINLINE output_t operator()(input_t val)
	{
		return mpt::lshift_signed(static_cast<int64>(val), 32);
	}
};

template <>
struct Convert<int64, somefloat32>
{
	using input_t = somefloat32;
	using output_t = int64;
	MPT_FORCEINLINE output_t operator()(input_t val)
	{
		val = mpt::safe_clamp(val, -1.0f, 1.0f);
		val *= static_cast<float>(uint64(1) << 63);
		return mpt::saturate_trunc<int64>(SC::fastround(val));
	}
};

template <>
struct Convert<int64, double>
{
	using input_t = double;
	using output_t = int64;
	MPT_FORCEINLINE output_t operator()(input_t val)
	{
		val = std::clamp(val, -1.0, 1.0);
		val *= static_cast<double>(uint64(1) << 63);
		return mpt::saturate_trunc<int64>(SC::fastround(val));
	}
};

template <>
struct Convert<somefloat32, uint8>
{
	using input_t = uint8;
	using output_t = somefloat32;
	MPT_FORCEINLINE output_t operator()(input_t val)
	{
		return static_cast<somefloat32>(static_cast<int>(val) - 0x80) * (1.0f / static_cast<somefloat32>(static_cast<uint8>(1) << 7));
	}
};

template <>
struct Convert<somefloat32, int8>
{
	using input_t = int8;
	using output_t = somefloat32;
	MPT_FORCEINLINE output_t operator()(input_t val)
	{
		return static_cast<somefloat32>(val) * (1.0f / static_cast<float>(static_cast<uint8>(1) << 7));
	}
};

template <>
struct Convert<somefloat32, int16>
{
	using input_t = int16;
	using output_t = somefloat32;
	MPT_FORCEINLINE output_t operator()(input_t val)
	{
		return static_cast<somefloat32>(val) * (1.0f / static_cast<float>(static_cast<uint16>(1) << 15));
	}
};

template <>
struct Convert<somefloat32, int24>
{
	using input_t = int24;
	using output_t = somefloat32;
	MPT_FORCEINLINE output_t operator()(input_t val)
	{
		return static_cast<somefloat32>(val) * (1.0f / static_cast<float>(static_cast<uint32>(1) << 23));
	}
};

template <>
struct Convert<somefloat32, int32>
{
	using input_t = int32;
	using output_t = somefloat32;
	MPT_FORCEINLINE output_t operator()(input_t val)
	{
		return static_cast<somefloat32>(val) * (1.0f / static_cast<float>(static_cast<uint32>(1) << 31));
	}
};

template <>
struct Convert<somefloat32, int64>
{
	using input_t = int64;
	using output_t = somefloat32;
	MPT_FORCEINLINE output_t operator()(input_t val)
	{
		return static_cast<somefloat32>(val) * (1.0f / static_cast<float>(static_cast<uint64>(1) << 63));
	}
};

template <>
struct Convert<double, uint8>
{
	using input_t = uint8;
	using output_t = double;
	MPT_FORCEINLINE output_t operator()(input_t val)
	{
		return static_cast<double>(static_cast<int>(val) - 0x80) * (1.0 / static_cast<double>(static_cast<uint8>(1) << 7));
	}
};

template <>
struct Convert<double, int8>
{
	using input_t = int8;
	using output_t = double;
	MPT_FORCEINLINE output_t operator()(input_t val)
	{
		return static_cast<double>(val) * (1.0 / static_cast<double>(static_cast<uint8>(1) << 7));
	}
};

template <>
struct Convert<double, int16>
{
	using input_t = int16;
	using output_t = double;
	MPT_FORCEINLINE output_t operator()(input_t val)
	{
		return static_cast<double>(val) * (1.0 / static_cast<double>(static_cast<uint16>(1) << 15));
	}
};

template <>
struct Convert<double, int24>
{
	using input_t = int24;
	using output_t = double;
	MPT_FORCEINLINE output_t operator()(input_t val)
	{
		return static_cast<double>(val) * (1.0 / static_cast<double>(static_cast<uint32>(1) << 23));
	}
};

template <>
struct Convert<double, int32>
{
	using input_t = int32;
	using output_t = double;
	MPT_FORCEINLINE output_t operator()(input_t val)
	{
		return static_cast<double>(val) * (1.0 / static_cast<double>(static_cast<uint32>(1) << 31));
	}
};

template <>
struct Convert<double, int64>
{
	using input_t = int64;
	using output_t = double;
	MPT_FORCEINLINE output_t operator()(input_t val)
	{
		return static_cast<double>(val) * (1.0 / static_cast<double>(static_cast<uint64>(1) << 63));
	}
};

template <>
struct Convert<double, float>
{
	using input_t = float;
	using output_t = double;
	MPT_FORCEINLINE output_t operator()(input_t val)
	{
		return static_cast<double>(val);
	}
};

template <>
struct Convert<float, double>
{
	using input_t = double;
	using output_t = float;
	MPT_FORCEINLINE output_t operator()(input_t val)
	{
		return static_cast<float>(val);
	}
};



template <typename Tdst, typename Tsrc>
MPT_FORCEINLINE Tdst sample_cast(Tsrc src)
{
	return SC::Convert<Tdst, Tsrc>{}(src);
}



}  // namespace SC



OPENMPT_NAMESPACE_END
