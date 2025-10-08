/* SPDX-License-Identifier: BSD-3-Clause */
/* SPDX-FileCopyrightText: OpenMPT Project Developers and Contributors */


#pragma once

#include "openmpt/all/BuildSettings.hpp"

#include "mpt/base/arithmetic_shift.hpp"
#include "mpt/base/macros.hpp"
#include "mpt/base/math.hpp"
#include "mpt/base/saturate_cast.hpp"
#include "openmpt/base/Int24.hpp"
#include "openmpt/base/Types.hpp"
#include "openmpt/soundbase/SampleConvert.hpp"

#include <algorithm>
#include <limits>



OPENMPT_NAMESPACE_BEGIN


namespace SC
{  // SC = _S_ample_C_onversion


template <typename Tdst, typename Tsrc, int fractionalBits>
struct ConvertFixedPoint;

template <int fractionalBits>
struct ConvertFixedPoint<uint8, int32, fractionalBits>
{
	using input_t = int32;
	using output_t = uint8;
	static constexpr int shiftBits = fractionalBits + 1 - sizeof(output_t) * 8;
	MPT_FORCEINLINE output_t operator()(input_t val)
	{
		static_assert(fractionalBits >= 0 && fractionalBits <= sizeof(input_t) * 8 - 1);
		static_assert(shiftBits >= 1);
		val = mpt::rshift_signed((val + (1 << (shiftBits - 1))), shiftBits);  // round
		if(val < std::numeric_limits<int8>::min()) val = std::numeric_limits<int8>::min();
		if(val > std::numeric_limits<int8>::max()) val = std::numeric_limits<int8>::max();
		return static_cast<uint8>(val + 0x80);  // unsigned
	}
};

template <int fractionalBits>
struct ConvertFixedPoint<int8, int32, fractionalBits>
{
	using input_t = int32;
	using output_t = int8;
	static constexpr int shiftBits = fractionalBits + 1 - sizeof(output_t) * 8;
	MPT_FORCEINLINE output_t operator()(input_t val)
	{
		static_assert(fractionalBits >= 0 && fractionalBits <= sizeof(input_t) * 8 - 1);
		static_assert(shiftBits >= 1);
		val = mpt::rshift_signed((val + (1 << (shiftBits - 1))), shiftBits);  // round
		if(val < std::numeric_limits<int8>::min()) val = std::numeric_limits<int8>::min();
		if(val > std::numeric_limits<int8>::max()) val = std::numeric_limits<int8>::max();
		return static_cast<int8>(val);
	}
};

template <int fractionalBits>
struct ConvertFixedPoint<int16, int32, fractionalBits>
{
	using input_t = int32;
	using output_t = int16;
	static constexpr int shiftBits = fractionalBits + 1 - sizeof(output_t) * 8;
	MPT_FORCEINLINE output_t operator()(input_t val)
	{
		static_assert(fractionalBits >= 0 && fractionalBits <= sizeof(input_t) * 8 - 1);
		static_assert(shiftBits >= 1);
		val = mpt::rshift_signed((val + (1 << (shiftBits - 1))), shiftBits);  // round
		if(val < std::numeric_limits<int16>::min()) val = std::numeric_limits<int16>::min();
		if(val > std::numeric_limits<int16>::max()) val = std::numeric_limits<int16>::max();
		return static_cast<int16>(val);
	}
};

template <int fractionalBits>
struct ConvertFixedPoint<int24, int32, fractionalBits>
{
	using input_t = int32;
	using output_t = int24;
	static constexpr int shiftBits = fractionalBits + 1 - sizeof(output_t) * 8;
	MPT_FORCEINLINE output_t operator()(input_t val)
	{
		static_assert(fractionalBits >= 0 && fractionalBits <= sizeof(input_t) * 8 - 1);
		static_assert(shiftBits >= 1);
		val = mpt::rshift_signed((val + (1 << (shiftBits - 1))), shiftBits);  // round
		if(val < std::numeric_limits<int24>::min()) val = std::numeric_limits<int24>::min();
		if(val > std::numeric_limits<int24>::max()) val = std::numeric_limits<int24>::max();
		return static_cast<int24>(val);
	}
};

template <int fractionalBits>
struct ConvertFixedPoint<int32, int32, fractionalBits>
{
	using input_t = int32;
	using output_t = int32;
	MPT_FORCEINLINE output_t operator()(input_t val)
	{
		static_assert(fractionalBits >= 0 && fractionalBits <= sizeof(input_t) * 8 - 1);
		return static_cast<int32>(std::clamp(val, static_cast<int32>(-((1 << fractionalBits) - 1)), static_cast<int32>(1 << fractionalBits) - 1)) << (sizeof(input_t) * 8 - 1 - fractionalBits);
	}
};

template <int fractionalBits>
struct ConvertFixedPoint<somefloat32, int32, fractionalBits>
{
	using input_t = int32;
	using output_t = somefloat32;
	const float factor;
	MPT_FORCEINLINE ConvertFixedPoint()
		: factor(1.0f / static_cast<float>(1 << fractionalBits))
	{
		return;
	}
	MPT_FORCEINLINE output_t operator()(input_t val)
	{
		static_assert(fractionalBits >= 0 && fractionalBits <= sizeof(input_t) * 8 - 1);
		return static_cast<somefloat32>(val) * factor;
	}
};

template <int fractionalBits>
struct ConvertFixedPoint<somefloat64, int32, fractionalBits>
{
	using input_t = int32;
	using output_t = somefloat64;
	const double factor;
	MPT_FORCEINLINE ConvertFixedPoint()
		: factor(1.0 / static_cast<double>(1 << fractionalBits))
	{
		return;
	}
	MPT_FORCEINLINE output_t operator()(input_t val)
	{
		static_assert(fractionalBits >= 0 && fractionalBits <= sizeof(input_t) * 8 - 1);
		return static_cast<somefloat64>(val) * factor;
	}
};


template <typename Tdst, typename Tsrc, int fractionalBits>
struct ConvertToFixedPoint;

template <int fractionalBits>
struct ConvertToFixedPoint<int32, uint8, fractionalBits>
{
	using input_t = uint8;
	using output_t = int32;
	static constexpr int shiftBits = fractionalBits + 1 - sizeof(input_t) * 8;
	MPT_FORCEINLINE output_t operator()(input_t val)
	{
		static_assert(fractionalBits >= 0 && fractionalBits <= sizeof(output_t) * 8 - 1);
		static_assert(shiftBits >= 1);
		return mpt::lshift_signed(static_cast<output_t>(static_cast<int>(val) - 0x80), shiftBits);
	}
};

template <int fractionalBits>
struct ConvertToFixedPoint<int32, int8, fractionalBits>
{
	using input_t = int8;
	using output_t = int32;
	static constexpr int shiftBits = fractionalBits + 1 - sizeof(input_t) * 8;
	MPT_FORCEINLINE output_t operator()(input_t val)
	{
		static_assert(fractionalBits >= 0 && fractionalBits <= sizeof(output_t) * 8 - 1);
		static_assert(shiftBits >= 1);
		return mpt::lshift_signed(static_cast<output_t>(val), shiftBits);
	}
};

template <int fractionalBits>
struct ConvertToFixedPoint<int32, int16, fractionalBits>
{
	using input_t = int16;
	using output_t = int32;
	static constexpr int shiftBits = fractionalBits + 1 - sizeof(input_t) * 8;
	MPT_FORCEINLINE output_t operator()(input_t val)
	{
		static_assert(fractionalBits >= 0 && fractionalBits <= sizeof(output_t) * 8 - 1);
		static_assert(shiftBits >= 1);
		return mpt::lshift_signed(static_cast<output_t>(val), shiftBits);
	}
};

template <int fractionalBits>
struct ConvertToFixedPoint<int32, int24, fractionalBits>
{
	using input_t = int24;
	using output_t = int32;
	static constexpr int shiftBits = fractionalBits + 1 - sizeof(input_t) * 8;
	MPT_FORCEINLINE output_t operator()(input_t val)
	{
		static_assert(fractionalBits >= 0 && fractionalBits <= sizeof(output_t) * 8 - 1);
		static_assert(shiftBits >= 1);
		return mpt::lshift_signed(static_cast<output_t>(val), shiftBits);
	}
};

template <int fractionalBits>
struct ConvertToFixedPoint<int32, int32, fractionalBits>
{
	using input_t = int32;
	using output_t = int32;
	MPT_FORCEINLINE output_t operator()(input_t val)
	{
		static_assert(fractionalBits >= 0 && fractionalBits <= sizeof(output_t) * 8 - 1);
		return mpt::rshift_signed(static_cast<output_t>(val), (sizeof(input_t) * 8 - 1 - fractionalBits));
	}
};

template <int fractionalBits>
struct ConvertToFixedPoint<int32, somefloat32, fractionalBits>
{
	using input_t = somefloat32;
	using output_t = int32;
	const float factor;
	MPT_FORCEINLINE ConvertToFixedPoint()
		: factor(static_cast<float>(1 << fractionalBits))
	{
		return;
	}
	MPT_FORCEINLINE output_t operator()(input_t val)
	{
		static_assert(fractionalBits >= 0 && fractionalBits <= sizeof(input_t) * 8 - 1);
		val = mpt::sanitize_nan(val);
		return mpt::saturate_trunc<output_t>(SC::fastround(val * factor));
	}
};

template <int fractionalBits>
struct ConvertToFixedPoint<int32, somefloat64, fractionalBits>
{
	using input_t = somefloat64;
	using output_t = int32;
	const double factor;
	MPT_FORCEINLINE ConvertToFixedPoint()
		: factor(static_cast<double>(1 << fractionalBits))
	{
		return;
	}
	MPT_FORCEINLINE output_t operator()(input_t val)
	{
		static_assert(fractionalBits >= 0 && fractionalBits <= sizeof(input_t) * 8 - 1);
		val = mpt::sanitize_nan(val);
		return mpt::saturate_trunc<output_t>(SC::fastround(val * factor));
	}
};


}  // namespace SC


OPENMPT_NAMESPACE_END
