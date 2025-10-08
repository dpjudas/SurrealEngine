/* SPDX-License-Identifier: BSD-3-Clause */
/* SPDX-FileCopyrightText: Olivier Lapicque */
/* SPDX-FileCopyrightText: OpenMPT Project Developers and Contributors */


#pragma once

#include "openmpt/all/BuildSettings.hpp"

#include "mpt/base/arithmetic_shift.hpp"
#include "mpt/base/macros.hpp"
#include "mpt/random/random.hpp"
#include "openmpt/base/Types.hpp"
#include "openmpt/random/ModPlug.hpp"
#include "openmpt/soundbase/MixSample.hpp"
#include "openmpt/soundbase/MixSampleConvert.hpp"


OPENMPT_NAMESPACE_BEGIN


struct Dither_ModPlug
{
public:
	using prng_type = mpt::rng::modplug_dither;
	template <typename Trd>
	static prng_type prng_init(Trd &)
	{
		return prng_type{0, 0};
	}

public:
	template <uint32 targetbits, typename Trng>
	MPT_FORCEINLINE MixSampleInt process(MixSampleInt sample, Trng &rng)
	{
		if constexpr(targetbits == 0)
		{
			MPT_UNUSED(rng);
			return sample;
		} else if constexpr(targetbits + MixSampleIntTraits::mix_headroom_bits + 1 >= 32)
		{
			MPT_UNUSED(rng);
			return sample;
		} else
		{
			sample += mpt::rshift_signed(static_cast<int32>(mpt::random<uint32>(rng)), (targetbits + MixSampleIntTraits::mix_headroom_bits + 1));
			return sample;
		}
	}
	template <uint32 targetbits, typename Trng>
	MPT_FORCEINLINE MixSampleFloat process(MixSampleFloat sample, Trng &prng)
	{
		return mix_sample_cast<MixSampleFloat>(process<targetbits>(mix_sample_cast<MixSampleInt>(sample), prng));
	}
};


OPENMPT_NAMESPACE_END
