/* SPDX-License-Identifier: BSD-3-Clause */
/* SPDX-FileCopyrightText: OpenMPT Project Developers and Contributors */


#pragma once

#include "openmpt/all/BuildSettings.hpp"

#include "mpt/base/macros.hpp"
#include "openmpt/base/Types.hpp"
#include "openmpt/soundbase/MixSample.hpp"


OPENMPT_NAMESPACE_BEGIN


struct Dither_None
{
public:
	using prng_type = struct
	{
	};
	template <typename Trd>
	static prng_type prng_init(Trd &)
	{
		return prng_type{};
	}

public:
	template <uint32 targetbits, typename Trng>
	MPT_FORCEINLINE MixSampleInt process(MixSampleInt sample, Trng &)
	{
		return sample;
	}
	template <uint32 targetbits, typename Trng>
	MPT_FORCEINLINE MixSampleFloat process(MixSampleFloat sample, Trng &)
	{
		return sample;
	}
};


OPENMPT_NAMESPACE_END
