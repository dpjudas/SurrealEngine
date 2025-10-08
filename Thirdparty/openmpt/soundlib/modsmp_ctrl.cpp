/*
 * modsmp_ctrl.cpp
 * ---------------
 * Purpose: Basic sample editing code.
 * Notes  : This is a legacy namespace. Some of this stuff is not required in libopenmpt (but stuff in soundlib/ still depends on it). The rest could be merged into struct ModSample.
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"
#include "modsmp_ctrl.h"
#include "AudioCriticalSection.h"
#include "Sndfile.h"

OPENMPT_NAMESPACE_BEGIN

namespace ctrlSmp
{

template <class T>
static void ReverseSampleImpl(T *pStart, const SmpLength length)
{
	for(SmpLength i = 0; i < length / 2; i++)
	{
		std::swap(pStart[i], pStart[length - 1 - i]);
	}
}

// Reverse sample data
bool ReverseSample(ModSample &smp, SmpLength start, SmpLength end, CSoundFile &sndFile)
{
	if(!smp.HasSampleData()) return false;
	if(end == 0 || start > smp.nLength || end > smp.nLength)
	{
		start = 0;
		end   = smp.nLength;
	}

	if(end - start < 2) return false;

	static_assert(MaxSamplingPointSize <= 4);
	if(smp.GetBytesPerSample() == 4)  // 16 bit stereo
		ReverseSampleImpl(static_cast<int32 *>(smp.samplev()) + start, end - start);
	else if(smp.GetBytesPerSample() == 2)  // 16 bit mono / 8 bit stereo
		ReverseSampleImpl(static_cast<int16 *>(smp.samplev()) + start, end - start);
	else if(smp.GetBytesPerSample() == 1)  // 8 bit mono
		ReverseSampleImpl(static_cast<int8 *>(smp.samplev()) + start, end - start);
	else
		return false;

	smp.PrecomputeLoops(sndFile, false);
	return true;
}


template <class T>
static void InvertSampleImpl(T *pStart, const SmpLength length)
{
	for(SmpLength i = 0; i < length; i++)
	{
		pStart[i] = ~pStart[i];
	}
}

// Invert sample data (flip by 180 degrees)
bool InvertSample(ModSample &smp, SmpLength start, SmpLength end, CSoundFile &sndFile)
{
	if(!smp.HasSampleData()) return false;
	if(end == 0 || start > smp.nLength || end > smp.nLength)
	{
		start = 0;
		end = smp.nLength;
	}
	start *= smp.GetNumChannels();
	end *= smp.GetNumChannels();
	if(smp.GetElementarySampleSize() == 2)
		InvertSampleImpl(smp.sample16() + start, end - start);
	else if(smp.GetElementarySampleSize() == 1)
		InvertSampleImpl(smp.sample8() + start, end - start);
	else
		return false;

	smp.PrecomputeLoops(sndFile, false);
	return true;
}


template <class T>
static void XFadeSampleImpl(const T *srcIn, const T *srcOut, T *output, const SmpLength fadeLength, double e)
{
	const double length = 1.0 / static_cast<double>(fadeLength);
	for(SmpLength i = 0; i < fadeLength; i++, srcIn++, srcOut++, output++)
	{
		double fact1 = std::pow(i * length, e);
		double fact2 = std::pow((fadeLength - i) * length, e);
		int32 val = static_cast<int32>(
			static_cast<double>(*srcIn) * fact1 +
			static_cast<double>(*srcOut) * fact2);
		*output = mpt::saturate_cast<T>(val);
	}
}

// X-Fade sample data to create smooth loop transitions
bool XFadeSample(ModSample &smp, SmpLength fadeLength, int fadeLaw, bool afterloopFade, bool useSustainLoop, CSoundFile &sndFile)
{
	if(!smp.HasSampleData()) return false;
	const auto [loopStart, loopEnd] = useSustainLoop ? smp.GetSustainLoop() : smp.GetLoop();
	
	if(loopEnd <= loopStart || loopEnd > smp.nLength) return false;
	if(loopStart < fadeLength) return false;

	const SmpLength start = (loopStart - fadeLength) * smp.GetNumChannels();
	const SmpLength end = (loopEnd - fadeLength) * smp.GetNumChannels();
	const SmpLength afterloopStart = loopStart * smp.GetNumChannels();
	const SmpLength afterloopEnd = loopEnd * smp.GetNumChannels();
	const SmpLength afterLoopLength = std::min(smp.nLength - loopEnd, fadeLength) * smp.GetNumChannels();
	fadeLength *= smp.GetNumChannels();

	// e=0.5: constant power crossfade (for uncorrelated samples), e=1.0: constant volume crossfade (for perfectly correlated samples)
	const double e = 1.0 - fadeLaw / 200000.0;

	if(smp.GetElementarySampleSize() == 2)
	{
		XFadeSampleImpl(smp.sample16() + start, smp.sample16() + end, smp.sample16() + end, fadeLength, e);
		if(afterloopFade) XFadeSampleImpl(smp.sample16() + afterloopEnd, smp.sample16() + afterloopStart, smp.sample16() + afterloopEnd, afterLoopLength, e);
	} else if(smp.GetElementarySampleSize() == 1)
	{
		XFadeSampleImpl(smp.sample8() + start, smp.sample8() + end, smp.sample8() + end, fadeLength, e);
		if(afterloopFade) XFadeSampleImpl(smp.sample8() + afterloopEnd, smp.sample8() + afterloopStart, smp.sample8() + afterloopEnd, afterLoopLength, e);
	} else
		return false;

	smp.PrecomputeLoops(sndFile, true);
	return true;
}


template <class T>
static void ConvertStereoToMonoMixImpl(T *pDest, const SmpLength length)
{
	const T *pEnd = pDest + length;
	for(T *pSource = pDest; pDest != pEnd; pDest++, pSource += 2)
	{
		*pDest = static_cast<T>(mpt::rshift_signed(pSource[0] + pSource[1] + 1, 1));
	}
}


template <class T>
static void ConvertStereoToMonoOneChannelImpl(T *pDest, const T *pSource, const SmpLength length)
{
	for(const T *pEnd = pDest + length; pDest != pEnd; pDest++, pSource += 2)
	{
		*pDest = *pSource;
	}
}


// Convert a multichannel sample to mono (currently only implemented for stereo)
bool ConvertToMono(ModSample &smp, CSoundFile &sndFile, StereoToMonoMode conversionMode)
{
	if(!smp.HasSampleData() || smp.GetNumChannels() != 2) return false;

	// Note: Sample is overwritten in-place! Unused data is not deallocated!
	if(conversionMode == mixChannels)
	{
		if(smp.GetElementarySampleSize() == 2)
			ConvertStereoToMonoMixImpl(smp.sample16(), smp.nLength);
		else if(smp.GetElementarySampleSize() == 1)
			ConvertStereoToMonoMixImpl(smp.sample8(), smp.nLength);
		else
			return false;
	} else
	{
		if(conversionMode == splitSample)
		{
			conversionMode = onlyLeft;
		}
		if(smp.GetElementarySampleSize() == 2)
			ConvertStereoToMonoOneChannelImpl(smp.sample16(), smp.sample16() + (conversionMode == onlyLeft ? 0 : 1), smp.nLength);
		else if(smp.GetElementarySampleSize() == 1)
			ConvertStereoToMonoOneChannelImpl(smp.sample8(), smp.sample8() + (conversionMode == onlyLeft ? 0 : 1), smp.nLength);
		else
			return false;
	}

	CriticalSection cs;
	smp.uFlags.reset(CHN_STEREO);
	for(auto &chn : sndFile.m_PlayState.Chn)
	{
		if(chn.pModSample == &smp)
		{
			chn.dwFlags.reset(CHN_STEREO);
		}
	}

	smp.PrecomputeLoops(sndFile, false);
	return true;
}


template <class T>
static void SplitStereoImpl(void *destL, void *destR, const T *source, SmpLength length)
{
	T *l = static_cast<T *>(destL), *r = static_cast<T*>(destR);
	while(length--)
	{
		*(l++) = source[0];
		*(r++) = source[1];
		source += 2;
	}
}


// Converts a stereo sample into two mono samples. Source sample will not be deleted.
bool SplitStereo(const ModSample &source, ModSample &left, ModSample &right, CSoundFile &sndFile)
{
	if(!source.HasSampleData() || source.GetNumChannels() != 2 || &left == &right)
		return false;
	const bool sourceIsLeft = &left == &source, sourceIsRight = &right == &source;
	if(left.HasSampleData() && !sourceIsLeft)
		return false;
	if(right.HasSampleData() && !sourceIsRight)
		return false;

	void *leftData  = sourceIsLeft ? left.samplev() : ModSample::AllocateSample(source.nLength, source.GetElementarySampleSize());
	void *rightData = sourceIsRight ? right.samplev() : ModSample::AllocateSample(source.nLength, source.GetElementarySampleSize());
	if(!leftData || !rightData)
	{
		if(!sourceIsLeft)
			ModSample::FreeSample(leftData);
		if(!sourceIsRight)
			ModSample::FreeSample(rightData);
		return false;
	}

	if(source.GetElementarySampleSize() == 2)
		SplitStereoImpl(leftData, rightData, source.sample16(), source.nLength);
	else if(source.GetElementarySampleSize() == 1)
		SplitStereoImpl(leftData, rightData, source.sample8(), source.nLength);
	else
		MPT_ASSERT_NOTREACHED();

	CriticalSection cs;
	left = source;
	left.uFlags.reset(CHN_STEREO);
	left.pData.pSample = leftData;

	right = source;
	right.uFlags.reset(CHN_STEREO);
	right.pData.pSample = rightData;

	for(auto &chn : sndFile.m_PlayState.Chn)
	{
		if(chn.pModSample == &left || chn.pModSample == &right)
			chn.dwFlags.reset(CHN_STEREO);
	}

	left.PrecomputeLoops(sndFile, false);
	right.PrecomputeLoops(sndFile, false);
	return true;
}


template <class T>
static void ConvertMonoToStereoImpl(const T *MPT_RESTRICT src, T *MPT_RESTRICT dst, SmpLength length)
{
	while(length--)
	{
		dst[0] = *src;
		dst[1] = *src;
		dst += 2;
		src++;
	}
}


// Convert a multichannel sample to mono (currently only implemented for stereo)
bool ConvertToStereo(ModSample &smp, CSoundFile &sndFile)
{
	if(!smp.HasSampleData() || smp.GetNumChannels() != 1) return false;

	void *newSample = ModSample::AllocateSample(smp.nLength, smp.GetBytesPerSample() * 2);
	if(newSample == nullptr)
	{
		return 0;
	}

	if(smp.GetElementarySampleSize() == 2)
		ConvertMonoToStereoImpl(smp.sample16(), (int16 *)newSample, smp.nLength);
	else if(smp.GetElementarySampleSize() == 1)
		ConvertMonoToStereoImpl(smp.sample8(), (int8 *)newSample, smp.nLength);
	else
		return false;

	CriticalSection cs;
	smp.uFlags.set(CHN_STEREO);
	smp.ReplaceWaveform(newSample, smp.nLength, sndFile);

	smp.PrecomputeLoops(sndFile, false);
	return true;
}


} // namespace ctrlSmp



namespace ctrlChn
{

void ReplaceSample( CSoundFile &sndFile,
					const ModSample &sample,
					const void * const pNewSample,
					const SmpLength newLength,
					FlagSet<ChannelFlags> setFlags,
					FlagSet<ChannelFlags> resetFlags)
{
	const bool periodIsFreq = sndFile.PeriodsAreFrequencies();
	for(auto &chn : sndFile.m_PlayState.Chn)
	{
		if(chn.pModSample == &sample)
		{
			if(chn.pCurrentSample != nullptr)
				chn.pCurrentSample = pNewSample;
			if(chn.position.GetUInt() > newLength)
				chn.position.Set(0);
			if(chn.nLength > 0)
				LimitMax(chn.nLength, newLength);
			if(chn.InSustainLoop())
			{
				chn.nLoopStart = sample.nSustainStart;
				chn.nLoopEnd = sample.nSustainEnd;
			} else
			{
				chn.nLoopStart = sample.nLoopStart;
				chn.nLoopEnd = sample.nLoopEnd;
			}
			chn.dwFlags.set(setFlags);
			chn.dwFlags.reset(resetFlags);
			if(chn.nC5Speed && sample.nC5Speed && !sndFile.UseFinetuneAndTranspose())
			{
				if(periodIsFreq)
					chn.nPeriod = Util::muldivr_unsigned(chn.nPeriod, sample.nC5Speed, chn.nC5Speed);
				else
					chn.nPeriod = Util::muldivr_unsigned(chn.nPeriod, chn.nC5Speed, sample.nC5Speed);
			}
			chn.nC5Speed = sample.nC5Speed;
		}
	}
}

} // namespace ctrlChn


OPENMPT_NAMESPACE_END
