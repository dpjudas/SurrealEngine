/* SPDX-License-Identifier: BSD-3-Clause */
/* SPDX-FileCopyrightText: OpenMPT Project Developers and Contributors */


#pragma once

#include "openmpt/all/BuildSettings.hpp"

#include "mpt/base/macros.hpp"
#include "openmpt/base/Int24.hpp"
#include "openmpt/base/Types.hpp"


OPENMPT_NAMESPACE_BEGIN


namespace SC
{  // SC = _S_ample_C_onversion


template <typename Tsample, bool clipOutput>
struct Clip;

template <bool clipOutput>
struct Clip<uint8, clipOutput>
{
	using input_t = uint8;
	using output_t = uint8;
	MPT_FORCEINLINE uint8 operator()(uint8 val)
	{
		return val;
	}
};

template <bool clipOutput>
struct Clip<int8, clipOutput>
{
	using input_t = int8;
	using output_t = int8;
	MPT_FORCEINLINE int8 operator()(int8 val)
	{
		return val;
	}
};

template <bool clipOutput>
struct Clip<int16, clipOutput>
{
	using input_t = int16;
	using output_t = int16;
	MPT_FORCEINLINE int16 operator()(int16 val)
	{
		return val;
	}
};

template <bool clipOutput>
struct Clip<int24, clipOutput>
{
	using input_t = int24;
	using output_t = int24;
	MPT_FORCEINLINE int24 operator()(int24 val)
	{
		return val;
	}
};

template <bool clipOutput>
struct Clip<int32, clipOutput>
{
	using input_t = int32;
	using output_t = int32;
	MPT_FORCEINLINE int32 operator()(int32 val)
	{
		return val;
	}
};

template <bool clipOutput>
struct Clip<int64, clipOutput>
{
	using input_t = int64;
	using output_t = int64;
	MPT_FORCEINLINE int64 operator()(int64 val)
	{
		return val;
	}
};

template <bool clipOutput>
struct Clip<float, clipOutput>
{
	using input_t = float;
	using output_t = float;
	MPT_FORCEINLINE float operator()(float val)
	{
		if constexpr(clipOutput)
		{
			if(val < -1.0f) val = -1.0f;
			if(val > 1.0f) val = 1.0f;
			return val;
		} else
		{
			return val;
		}
	}
};

template <bool clipOutput>
struct Clip<double, clipOutput>
{
	using input_t = double;
	using output_t = double;
	MPT_FORCEINLINE double operator()(double val)
	{
		if constexpr(clipOutput)
		{
			if(val < -1.0) val = -1.0;
			if(val > 1.0) val = 1.0;
			return val;
		} else
		{
			return val;
		}
	}
};


}  // namespace SC


OPENMPT_NAMESPACE_END
