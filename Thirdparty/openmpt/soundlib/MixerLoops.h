/*
 * MixerLoops.h
 * ------------
 * Purpose: Utility inner loops for mixer-related functionality.
 * Notes  : none.
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#pragma once

#include "openmpt/all/BuildSettings.hpp"

#include "Mixer.h"

OPENMPT_NAMESPACE_BEGIN

struct ModChannel;

void StereoMixToFloat(const int32 *pSrc, float *pOut1, float *pOut2, uint32 nCount, const float _i2fc);
void FloatToStereoMix(const float *pIn1, const float *pIn2, int32 *pOut, uint32 uint32, const float _f2ic);

void InitMixBuffer(mixsample_t *pBuffer, uint32 nSamples);
void InterleaveFrontRear(mixsample_t *pFrontBuf, mixsample_t *pRearBuf, uint32 nFrames);
void MonoFromStereo(mixsample_t *pMixBuf, uint32 nSamples);

#ifndef MPT_INTMIXER
void InterleaveStereo(const mixsample_t * MPT_RESTRICT inputL, const mixsample_t * MPT_RESTRICT inputR, mixsample_t * MPT_RESTRICT output, size_t numSamples);
void DeinterleaveStereo(const mixsample_t * MPT_RESTRICT input, mixsample_t * MPT_RESTRICT outputL, mixsample_t * MPT_RESTRICT outputR, size_t numSamples);
#endif

void EndChannelOfs(ModChannel &chn, mixsample_t *pBuffer, uint32 nSamples);
void StereoFill(mixsample_t *pBuffer, uint32 nSamples, mixsample_t &rofs, mixsample_t &lofs);

OPENMPT_NAMESPACE_END
