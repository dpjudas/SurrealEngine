/*
 * WindowedFIR.h
 * -------------
 * Purpose: FIR resampling code
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 *          ModPlug-XMMS Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#pragma once

#include "openmpt/all/BuildSettings.hpp"

#include "Mixer.h"

OPENMPT_NAMESPACE_BEGIN

/*
  ------------------------------------------------------------------------------------------------
   fir interpolation doc,
	(derived from "an engineer's guide to fir digital filters", n.j. loy)

	calculate coefficients for ideal lowpass filter (with cutoff = fc in 0..1 (mapped to 0..nyquist))
	  c[-N..N] = (i==0) ? fc : sin(fc*pi*i)/(pi*i)

	then apply selected window to coefficients
	  c[-N..N] *= w(0..N)
	with n in 2*N and w(n) being a window function (see loy)

	then calculate gain and scale filter coefs to have unity gain.
  ------------------------------------------------------------------------------------------------
*/

#ifdef MPT_INTMIXER
// quantizer scale of window coefs - only required for integer mixing
inline constexpr int WFIR_QUANTBITS = 15;
inline constexpr double WFIR_QUANTSCALE = 1 << WFIR_QUANTBITS;
inline constexpr int WFIR_8SHIFT = (WFIR_QUANTBITS - 8);
inline constexpr int WFIR_16BITSHIFT = (WFIR_QUANTBITS);
using WFIR_TYPE = int16;
#else
using WFIR_TYPE = mixsample_t;
#endif // INTMIXER
// log2(number)-1 of precalculated taps range is [4..12]
inline constexpr int WFIR_FRACBITS = 12;  //10
inline constexpr int WFIR_LUTLEN = ((1 << (WFIR_FRACBITS + 1)) + 1);
// number of samples in window
inline constexpr int WFIR_LOG2WIDTH = 3;
inline constexpr int WFIR_WIDTH = (1 << WFIR_LOG2WIDTH);
// cutoff (1.0 == pi/2)
// wfir type
enum WFIRType
{
	WFIR_HANN          = 0,  // Hann
	WFIR_HAMMING       = 1,  // Hamming
	WFIR_BLACKMANEXACT = 2,  // Blackman Exact
	WFIR_BLACKMAN3T61  = 3,  // Blackman 3-Tap 61
	WFIR_BLACKMAN3T67  = 4,  // Blackman 3-Tap 67
	WFIR_BLACKMAN4T92  = 5,  // Blackman-Harris
	WFIR_BLACKMAN4T74  = 6,  // Blackman 4-Tap 74
	WFIR_KAISER4T      = 7,  // Kaiser a=7.5
};


// fir interpolation
inline constexpr int WFIR_FRACSHIFT = (16 - (WFIR_FRACBITS + 1 + WFIR_LOG2WIDTH));
inline constexpr int WFIR_FRACMASK = ((((1 << (17 - WFIR_FRACSHIFT)) - 1) & ~(WFIR_WIDTH - 1)));
inline constexpr int WFIR_FRACHALVE = (1 << (16 - (WFIR_FRACBITS + 2)));

class CWindowedFIR
{
private:
	double coef(int,double,double,int,int);

public:
	void InitTable(double WFIRCutoff, uint8 WFIRType);
	WFIR_TYPE lut[WFIR_LUTLEN * WFIR_WIDTH];
};

OPENMPT_NAMESPACE_END
