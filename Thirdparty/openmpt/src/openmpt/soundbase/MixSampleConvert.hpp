/* SPDX-License-Identifier: BSD-3-Clause */
/* SPDX-FileCopyrightText: OpenMPT Project Developers and Contributors */


#pragma once

#include "openmpt/all/BuildSettings.hpp"

#include "mpt/base/macros.hpp"
#include "openmpt/soundbase/MixSample.hpp"
#include "openmpt/soundbase/SampleConvert.hpp"
#include "openmpt/soundbase/SampleConvertFixedPoint.hpp"


OPENMPT_NAMESPACE_BEGIN


template <typename Tdst, typename Tsrc>
struct ConvertMixSample;

template <>
struct ConvertMixSample<MixSampleInt, MixSampleInt>
{
	MPT_FORCEINLINE MixSampleInt conv(MixSampleInt src)
	{
		return src;
	}
};

template <>
struct ConvertMixSample<MixSampleFloat, MixSampleFloat>
{
	MPT_FORCEINLINE MixSampleFloat conv(MixSampleFloat src)
	{
		return src;
	}
};

template <typename Tsrc>
struct ConvertMixSample<MixSampleInt, Tsrc>
{
	MPT_FORCEINLINE MixSampleInt conv(Tsrc src)
	{
		return SC::ConvertToFixedPoint<MixSampleInt, Tsrc, MixSampleIntTraits::mix_fractional_bits>{}(src);
	}
};

template <typename Tdst>
struct ConvertMixSample<Tdst, MixSampleInt>
{
	MPT_FORCEINLINE Tdst conv(MixSampleInt src)
	{
		return SC::ConvertFixedPoint<Tdst, MixSampleInt, MixSampleIntTraits::mix_fractional_bits>{}(src);
	}
};

template <typename Tsrc>
struct ConvertMixSample<MixSampleFloat, Tsrc>
{
	MPT_FORCEINLINE MixSampleFloat conv(Tsrc src)
	{
		return SC::Convert<MixSampleFloat, Tsrc>{}(src);
	}
};

template <typename Tdst>
struct ConvertMixSample<Tdst, MixSampleFloat>
{
	MPT_FORCEINLINE Tdst conv(MixSampleFloat src)
	{
		return SC::Convert<Tdst, MixSampleFloat>{}(src);
	}
};

template <>
struct ConvertMixSample<MixSampleInt, MixSampleFloat>
{
	MPT_FORCEINLINE MixSampleInt conv(MixSampleFloat src)
	{
		return SC::ConvertToFixedPoint<MixSampleInt, MixSampleFloat, MixSampleIntTraits::mix_fractional_bits>{}(src);
	}
};

template <>
struct ConvertMixSample<MixSampleFloat, MixSampleInt>
{
	MPT_FORCEINLINE MixSampleFloat conv(MixSampleInt src)
	{
		return SC::ConvertFixedPoint<MixSampleFloat, MixSampleInt, MixSampleIntTraits::mix_fractional_bits>{}(src);
	}
};


template <typename Tdst, typename Tsrc>
MPT_FORCEINLINE Tdst mix_sample_cast(Tsrc src)
{
	return ConvertMixSample<Tdst, Tsrc>{}.conv(src);
}


OPENMPT_NAMESPACE_END
