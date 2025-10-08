/* SPDX-License-Identifier: BSD-3-Clause */
/* SPDX-FileCopyrightText: Olivier Lapicque */
/* SPDX-FileCopyrightText: OpenMPT Project Developers and Contributors */


#pragma once

#include "openmpt/all/BuildSettings.hpp"

#include "mpt/audio/sample.hpp"
#include "mpt/base/float.hpp"

#include <type_traits>

#include <cstddef>


OPENMPT_NAMESPACE_BEGIN


template <typename Tsample, std::size_t MIX_HEADROOM_BITS, std::size_t FILTER_HEADROOM_BITS>
struct FixedPointSampleTraits
{
	static_assert(std::is_integral<Tsample>::value);
	static_assert(std::is_signed<Tsample>::value);
	static_assert((sizeof(Tsample) * 8u) - 1 > MIX_HEADROOM_BITS);
	static_assert((sizeof(Tsample) * 8u) - 1 > FILTER_HEADROOM_BITS);
	using sample_type = Tsample;
	enum class sample_type_strong : sample_type
	{
	};
	static constexpr int mix_headroom_bits = static_cast<int>(MIX_HEADROOM_BITS);
	static constexpr int mix_precision_bits = static_cast<int>((sizeof(Tsample) * 8) - MIX_HEADROOM_BITS);       // including sign bit
	static constexpr int mix_fractional_bits = static_cast<int>((sizeof(Tsample) * 8) - 1 - MIX_HEADROOM_BITS);  // excluding sign bit
	static constexpr sample_type mix_clip_max = ((sample_type(1) << mix_fractional_bits) - sample_type(1));
	static constexpr sample_type mix_clip_min = -((sample_type(1) << mix_fractional_bits) - sample_type(1));
	static constexpr int filter_headroom_bits = static_cast<int>(FILTER_HEADROOM_BITS);
	static constexpr int filter_precision_bits = static_cast<int>((sizeof(Tsample) * 8) - FILTER_HEADROOM_BITS);       // including sign bit
	static constexpr int filter_fractional_bits = static_cast<int>((sizeof(Tsample) * 8) - 1 - FILTER_HEADROOM_BITS);  // excluding sign bit
	template <typename Tfloat>
	static constexpr Tfloat mix_scale = static_cast<Tfloat>(sample_type(1) << mix_fractional_bits);
};

using MixSampleIntTraits = FixedPointSampleTraits<int32, 4, 8>;

using MixSampleInt = MixSampleIntTraits::sample_type;
using MixSampleFloat = mpt::audio_sample_float;

using MixSample = std::conditional<mpt::float_traits<nativefloat>::is_hard, MixSampleFloat, MixSampleInt>::type;


OPENMPT_NAMESPACE_END
