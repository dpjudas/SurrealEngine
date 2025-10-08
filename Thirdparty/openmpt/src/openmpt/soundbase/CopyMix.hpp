/* SPDX-License-Identifier: BSD-3-Clause */
/* SPDX-FileCopyrightText: OpenMPT Project Developers and Contributors */


#pragma once

#include "openmpt/all/BuildSettings.hpp"


#include "mpt/audio/span.hpp"
#include "mpt/base/macros.hpp"
#include "openmpt/soundbase/SampleClip.hpp"
#include "openmpt/soundbase/SampleClipFixedPoint.hpp"
#include "openmpt/soundbase/SampleConvert.hpp"
#include "openmpt/soundbase/SampleConvertFixedPoint.hpp"
#include "openmpt/soundbase/SampleFormat.hpp"

#include <algorithm>

#include <cassert>
#include <cstddef>


OPENMPT_NAMESPACE_BEGIN



template <int fractionalBits, bool clipOutput, typename TOutBuf, typename TInBuf, typename Tdither>
void ConvertBufferMixInternalFixedToBuffer(TOutBuf outBuf, TInBuf inBuf, Tdither &dither, std::size_t channels, std::size_t count)
{
	using TOutSample = typename std::remove_const<typename TOutBuf::sample_type>::type;
	using TInSample = typename std::remove_const<typename TInBuf::sample_type>::type;
	assert(inBuf.size_channels() >= channels);
	assert(outBuf.size_channels() >= channels);
	assert(inBuf.size_frames() >= count);
	assert(outBuf.size_frames() >= count);
	constexpr int ditherBits = SampleFormat(SampleFormatTraits<TOutSample>::sampleFormat()).IsInt()
								 ? SampleFormat(SampleFormatTraits<TOutSample>::sampleFormat()).GetBitsPerSample()
								 : 0;
	SC::ClipFixed<int32, fractionalBits, clipOutput> clip;
	SC::ConvertFixedPoint<TOutSample, TInSample, fractionalBits> conv;
	for(std::size_t i = 0; i < count; ++i)
	{
		for(std::size_t channel = 0; channel < channels; ++channel)
		{
			outBuf(channel, i) = conv(clip(dither.template process<ditherBits>(channel, inBuf(channel, i))));
		}
	}
}


template <int fractionalBits, typename TOutBuf, typename TInBuf>
void ConvertBufferToBufferMixInternalFixed(TOutBuf outBuf, TInBuf inBuf, std::size_t channels, std::size_t count)
{
	using TOutSample = typename std::remove_const<typename TOutBuf::sample_type>::type;
	using TInSample = typename std::remove_const<typename TInBuf::sample_type>::type;
	assert(inBuf.size_channels() >= channels);
	assert(outBuf.size_channels() >= channels);
	assert(inBuf.size_frames() >= count);
	assert(outBuf.size_frames() >= count);
	SC::ConvertToFixedPoint<TOutSample, TInSample, fractionalBits> conv;
	for(std::size_t i = 0; i < count; ++i)
	{
		for(std::size_t channel = 0; channel < channels; ++channel)
		{
			outBuf(channel, i) = conv(inBuf(channel, i));
		}
	}
}


template <bool clipOutput, typename TOutBuf, typename TInBuf, typename Tdither>
void ConvertBufferMixInternalToBuffer(TOutBuf outBuf, TInBuf inBuf, Tdither &dither, std::size_t channels, std::size_t count)
{
	using TOutSample = typename std::remove_const<typename TOutBuf::sample_type>::type;
	using TInSample = typename std::remove_const<typename TInBuf::sample_type>::type;
	assert(inBuf.size_channels() >= channels);
	assert(outBuf.size_channels() >= channels);
	assert(inBuf.size_frames() >= count);
	assert(outBuf.size_frames() >= count);
	constexpr int ditherBits = SampleFormat(SampleFormatTraits<TOutSample>::sampleFormat()).IsInt()
								 ? SampleFormat(SampleFormatTraits<TOutSample>::sampleFormat()).GetBitsPerSample()
								 : 0;
	SC::Clip<TInSample, clipOutput> clip;
	SC::Convert<TOutSample, TInSample> conv;
	for(std::size_t i = 0; i < count; ++i)
	{
		for(std::size_t channel = 0; channel < channels; ++channel)
		{
			outBuf(channel, i) = conv(clip(dither.template process<ditherBits>(channel, inBuf(channel, i))));
		}
	}
}


template <typename TOutBuf, typename TInBuf>
void ConvertBufferToBufferMixInternal(TOutBuf outBuf, TInBuf inBuf, std::size_t channels, std::size_t count)
{
	using TOutSample = typename std::remove_const<typename TOutBuf::sample_type>::type;
	using TInSample = typename std::remove_const<typename TInBuf::sample_type>::type;
	assert(inBuf.size_channels() >= channels);
	assert(outBuf.size_channels() >= channels);
	assert(inBuf.size_frames() >= count);
	assert(outBuf.size_frames() >= count);
	SC::Convert<TOutSample, TInSample> conv;
	for(std::size_t i = 0; i < count; ++i)
	{
		for(std::size_t channel = 0; channel < channels; ++channel)
		{
			outBuf(channel, i) = conv(inBuf(channel, i));
		}
	}
}


OPENMPT_NAMESPACE_END
