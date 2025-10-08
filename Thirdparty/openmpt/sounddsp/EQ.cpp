/*
 * EQ.cpp
 * ------
 * Purpose: Mixing code for equalizer.
 * Notes  : Ugh... This should really be removed at some point.
 * Authors: Olivier Lapicque
 *          OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"

#include "EQ.h"

#include "mpt/audio/span.hpp"
#include "mpt/base/numbers.hpp"
#include "openmpt/base/Types.hpp"
#include "openmpt/soundbase/MixSample.hpp"
#include "openmpt/soundbase/MixSampleConvert.hpp"

#if defined(MPT_WANT_ARCH_INTRINSICS_X86_SSE)
#include "../common/mptCPU.h"
#endif

#include <algorithm>
#include <array>

#include <cstddef>

#if defined(MPT_WANT_ARCH_INTRINSICS_X86_SSE) && defined(MPT_ARCH_INTRINSICS_X86_SSE)
#if MPT_COMPILER_MSVC
#include <intrin.h>
#endif
#include <xmmintrin.h>
#endif


OPENMPT_NAMESPACE_BEGIN


#ifndef NO_EQ


static constexpr float EQ_BANDWIDTH = 2.0f;


static constexpr std::array<uint32, 33> gEqLinearToDB =
{
	16, 19, 22, 25, 28, 31, 34, 37,
	40, 43, 46, 49, 52, 55, 58, 61,
	64, 76, 88, 100, 112, 124, 136, 148,
	160, 172, 184, 196, 208, 220, 232, 244, 256
};


static constexpr std::array<EQBANDSETTINGS, MAX_EQ_BANDS> gEQDefaults =
{{
	// Default: Flat EQ
	{0,0,0,0,0, 1,   120},
	{0,0,0,0,0, 1,   600},
	{0,0,0,0,0, 1,  1200},
	{0,0,0,0,0, 1,  3000},
	{0,0,0,0,0, 1,  6000},
	{0,0,0,0,0, 1, 10000}
}};


template <std::size_t channels, typename Tbuf>
static void EQFilter(Tbuf & buf, const std::array<EQBANDSETTINGS, MAX_EQ_BANDS> &bands, std::array<std::array<EQBANDSTATE, MAX_EQ_BANDS>, MAX_EQ_CHANNELS> &states)
{
	for(std::size_t frame = 0; frame < buf.size_frames(); ++frame)
	{
		for(std::size_t channel = 0; channel < channels; ++channel)
		{
			float sample = mix_sample_cast<float>(buf(channel, frame));
			for(std::size_t b = 0; b < std::size(bands); ++b)
			{
				const EQBANDSETTINGS &band = bands[b];
				if(band.Gain != 1.0f)
				{
					EQBANDSTATE &bandState = states[channel][b];
					float x = sample;
					float y = band.a1 * bandState.x1 + band.a2 * bandState.x2 + band.a0 * x + band.b1 * bandState.y1 + band.b2 * bandState.y2;
					bandState.x2 = bandState.x1;
					bandState.y2 = bandState.y1;
					bandState.x1 = x;
					bandState.y1 = y;
					sample = y;
				}
			}
			buf(channel, frame) = mix_sample_cast<typename Tbuf::sample_type>(sample);
		}
	}
}


template <typename TMixSample>
void CEQ::ProcessTemplate(TMixSample *frontBuffer, TMixSample *rearBuffer, std::size_t countFrames, std::size_t numChannels)
{
#if defined(MPT_WANT_ARCH_INTRINSICS_X86_SSE) && defined(MPT_ARCH_INTRINSICS_X86_SSE)
	unsigned int old_csr = 0;
	if(CPU::HasFeatureSet(CPU::feature::sse) && CPU::HasModesEnabled(CPU::mode::xmm128sse))
	{
		mpt::arch::feature_fence_aquire();
		old_csr = _mm_getcsr();
		_mm_setcsr((old_csr & ~(_MM_DENORMALS_ZERO_MASK | _MM_FLUSH_ZERO_MASK)) | _MM_DENORMALS_ZERO_ON | _MM_FLUSH_ZERO_ON);
	}
#endif
	if(numChannels == 1)
	{
		mpt::audio_span_interleaved<TMixSample> buf{ frontBuffer, 1, countFrames };
		EQFilter<1>(buf, m_Bands, m_ChannelState);
	} else if(numChannels == 2)
	{
		mpt::audio_span_interleaved<TMixSample> buf{ frontBuffer, 2, countFrames };
		EQFilter<2>(buf, m_Bands, m_ChannelState);
	} else if(numChannels == 4)
	{
		std::array<TMixSample*, 4> buffers = { &frontBuffer[0], &frontBuffer[1], &rearBuffer[0], &rearBuffer[1] };
		mpt::audio_span_planar_strided<TMixSample> buf{ buffers.data(), 4, countFrames, 2 };
		EQFilter<4>(buf, m_Bands, m_ChannelState);
	}
#if defined(MPT_WANT_ARCH_INTRINSICS_X86_SSE) && defined(MPT_ARCH_INTRINSICS_X86_SSE)
	if(CPU::HasFeatureSet(CPU::feature::sse) && CPU::HasModesEnabled(CPU::mode::xmm128sse))
	{
		_mm_setcsr(old_csr);
		mpt::arch::feature_fence_release();
	}
#endif
}


void CEQ::Process(MixSampleInt *frontBuffer, MixSampleInt *rearBuffer, std::size_t countFrames, std::size_t numChannels)
{
	ProcessTemplate<MixSampleInt>(frontBuffer, rearBuffer, countFrames, numChannels);
}


void CEQ::Process(MixSampleFloat *frontBuffer, MixSampleFloat *rearBuffer, std::size_t countFrames, std::size_t numChannels)
{
	ProcessTemplate<MixSampleFloat>(frontBuffer, rearBuffer, countFrames, numChannels);
}


CEQ::CEQ()
	: m_Bands(gEQDefaults)
{
	return;
}


void CEQ::Initialize(bool bReset, uint32 MixingFreq)
{
	float fMixingFreq = static_cast<float>(MixingFreq);
	// Gain = 0.5 (-6dB) .. 2 (+6dB)
	for(std::size_t band = 0; band < MAX_EQ_BANDS; ++band)
	{
		float k, k2, r, f;
		float v0, v1;
		bool b = bReset;
		f = m_Bands[band].CenterFrequency / fMixingFreq;
		if(f > 0.45f)
		{
			m_Bands[band].Gain = 1.0f;
		}
		k = f * mpt::numbers::pi_v<float>;
		k = k + k*f;
		k2 = k*k;
		v0 = m_Bands[band].Gain;
		v1 = 1;
		if(m_Bands[band].Gain < 1.0f)
		{
			v0 *= (0.5f/EQ_BANDWIDTH);
			v1 *= (0.5f/EQ_BANDWIDTH);
		} else
		{
			v0 *= (1.0f/EQ_BANDWIDTH);
			v1 *= (1.0f/EQ_BANDWIDTH);
		}
		r = (1 + v0*k + k2) / (1 + v1*k + k2);
		if(r != m_Bands[band].a0)
		{
			m_Bands[band].a0 = r;
			b = true;
		}
		r = 2 * (k2 - 1) / (1 + v1*k + k2);
		if(r != m_Bands[band].a1)
		{
			m_Bands[band].a1 = r;
			b = true;
		}
		r = (1 - v0*k + k2) / (1 + v1*k + k2);
		if(r != m_Bands[band].a2)
		{
			m_Bands[band].a2 = r;
			b = true;
		}
		r = - 2 * (k2 - 1) / (1 + v1*k + k2);
		if(r != m_Bands[band].b1)
		{
			m_Bands[band].b1 = r;
			b = true;
		}
		r = - (1 - v1*k + k2) / (1 + v1*k + k2);
		if(r != m_Bands[band].b2)
		{
			m_Bands[band].b2 = r;
			b = true;
		}
		if(b)
		{
			for(std::size_t channel = 0; channel < MAX_EQ_CHANNELS; ++channel)
			{
				m_ChannelState[channel][band] = EQBANDSTATE{};
			}
		}
	}
}


void CEQ::SetEQGains(const uint32 *pGains, const uint32 *pFreqs, bool bReset, uint32 MixingFreq)
{
	for(std::size_t i = 0; i < MAX_EQ_BANDS; ++i)
	{
		m_Bands[i].Gain = static_cast<float>(gEqLinearToDB[std::clamp(pGains[i], static_cast<uint32>(0), static_cast<uint32>(std::size(gEqLinearToDB) - 1))]) / 64.0f;
		m_Bands[i].CenterFrequency = static_cast<float>(pFreqs[i]);
	}
	Initialize(bReset, MixingFreq);
}


#else


MPT_MSVC_WORKAROUND_LNK4221(EQ)


#endif // !NO_EQ


OPENMPT_NAMESPACE_END
