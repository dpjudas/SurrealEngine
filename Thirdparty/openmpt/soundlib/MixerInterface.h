/*
 * MixerInterface.h
 * ----------------
 * Purpose: The basic mixer interface and main mixer loop, completely agnostic of the actual sample input / output formats.
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#pragma once

#include "openmpt/all/BuildSettings.hpp"

#include "Snd_defs.h"
#include "ModChannel.h"

OPENMPT_NAMESPACE_BEGIN

class CResampler;

//////////////////////////////////////////////////////////////////////////
// Sample conversion traits

template<int channelsOut, int channelsIn, typename out, typename in>
struct MixerTraits
{
	static constexpr int numChannelsIn = channelsIn;    // Number of channels in sample
	static constexpr int numChannelsOut = channelsOut;  // Number of mixer output channels
	using output_t = out;                               // Output buffer sample type
	using input_t = in;                                 // Input buffer sample type
	using outbuf_t = out[channelsOut];                  // Output buffer sampling point type
	// To perform sample conversion, add a function with the following signature to your derived classes:
	// static MPT_CONSTEXPRINLINE output_t Convert(const input_t x)
};


//////////////////////////////////////////////////////////////////////////
// Interpolation templates

template<class Traits>
struct NoInterpolation
{
	ModChannel &channel;

	MPT_FORCEINLINE NoInterpolation(ModChannel &c, const CResampler &, unsigned int)
		: channel{c}
	{
		// Adding 0.5 to the sample position before the interpolation loop starts
		// effectively gives us nearest-neighbour with rounding instead of truncation.
		// This gives us more consistent behaviour between forward and reverse playing of a sample.
		c.position += SamplePosition::Ratio(1, 2);
	}
	MPT_FORCEINLINE ~NoInterpolation()
	{
		channel.position -= SamplePosition::Ratio(1, 2);
	}

	MPT_FORCEINLINE void operator() (typename Traits::outbuf_t &outSample, const typename Traits::input_t * const inBuffer, const int32)
	{
		static_assert(static_cast<int>(Traits::numChannelsIn) <= static_cast<int>(Traits::numChannelsOut), "Too many input channels");

		for(int i = 0; i < Traits::numChannelsIn; i++)
		{
			outSample[i] = Traits::Convert(inBuffer[i]);
		}
	}
};

// Other interpolation algorithms depend on the input format type (integer / float) and can thus be found in FloatMixer.h and IntMixer.h


//////////////////////////////////////////////////////////////////////////
// Main sample render loop template

// Template parameters:
// Traits: A class derived from MixerTraits that defines the number of channels, sample buffer types, etc..
// InterpolationFunc: Functor for reading the sample data and doing the SRC
// FilterFunc: Functor for applying the resonant filter
// MixFunc: Functor for mixing the computed sample data into the output buffer
template<class Traits, class InterpolationFunc, class FilterFunc, class MixFunc>
inline void SampleLoop(ModChannel &chn, const CResampler &resampler, typename Traits::output_t * MPT_RESTRICT outBuffer, unsigned int numSamples)
{
	ModChannel &c = chn;
	const typename Traits::input_t * MPT_RESTRICT inSample = static_cast<const typename Traits::input_t *>(c.pCurrentSample);

	InterpolationFunc interpolate{c, resampler, numSamples};
	FilterFunc filter{c};
	MixFunc mix{c};

	unsigned int samples = numSamples;
	SamplePosition smpPos = c.position;            // Fixed-point sample position
	const SamplePosition increment = c.increment;  // Fixed-point sample increment

	while(samples--)
	{
		typename Traits::outbuf_t outSample;
		interpolate(outSample, inSample + smpPos.GetInt() * Traits::numChannelsIn, smpPos.GetFract());
		filter(outSample, c);
		mix(outSample, c, outBuffer);
		outBuffer += Traits::numChannelsOut;

		smpPos += increment;
	}

	c.position = smpPos;
}

// Type of the SampleLoop function above
using MixFuncInterface = void (*)(ModChannel &, const CResampler &, mixsample_t *, unsigned int);

OPENMPT_NAMESPACE_END
