/*
 * Snd_flt.cpp
 * -----------
 * Purpose: Calculation of resonant filter coefficients.
 * Notes  : Extended filter range was introduced in MPT 1.12 and went up to 8652 Hz.
 *          MPT 1.16 upped this to the current 10670 Hz.
 *          We have no way of telling whether a file was made with MPT 1.12 or 1.16 though.
 * Authors: Olivier Lapicque
 *          OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"
#include "Sndfile.h"
#include "../common/misc_util.h"
#include "mpt/base/numbers.hpp"


OPENMPT_NAMESPACE_BEGIN


// AWE32: cutoff = reg[0-255] * 31.25 + 100 -> [100Hz-8060Hz]
// EMU10K1 docs: cutoff = reg[0-127]*62+100


uint8 CSoundFile::FrequencyToCutOff(double frequency) const
{
	// IT Cutoff is computed as cutoff = 110 * 2 ^ (0.25 + x/y), where x is the cutoff and y defines the filter range.
	// Reversed, this gives us x = (log2(cutoff / 110) - 0.25) * y.
	// <==========> Rewrite as x = (log2(cutoff) - log2(110) - 0.25) * y.
	// <==========> Rewrite as x = (ln(cutoff) - ln(110) - 0.25*ln(2)) * y/ln(2).
	//                                           <4.8737671609324025>
	double cutoff = (std::log(frequency) - 4.8737671609324025) * (m_SongFlags[SONG_EXFILTERRANGE] ? (20.0 / mpt::numbers::ln2) : (24.0 / mpt::numbers::ln2));
	Limit(cutoff, 0.0, 127.0);
	return mpt::saturate_round<uint8>(cutoff);
}


float CSoundFile::CutOffToFrequency(uint32 nCutOff, int envModifier) const
{
	MPT_ASSERT(nCutOff < 128);
	float computedCutoff = static_cast<float>(nCutOff * (envModifier + 256));	// 0...127*512
	float frequency;
	if(GetType() != MOD_TYPE_IMF)
	{
		frequency = 110.0f * std::pow(2.0f, 0.25f + computedCutoff / (m_SongFlags[SONG_EXFILTERRANGE] ? 20.0f * 512.0f : 24.0f * 512.0f));
	} else
	{
		// EMU8000: Documentation says the cutoff is in quarter semitones, with 0x00 being 125 Hz and 0xFF being 8 kHz
		// The first half of the sentence contradicts the second, though.
		frequency = 125.0f * std::pow(2.0f, computedCutoff * 6.0f / (127.0f * 512.0f));
	}
	Limit(frequency, 120.0f, 20000.0f);
	LimitMax(frequency, static_cast<float>(m_MixerSettings.gdwMixingFreq) * 0.5f);
	return frequency;
}


// Update channels with instrument filter settings updated through tracker UI
void CSoundFile::UpdateInstrumentFilter(const ModInstrument &ins, bool updateMode, bool updateCutoff, bool updateResonance)
{
	for(auto &chn : m_PlayState.Chn)
	{
		if(chn.pModInstrument != &ins)
			continue;

		bool change = false;
		if(updateMode && ins.filterMode != FilterMode::Unchanged && chn.nFilterMode != ins.filterMode)
		{
			chn.nFilterMode = ins.filterMode;
			change = true;
		}
		if(updateCutoff)
		{
			chn.nCutOff = ins.IsCutoffEnabled() ? ins.GetCutoff() : 0x7F;
			change |= (chn.nCutOff < 0x7F || chn.dwFlags[CHN_FILTER]);
		}
		if(updateResonance)
		{
			chn.nResonance = ins.IsResonanceEnabled() ? ins.GetResonance() : 0;
			change |= (chn.nResonance > 0 || chn.dwFlags[CHN_FILTER]);
		}
		// If filter envelope is active, the filter will be updated in the next player tick anyway.
		if(change && (!ins.PitchEnv.dwFlags[ENV_FILTER] || !IsEnvelopeProcessed(chn, ENV_PITCH)))
			SetupChannelFilter(chn, false);
	}
}


// Simple 2-poles resonant filter. Returns computed cutoff in range [0, 254] or -1 if filter is not applied.
int CSoundFile::SetupChannelFilter(ModChannel &chn, bool bReset, int envModifier) const
{
	int cutoff = static_cast<int>(chn.nCutOff) + chn.nCutSwing;
	int resonance = static_cast<int>(chn.nResonance & 0x7F) + chn.nResSwing;

	Limit(cutoff, 0, 127);
	Limit(resonance, 0, 127);

	if(!m_playBehaviour[kMPTOldSwingBehaviour])
	{
		chn.nCutOff = (uint8)cutoff;
		chn.nCutSwing = 0;
		chn.nResonance = (uint8)resonance;
		chn.nResSwing = 0;
	}

	// envModifier is in [-256, 256], so cutoff is in [0, 127 * 2] after this calculation.
	const int computedCutoff = cutoff * (envModifier + 256) / 256;

	// Filtering is only ever done in IT if either cutoff is not full or if resonance is set.
	if(m_playBehaviour[kITFilterBehaviour] && resonance == 0 && computedCutoff >= 254)
	{
		if(chn.triggerNote)
		{
			// Z7F next to a note disables the filter, however in other cases this should not happen.
			// Test cases: filter-reset.it, filter-reset-carry.it, filter-reset-envelope.it, filter-nna.it, FilterResetPatDelay.it, FilterPortaSmpChange.it, FilterPortaSmpChange-InsMode.it
			chn.dwFlags.reset(CHN_FILTER);
		}
		return -1;
	}

	chn.dwFlags.set(CHN_FILTER);

	// 2 * damping factor
	const float dmpfac = std::pow(10.0f, static_cast<float>(-resonance) * ((24.0f / 128.0f) / 20.0f));
	const float fc = CutOffToFrequency(cutoff, envModifier) * (2.0f * mpt::numbers::pi_v<float>);
	float d, e;
	if(m_playBehaviour[kITFilterBehaviour] && !m_SongFlags[SONG_EXFILTERRANGE])
	{
		const float r = static_cast<float>(m_MixerSettings.gdwMixingFreq) / fc;

		d = dmpfac * r + dmpfac - 1.0f;
		e = r * r;
	} else
	{
		const float r = fc / static_cast<float>(m_MixerSettings.gdwMixingFreq);

		d = (1.0f - 2.0f * dmpfac) * r;
		LimitMax(d, 2.0f);
		d = (2.0f * dmpfac - d) / r;
		e = 1.0f / (r * r);
	}

	float fg = 1.0f / (1.0f + d + e);
	float fb0 = (d + e + e) / (1 + d + e);
	float fb1 = -e / (1.0f + d + e);

#if defined(MPT_INTMIXER)
#define MPT_FILTER_CONVERT(x) mpt::saturate_round<mixsample_t>((x) * (1 << MIXING_FILTER_PRECISION))
#else
#define MPT_FILTER_CONVERT(x) (x)
#endif

	switch(chn.nFilterMode)
	{
	case FilterMode::HighPass:
		chn.nFilter_A0 = MPT_FILTER_CONVERT(1.0f - fg);
		chn.nFilter_B0 = MPT_FILTER_CONVERT(fb0);
		chn.nFilter_B1 = MPT_FILTER_CONVERT(fb1);
#ifdef MPT_INTMIXER
		chn.nFilter_HP = -1;
#else
		chn.nFilter_HP = 1.0f;
#endif // MPT_INTMIXER
		break;

	default:
		chn.nFilter_A0 = MPT_FILTER_CONVERT(fg);
		chn.nFilter_B0 = MPT_FILTER_CONVERT(fb0);
		chn.nFilter_B1 = MPT_FILTER_CONVERT(fb1);
#ifdef MPT_INTMIXER
		if(chn.nFilter_A0 == 0)
			chn.nFilter_A0 = 1;	// Prevent silence at low filter cutoff and very high sampling rate
		chn.nFilter_HP = 0;
#else
		chn.nFilter_HP = 0;
#endif // MPT_INTMIXER
		break;
	}
#undef MPT_FILTER_CONVERT

	if (bReset)
	{
		chn.nFilter_Y[0][0] = chn.nFilter_Y[0][1] = 0;
		chn.nFilter_Y[1][0] = chn.nFilter_Y[1][1] = 0;
	}

	return computedCutoff;
}


OPENMPT_NAMESPACE_END
