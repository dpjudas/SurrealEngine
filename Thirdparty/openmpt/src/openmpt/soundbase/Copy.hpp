/* SPDX-License-Identifier: BSD-3-Clause */
/* SPDX-FileCopyrightText: OpenMPT Project Developers and Contributors */


#pragma once

#include "openmpt/all/BuildSettings.hpp"

#include "mpt/audio/span.hpp"
#include "mpt/base/macros.hpp"
#include "openmpt/soundbase/SampleConvert.hpp"

#include <algorithm>

#include <cassert>
#include <cstddef>



OPENMPT_NAMESPACE_BEGIN



template <typename TBufOut, typename TBufIn>
void CopyAudio(TBufOut buf_out, TBufIn buf_in)
{
	assert(buf_in.size_frames() == buf_out.size_frames());
	assert(buf_in.size_channels() == buf_out.size_channels());
	std::size_t countFrames = std::min(buf_in.size_frames(), buf_out.size_frames());
	std::size_t channels = std::min(buf_in.size_channels(), buf_out.size_channels());
	for(std::size_t frame = 0; frame < countFrames; ++frame)
	{
		for(std::size_t channel = 0; channel < channels; ++channel)
		{
			buf_out(channel, frame) = SC::sample_cast<typename TBufOut::sample_type>(buf_in(channel, frame));
		}
	}
}


template <typename TBufOut, typename TBufIn>
void CopyAudio(TBufOut buf_out, TBufIn buf_in, std::size_t countFrames)
{
	assert(countFrames <= buf_in.size_frames());
	assert(countFrames <= buf_out.size_frames());
	assert(buf_in.size_channels() == buf_out.size_channels());
	std::size_t channels = std::min(buf_in.size_channels(), buf_out.size_channels());
	for(std::size_t frame = 0; frame < countFrames; ++frame)
	{
		for(std::size_t channel = 0; channel < channels; ++channel)
		{
			buf_out(channel, frame) = SC::sample_cast<typename TBufOut::sample_type>(buf_in(channel, frame));
		}
	}
}


template <typename TBufOut, typename TBufIn>
void CopyAudioChannels(TBufOut buf_out, TBufIn buf_in, std::size_t channels, std::size_t countFrames)
{
	assert(countFrames <= buf_in.size_frames());
	assert(countFrames <= buf_out.size_frames());
	assert(channels <= buf_in.size_channels());
	assert(channels <= buf_out.size_channels());
	for(std::size_t frame = 0; frame < countFrames; ++frame)
	{
		for(std::size_t channel = 0; channel < channels; ++channel)
		{
			buf_out(channel, frame) = SC::sample_cast<typename TBufOut::sample_type>(buf_in(channel, frame));
		}
	}
}


// Copy numChannels interleaved sample streams.
template <typename Tin, typename Tout>
void CopyAudioChannelsInterleaved(Tout *MPT_RESTRICT outBuf, const Tin *MPT_RESTRICT inBuf, std::size_t numChannels, std::size_t countFrames)
{
	CopyAudio(mpt::audio_span_interleaved<Tout>(outBuf, numChannels, countFrames), mpt::audio_span_interleaved<const Tin>(inBuf, numChannels, countFrames));
}



OPENMPT_NAMESPACE_END
