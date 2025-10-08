/*
 * MixerLoops.cpp
 * --------------
 * Purpose: Utility inner loops for mixer-related functionality.
 * Notes  : This file contains performance-critical loops.
 * Authors: Olivier Lapicque
 *          OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"
#include "MixerLoops.h"
#include "Snd_defs.h"
#include "ModChannel.h"


OPENMPT_NAMESPACE_BEGIN



void FloatToStereoMix(const float *pIn1, const float *pIn2, int32 *pOut, uint32 nCount, const float _f2ic)
{
	for(uint32 i=0; i<nCount; ++i)
	{
		*pOut++ = static_cast<int>(*pIn1++ * _f2ic);
		*pOut++ = static_cast<int>(*pIn2++ * _f2ic);
	}
}


void StereoMixToFloat(const int32 *pSrc, float *pOut1, float *pOut2, uint32 nCount, const float _i2fc)
{
	for(uint32 i=0; i<nCount; ++i)
	{
		*pOut1++ = static_cast<float>(*pSrc++) * _i2fc;
		*pOut2++ = static_cast<float>(*pSrc++) * _i2fc;
	}
}



void InitMixBuffer(mixsample_t *pBuffer, uint32 nSamples)
{
	std::memset(pBuffer, 0, nSamples * sizeof(mixsample_t));
}



void InterleaveFrontRear(mixsample_t *pFrontBuf, mixsample_t *pRearBuf, uint32 nFrames)
{
	// copy backwards as we are writing back into FrontBuf
	for(int i=nFrames-1; i>=0; i--)
	{
		pFrontBuf[i*4+3] = pRearBuf[i*2+1];
		pFrontBuf[i*4+2] = pRearBuf[i*2+0];
		pFrontBuf[i*4+1] = pFrontBuf[i*2+1];
		pFrontBuf[i*4+0] = pFrontBuf[i*2+0];
	}
}



void MonoFromStereo(mixsample_t *pMixBuf, uint32 nSamples)
{
	for(uint32 i=0; i<nSamples; ++i)
	{
		pMixBuf[i] = (pMixBuf[i*2] + pMixBuf[i*2+1]) / 2;
	}
}



#define OFSDECAYSHIFT	8
#define OFSDECAYMASK	0xFF
#define OFSTHRESHOLD	static_cast<mixsample_t>(1.0 / (1 << 20))	// Decay threshold for floating point mixer


void StereoFill(mixsample_t *pBuffer, uint32 nSamples, mixsample_t &rofs, mixsample_t &lofs)
{
	if((!rofs) && (!lofs))
	{
		InitMixBuffer(pBuffer, nSamples*2);
		return;
	}
	for(uint32 i=0; i<nSamples; i++)
	{
#ifdef MPT_INTMIXER
		// Equivalent to int x_r = (rofs + (rofs > 0 ? 255 : -255)) / 256;
		const mixsample_t x_r = mpt::rshift_signed(rofs + (mpt::rshift_signed(-rofs, sizeof(mixsample_t) * 8 - 1) & OFSDECAYMASK), OFSDECAYSHIFT);
		const mixsample_t x_l = mpt::rshift_signed(lofs + (mpt::rshift_signed(-lofs, sizeof(mixsample_t) * 8 - 1) & OFSDECAYMASK), OFSDECAYSHIFT);
#else
		const mixsample_t x_r = rofs * (1.0f / (1 << OFSDECAYSHIFT));
		const mixsample_t x_l = lofs * (1.0f / (1 << OFSDECAYSHIFT));
#endif
		rofs -= x_r;
		lofs -= x_l;
		pBuffer[i*2] = rofs;
		pBuffer[i*2+1] = lofs;
	}

#ifndef MPT_INTMIXER
	if(fabs(rofs) < OFSTHRESHOLD) rofs = 0;
	if(fabs(lofs) < OFSTHRESHOLD) lofs = 0;
#endif
}


void EndChannelOfs(ModChannel &chn, mixsample_t *pBuffer, uint32 nSamples)
{

	mixsample_t rofs = chn.nROfs;
	mixsample_t lofs = chn.nLOfs;

	if((!rofs) && (!lofs))
	{
		return;
	}
	for(uint32 i=0; i<nSamples; i++)
	{
#ifdef MPT_INTMIXER
		const mixsample_t x_r = mpt::rshift_signed(rofs + (mpt::rshift_signed(-rofs, sizeof(mixsample_t) * 8 - 1) & OFSDECAYMASK), OFSDECAYSHIFT);
		const mixsample_t x_l = mpt::rshift_signed(lofs + (mpt::rshift_signed(-lofs, sizeof(mixsample_t) * 8 - 1) & OFSDECAYMASK), OFSDECAYSHIFT);
#else
		const mixsample_t x_r = rofs * (1.0f / (1 << OFSDECAYSHIFT));
		const mixsample_t x_l = lofs * (1.0f / (1 << OFSDECAYSHIFT));
#endif
		rofs -= x_r;
		lofs -= x_l;
		pBuffer[i*2] += rofs;
		pBuffer[i*2+1] += lofs;
	}
#ifndef MPT_INTMIXER
	if(std::abs(rofs) < OFSTHRESHOLD) rofs = 0;
	if(std::abs(lofs) < OFSTHRESHOLD) lofs = 0;
#endif

	chn.nROfs = rofs;
	chn.nLOfs = lofs;
}



#ifndef MPT_INTMIXER

void InterleaveStereo(const mixsample_t * MPT_RESTRICT inputL, const mixsample_t * MPT_RESTRICT inputR, mixsample_t * MPT_RESTRICT output, size_t numSamples)
{
	while(numSamples--)
	{
		*(output++) = *(inputL++);
		*(output++) = *(inputR++);
	}
}


void DeinterleaveStereo(const mixsample_t * MPT_RESTRICT input, mixsample_t * MPT_RESTRICT outputL, mixsample_t * MPT_RESTRICT outputR, size_t numSamples)
{
	while(numSamples--)
	{
		*(outputL++) = *(input++);
		*(outputR++) = *(input++);
	}
}

#endif



OPENMPT_NAMESPACE_END
