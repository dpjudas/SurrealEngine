/* SPDX-License-Identifier: BSD-3-Clause */
/* SPDX-FileCopyrightText: OpenMPT Project Developers and Contributors */


#pragma once

#include "openmpt/all/BuildSettings.hpp"

#include "mpt/base/macros.hpp"


OPENMPT_NAMESPACE_BEGIN


namespace SC
{  // SC = _S_ample_C_onversion


template <typename Tfixed, int fractionalBits, bool clipOutput>
struct ClipFixed
{
	using input_t = Tfixed;
	using output_t = Tfixed;
	MPT_FORCEINLINE Tfixed operator()(Tfixed val)
	{
		static_assert(fractionalBits >= 0 && fractionalBits <= sizeof(output_t) * 8 - 1);
		if constexpr(clipOutput)
		{
			constexpr Tfixed clip_max = (Tfixed(1) << fractionalBits) - Tfixed(1);
			constexpr Tfixed clip_min = Tfixed(0) - (Tfixed(1) << fractionalBits);
			if(val < clip_min) val = clip_min;
			if(val > clip_max) val = clip_max;
			return val;
		} else
		{
			return val;
		}
	}
};


}  // namespace SC


OPENMPT_NAMESPACE_END
