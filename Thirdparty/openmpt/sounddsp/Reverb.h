/*
 * Reverb.h
 * --------
 * Purpose: Mixing code for reverb.
 * Notes  : Ugh... This should really be removed at some point.
 * Authors: Olivier Lapicque
 *          OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */

#pragma once

#include "openmpt/all/BuildSettings.hpp"

#ifndef NO_REVERB

#include "../soundlib/Mixer.h"	// For MIXBUFFERSIZE

OPENMPT_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////
// Reverberation

/////////////////////////////////////////////////////////////////////////////
//
// SW Reverb structures
//

// Length-1 (in samples) of the reflections delay buffer: 32K, 371ms@22kHz
#define SNDMIX_REFLECTIONS_DELAY_MASK	0x1fff
#define SNDMIX_PREDIFFUSION_DELAY_MASK	0x7f	// 128 samples
#define SNDMIX_REVERB_DELAY_MASK		0xfff	// 4K samples (92ms @ 44kHz)

union LR16
{
	struct { int16 l, r; } c;
	int32 lr;
};

struct SWRvbReflection
{
	uint32 Delay, DelayDest;
	LR16   Gains[2];	// g_ll, g_rl, g_lr, g_rr
};

struct SWRvbRefDelay
{
	uint32 nDelayPos, nPreDifPos, nRefOutPos;
	int32  lMasterGain;			// reflections linear master gain
	LR16   nCoeffs;				// room low-pass coefficients
	LR16   History;				// room low-pass history
	LR16   nPreDifCoeffs;		// prediffusion coefficients
	LR16   ReflectionsGain;		// master reflections gain
	SWRvbReflection Reflections[8];	// Up to 8 SW Reflections
	LR16   RefDelayBuffer[SNDMIX_REFLECTIONS_DELAY_MASK + 1]; // reflections delay buffer
	LR16   PreDifBuffer[SNDMIX_PREDIFFUSION_DELAY_MASK + 1]; // pre-diffusion
	LR16   RefOut[SNDMIX_REVERB_DELAY_MASK + 1]; // stereo output of reflections
};

struct SNDMIX_REVERB_PROPERTIES;


// Late reverberation
// Tank diffusers lengths
#define RVBDIF1L_LEN		(149*2)	// 6.8ms
#define RVBDIF1R_LEN		(223*2)	// 10.1ms
#define RVBDIF2L_LEN		(421*2)	// 19.1ms
#define RVBDIF2R_LEN		(647*2)	// 29.3ms
// Tank delay lines lengths
#define RVBDLY1L_LEN		(683*2)	// 30.9ms
#define RVBDLY1R_LEN	    (811*2) // 36.7ms
#define RVBDLY2L_LEN		(773*2)	// 35.1ms
#define RVBDLY2R_LEN	    (1013*2) // 45.9ms
// Tank delay lines mask
#define RVBDLY_MASK			2047

// Min/Max reflections delay
#define RVBMINREFDELAY		96		// 96 samples
#define RVBMAXREFDELAY		7500	// 7500 samples
// Min/Max reverb delay
#define RVBMINRVBDELAY		128		// 256 samples (11.6ms @ 22kHz)
#define RVBMAXRVBDELAY		3800	// 1900 samples (86ms @ 24kHz)

struct SWLateReverb
{
	uint32 nReverbDelay;		// Reverb delay (in samples)
	uint32 nDelayPos;			// Delay line position
	LR16   nDifCoeffs[2];		// Reverb diffusion
	LR16   nDecayDC[2];			// Reverb DC decay
	LR16   nDecayLP[2];			// Reverb HF decay
	LR16   LPHistory[2];		// Low-pass history
	LR16   Dif2InGains[2];		// 2nd diffuser input gains
	LR16   RvbOutGains[2];		// 4x2 Reverb output gains
	int32  lMasterGain;			// late reverb master gain
	int32  lDummyAlign;
	// Tank Delay lines
	LR16   Diffusion1[RVBDLY_MASK + 1];	// {dif1_l, dif1_r}
	LR16   Diffusion2[RVBDLY_MASK + 1];	// {dif2_l, dif2_r}
	LR16   Delay1[RVBDLY_MASK + 1];		// {dly1_l, dly1_r}
	LR16   Delay2[RVBDLY_MASK + 1];		// {dly2_l, dly2_r}
};

#define ENVIRONMENT_NUMREFLECTIONS		8

struct EnvironmentReflection
{
	int16  GainLL, GainRR, GainLR, GainRL;	// +/- 32K scale
	uint32 Delay;							// In samples
};

