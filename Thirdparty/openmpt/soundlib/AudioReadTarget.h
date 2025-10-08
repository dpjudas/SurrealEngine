/*
 * AudioReadTarget.h
 * -----------------
 * Purpose: Callback class implementations for audio data read via CSoundFile::Read.
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */

#pragma once

#include "openmpt/all/BuildSettings.hpp"

#include "mpt/audio/span.hpp"
#include "mpt/base/macros.hpp"
#include "mpt/string/types.hpp"
#include "openmpt/soundbase/SampleFormat.hpp"
#include "openmpt/soundbase/CopyMix.hpp"
#include "openmpt/soundbase/Dither.hpp"
#include "openmpt/soundbase/DitherModPlug.hpp"
#include "openmpt/soundbase/DitherNone.hpp"
#include "openmpt/soundbase/DitherSimple.hpp"

#include "MixerLoops.h"
#include "Mixer.h"
#include "Sndfile.h"

#include <type_traits>
#include <variant>

#include <cstddef>


OPENMPT_NAMESPACE_BEGIN


using Dither_Default = Dither_Simple;


class DitherNamesOpenMPT
{
public:
	static mpt::ustring GetModeName(std::size_t mode)
	{
		mpt::ustring result;
		switch(mode)
		{
			case 0:
				// no dither
				result = MPT_USTRING("no");
				break;
			case 1:
				// chosen by OpenMPT code, might change
				result = MPT_USTRING("default");
				break;
			case 2:
				// rectangular, 0.5 bit depth, no noise shaping (original ModPlug Tracker)
				result = MPT_USTRING("0.5 bit");
				break;
			case 3:
				// rectangular, 1 bit depth, simple 1st order noise shaping
				result = MPT_USTRING("1 bit");
				break;
			default:
				result = MPT_USTRING("");
				break;
		}
		return result;
	}
};


using DithersOpenMPT =
	Dithers<std::variant<MultiChannelDither<Dither_None>, MultiChannelDither<Dither_Default>, MultiChannelDither<Dither_ModPlug>, MultiChannelDither<Dither_Simple>>, DitherNamesOpenMPT, 4, 1, 0, mpt::good_prng>;


struct DithersWrapperOpenMPT
	: DithersOpenMPT
{
	template <typename Trd>
	DithersWrapperOpenMPT(Trd &rd, std::size_t mode = DithersOpenMPT::DefaultDither, std::size_t channels = DithersOpenMPT::DefaultChannels)
		: DithersOpenMPT(rd, mode, channels)
	{
		return;
	}
};


template <typename Taudio_span, typename TDithers = DithersOpenMPT>
class AudioTargetBuffer
	: public IAudioTarget
{
private:
	std::size_t countRendered;
	TDithers &dithers;
protected:
	Taudio_span outputBuffer;
public:
	AudioTargetBuffer(Taudio_span buf, TDithers &dithers_)
		: countRendered(0)
		, dithers(dithers_)
		, outputBuffer(buf)
	{
		return;
	}
	std::size_t GetRenderedCount() const { return countRendered; }
public:
	void Process(mpt::audio_span_interleaved<MixSampleInt> buffer) override
	{
		std::visit(
			[&](auto &ditherInstance)
			{
				ConvertBufferMixInternalFixedToBuffer<MixSampleIntTraits::mix_fractional_bits, false>(mpt::make_audio_span_with_offset(outputBuffer, countRendered), buffer, ditherInstance, buffer.size_channels(), buffer.size_frames());
			},
			dithers.Variant()
		);
		countRendered += buffer.size_frames();
	}
	void Process(mpt::audio_span_interleaved<MixSampleFloat> buffer) override
	{
		std::visit(
			[&](auto &ditherInstance)
			{
				ConvertBufferMixInternalToBuffer<false>(mpt::make_audio_span_with_offset(outputBuffer, countRendered), buffer, ditherInstance, buffer.size_channels(), buffer.size_frames());
			},
			dithers.Variant()
		);
		countRendered += buffer.size_frames();
	}
};


template <typename Taudio_span, typename TDithers = DithersOpenMPT>
class AudioTargetBufferWithGain
	: public AudioTargetBuffer<Taudio_span>
{
private:
	using Tbase = AudioTargetBuffer<Taudio_span>;
private:
	const MixSampleFloat gainFactor;
public:
	AudioTargetBufferWithGain(Taudio_span buf, TDithers &dithers, float gainFactor_)
		: Tbase(buf, dithers)
		, gainFactor(static_cast<MixSampleFloat>(gainFactor_))
	{
		return;
	}
public:
	void Process(mpt::audio_span_interleaved<MixSampleInt> buffer) override
	{
		const std::size_t countRendered_ = Tbase::GetRenderedCount();
		if constexpr(!std::is_floating_point<typename Taudio_span::sample_type>::value)
		{
			int32 gainFactor16_16 = mpt::saturate_round<int32>(gainFactor * (1 << 16));
			if(gainFactor16_16 != (1<<16))
			{
				// only apply gain when != +/- 0dB
				// no clipping prevention is done here
				for(std::size_t frame = 0; frame < buffer.size_frames(); ++frame)
				{
					for(std::size_t channel = 0; channel < buffer.size_channels(); ++channel)
					{
						buffer(channel, frame) = Util::muldiv(buffer(channel, frame), gainFactor16_16, 1 << 16);
					}
				}
			}
		}
		Tbase::Process(buffer);
		if constexpr(std::is_floating_point<typename Taudio_span::sample_type>::value)
		{
			if(gainFactor != MixSampleFloat(1.0))
			{
				// only apply gain when != +/- 0dB
				for(std::size_t frame = 0; frame < buffer.size_frames(); ++frame)
				{
					for(std::size_t channel = 0; channel < buffer.size_channels(); ++channel)
					{
						Tbase::outputBuffer(channel, countRendered_ + frame) *= static_cast<typename Taudio_span::sample_type>(gainFactor);
					}
				}
			}
		}
	}
	void Process(mpt::audio_span_interleaved<MixSampleFloat> buffer) override
	{
		if(gainFactor != MixSampleFloat(1.0))
		{
			// only apply gain when != +/- 0dB
			for(std::size_t frame = 0; frame < buffer.size_frames(); ++frame)
			{
				for(std::size_t channel = 0; channel < buffer.size_channels(); ++channel)
				{
					buffer(channel, frame) *= gainFactor;
				}
			}
		}
		Tbase::Process(buffer);
	}
};


OPENMPT_NAMESPACE_END
