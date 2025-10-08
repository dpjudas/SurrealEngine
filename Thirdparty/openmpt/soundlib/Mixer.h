/*
 * Mixer.h
 * -------
 * Purpose: Basic mixer constants
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#pragma once

#include "openmpt/all/BuildSettings.hpp"

#include "openmpt/soundbase/MixSample.hpp"

OPENMPT_NAMESPACE_BEGIN

#define MPT_INTMIXER

#ifdef MPT_INTMIXER
using mixsample_t = MixSampleIntTraits::sample_type;
enum { MIXING_FILTER_PRECISION = MixSampleIntTraits::filter_precision_bits };  // Fixed point resonant filter bits
#else
using mixsample_t = MixSampleFloat;
#endif
enum { MIXING_ATTENUATION = MixSampleIntTraits::mix_headroom_bits };
enum { MIXING_FRACTIONAL_BITS = MixSampleIntTraits::mix_fractional_bits };

inline constexpr float MIXING_SCALEF = MixSampleIntTraits::mix_scale<float>;

#ifdef MPT_INTMIXER
static_assert(sizeof(mixsample_t) == 4);
static_assert(MIXING_FILTER_PRECISION == 24);
static_assert(MIXING_ATTENUATION == 4);
static_assert(MIXING_FRACTIONAL_BITS == 27);
static_assert(MixSampleIntTraits::mix_clip_max == int32(0x7FFFFFF));
static_assert(MixSampleIntTraits::mix_clip_min == (0 - int32(0x7FFFFFF)));
static_assert(MIXING_SCALEF == 134217728.0f);
#else
static_assert(sizeof(mixsample_t) == 4);
#endif

#define MIXBUFFERSIZE 512
#define NUMMIXINPUTBUFFERS 4

#define VOLUMERAMPPRECISION 12	// Fractional bits in volume ramp variables

// The absolute maximum number of sampling points any interpolation algorithm is going to look at in any direction from the current sampling point
// Currently, the maximum is 4 sampling points forwards and 3 sampling points backwards (Polyphase / FIR algorithms).
// Hence, this value must be at least 4.
inline constexpr uint8 InterpolationMaxLookahead = 4;
// While we could directly use the previous value in various places such as the interpolation wrap-around handling at loop points,
// choosing a higher value (e.g. 16) will reduce CPU usage when using many extremely short (length < 16) samples.
inline constexpr uint8 InterpolationLookaheadBufferSize = 16;

static_assert(InterpolationLookaheadBufferSize >= InterpolationMaxLookahead);

// Maximum size of a sampling point of a sample, in bytes.
// The biggest sampling point size is currently 16-bit stereo = 2 * 2 bytes.
inline constexpr uint8 MaxSamplingPointSize = 4;

OPENMPT_NAMESPACE_END