struct EnvironmentReverb
{
	int32  ReverbLevel;			// Late reverb gain (mB)
	int32  ReflectionsLevel;	// Master reflections gain (mB)
	int32  RoomHF;				// Room gain HF (mB)
	uint32 ReverbDecay;			// Reverb tank decay (0-7fff scale)
	int32  PreDiffusion;		// Reverb pre-diffusion amount (+/- 32K scale)
	int32  TankDiffusion;		// Reverb tank diffusion (+/- 32K scale)
	uint32 ReverbDelay;			// Reverb delay (in samples)
	float  flReverbDamping;		// HF tank gain [0.0, 1.0]
	int32  ReverbDecaySamples;	// Reverb decay time (in samples)
	EnvironmentReflection Reflections[ENVIRONMENT_NUMREFLECTIONS];
};


class CReverbSettings
{
public:
	uint32 m_nReverbDepth = 8; // 50%
	uint32 m_nReverbType = 0;
};


class CReverb
{
public:
	CReverbSettings m_Settings;

private:
	const SNDMIX_REVERB_PROPERTIES *m_currentPreset = nullptr;

	bool gnReverbSend = false;

	uint32 gnReverbSamples = 0;
	uint32 gnReverbDecaySamples = 0;

	// Internal reverb state
	bool g_bLastInPresent = 0;
	bool g_bLastOutPresent = 0;
	int g_nLastRvbIn_xl = 0;
	int g_nLastRvbIn_xr = 0;
	int g_nLastRvbIn_yl = 0;
	int g_nLastRvbIn_yr = 0;
	int g_nLastRvbOut_xl = 0;
	int g_nLastRvbOut_xr = 0;
	int32 gnDCRRvb_Y1[2] = { 0, 0 };
	int32 gnDCRRvb_X1[2] = { 0, 0 };

	// Reverb mix buffers
	SWRvbRefDelay g_RefDelay;
	SWLateReverb g_LateReverb;

public:
	CReverb();
public:
	void Initialize(bool bReset, MixSampleInt &gnRvbROfsVol, MixSampleInt &gnRvbLOfsVol, uint32 MixingFreq);

	// can be called multiple times or never (if no data is sent to reverb)
	void TouchReverbSendBuffer(MixSampleInt *MixReverbBuffer, MixSampleInt &gnRvbROfsVol, MixSampleInt &gnRvbLOfsVol, uint32 nSamples);

	// call once after all data has been sent.
	void Process(MixSampleInt *MixSoundBuffer, MixSampleInt *MixReverbBuffer, MixSampleInt &gnRvbROfsVol, MixSampleInt &gnRvbLOfsVol, uint32 nSamples);

private:
	void Shutdown(MixSampleInt &gnRvbROfsVol, MixSampleInt &gnRvbLOfsVol);
	// Pre/Post resampling and filtering
	uint32 ReverbProcessPreFiltering1x(int32 *pWet, uint32 nSamples);
	uint32 ReverbProcessPreFiltering2x(int32 *pWet, uint32 nSamples);
	void ReverbProcessPostFiltering1x(const int32 *pRvb, int32 *pDry, uint32 nSamples);
	void ReverbProcessPostFiltering2x(const int32 *pRvb, int32 *pDry, uint32 nSamples);
	void ReverbDCRemoval(int32 *pBuffer, uint32 nSamples);
	void ReverbDryMix(int32 *pDry, int32 *pWet, int lDryVol, uint32 nSamples);
	// Process pre-diffusion and pre-delay
	static void ProcessPreDelay(SWRvbRefDelay *pPreDelay, const int32 *pIn, uint32 nSamples);
	// Process reflections
	static void ProcessReflections(SWRvbRefDelay *pPreDelay, LR16 *pRefOut, int32 *pMixOut, uint32 nSamples);
	// Process Late Reverb (SW Reflections): stereo reflections output, 32-bit reverb output, SW reverb gain
	static void ProcessLateReverb(SWLateReverb *pReverb, LR16 *pRefOut, int32 *pMixOut, uint32 nSamples);
};


/////////////////////////////////////////////////////////////////////////////////
//
// I3DL2 reverb presets
//

struct SNDMIX_REVERB_PROPERTIES
{
	int32 lRoom;               // [-10000, 0]      default: -10000 mB
	int32 lRoomHF;             // [-10000, 0]      default: 0 mB
	float flDecayTime;         // [0.1, 20.0]      default: 1.0 s
	float flDecayHFRatio;      // [0.1, 2.0]       default: 0.5
	int32 lReflections;        // [-10000, 1000]   default: -10000 mB
	float flReflectionsDelay;  // [0.0, 0.3]       default: 0.02 s
	int32 lReverb;             // [-10000, 2000]   default: -10000 mB
	float flReverbDelay;       // [0.0, 0.1]       default: 0.04 s
	float flDiffusion;         // [0.0, 100.0]     default: 100.0 %
	float flDensity;           // [0.0, 100.0]     default: 100.0 %
};

enum : uint32
{
	NUM_REVERBTYPES = 29
};
mpt::ustring GetReverbPresetName(uint32 preset);
const SNDMIX_REVERB_PROPERTIES *GetReverbPreset(uint32 preset);

OPENMPT_NAMESPACE_END

#endif // NO_REVERB
