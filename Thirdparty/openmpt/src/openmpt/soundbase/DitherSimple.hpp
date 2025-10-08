/* SPDX-License-Identifier: BSD-3-Clause */
/* SPDX-FileCopyrightText: OpenMPT Project Developers and Contributors */


#pragma once

#include "openmpt/all/BuildSettings.hpp"

#include "mpt/base/macros.hpp"
#include "mpt/random/engine.hpp"
#include "mpt/random/default_engines.hpp"
#include "mpt/random/random.hpp"
#include "mpt/random/seed.hpp"
#include "openmpt/base/Types.hpp"
#include "openmpt/soundbase/MixSample.hpp"
#include "openmpt/soundbase/MixSampleConvert.hpp"


OPENMPT_NAMESPACE_BEGIN


template <int ditherdepth = 1, bool triangular = false, bool shaped = true>
struct Dither_SimpleImpl
{
public:
	using prng_type = mpt::fast_engine;
	template <typename Trd>
	static prng_type prng_init(Trd &rd)
	{
		return mpt::make_prng<prng_type>(rd);
	}

private:
	int32 error = 0;

public:
	template <uint32 targetbits, typename Trng>
	MPT_FORCEINLINE MixSampleInt process(MixSampleInt sample, Trng &prng)
	{
		if constexpr(targetbits == 0)
		{
			MPT_UNUSED(prng);
			return sample;
		} else
		{
			static_assert(sizeof(MixSampleInt) == 4);
			constexpr int rshift = (32 - targetbits) - MixSampleIntTraits::mix_headroom_bits;
			if constexpr(rshift <= 1)
			{
				MPT_UNUSED(prng);
				// nothing to dither
				return sample;
			} else
			{
				constexpr int rshiftpositive = (rshift > 1) ? rshift : 1;  // work-around warnings about negative shift with C++14 compilers
				constexpr int round_mask = ~((1 << rshiftpositive) - 1);
				constexpr int round_offset = 1 << (rshiftpositive - 1);
				constexpr int noise_bits = rshiftpositive + (ditherdepth - 1);
				constexpr int noise_bias = (1 << (noise_bits - 1));
				int32 e = error;
				unsigned int unoise = 0;
				if constexpr(triangular)
				{
					unoise = (mpt::random<unsigned int>(prng, noise_bits) + mpt::random<unsigned int>(prng, noise_bits)) >> 1;
				} else
				{
					unoise = mpt::random<unsigned int>(prng, noise_bits);
				}
				int noise = static_cast<int>(unoise) - noise_bias;  // un-bias
				int val = sample;
				if constexpr(shaped)
				{
					val += (e >> 1);
				}
				int rounded = (val + noise + round_offset) & round_mask;
				e = val - rounded;
				sample = rounded;
				error = e;
				return sample;
			}
		}
	}
	template <uint32 targetbits, typename Trng>
	MPT_FORCEINLINE MixSampleFloat process(MixSampleFloat sample, Trng &prng)
	{
		return mix_sample_cast<MixSampleFloat>(process<targetbits>(mix_sample_cast<MixSampleInt>(sample), prng));
	}
};

using Dither_Simple = Dither_SimpleImpl<>;


OPENMPT_NAMESPACE_END
