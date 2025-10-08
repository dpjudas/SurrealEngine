/*
 * WindowedFIR.cpp
 * ---------------
 * Purpose: FIR resampling code
 * Notes  : Original code from modplug-xmms
 * Authors: OpenMPT Devs
 *          ModPlug-XMMS Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"
#include "WindowedFIR.h"
#include "mpt/base/numbers.hpp"
#include <cmath>

OPENMPT_NAMESPACE_BEGIN

double CWindowedFIR::coef(int cnr, double ofs, double cut, int width, int type)
{
	const double epsilon = 1e-8;
	const double widthM1 = width - 1;
	const double widthM1Half = 0.5 * widthM1;
	const double posU = (cnr - ofs);
	const double idl = (2.0 * mpt::numbers::pi) / widthM1;
	double pos = posU - widthM1Half;
	double wc, si;
	if(std::abs(pos) < epsilon)
	{
		wc = 1.0;
		si = cut;
	} else
	{
		switch(type)
		{
		case WFIR_HANN:
			wc = 0.50 - 0.50 * std::cos(idl * posU);
			break;
		case WFIR_HAMMING:
			wc = 0.54 - 0.46 * std::cos(idl * posU);
			break;
		case WFIR_BLACKMANEXACT:
			wc = 0.42 - 0.50 * std::cos(idl * posU) + 0.08 * std::cos(2.0 * idl * posU);
			break;
		case WFIR_BLACKMAN3T61:
			wc = 0.44959 - 0.49364 * std::cos(idl * posU) + 0.05677 * std::cos(2.0 * idl * posU);
			break;
		case WFIR_BLACKMAN3T67:
			wc = 0.42323 - 0.49755 * std::cos(idl * posU) + 0.07922 * std::cos(2.0 * idl * posU);
			break;
		case WFIR_BLACKMAN4T92: // blackman harris
			wc = 0.35875 - 0.48829 * std::cos(idl * posU) + 0.14128 * std::cos(2.0 * idl * posU) - 0.01168 * std::cos(3.0 * idl * posU);
			break;
		case WFIR_BLACKMAN4T74:
			wc = 0.40217 - 0.49703 * std::cos(idl * posU) + 0.09392 * std::cos(2.0 * idl * posU) - 0.00183 * std::cos(3.0 * idl * posU);
			break;
		case WFIR_KAISER4T: // kaiser-bessel, alpha~7.5
			wc = 0.40243 - 0.49804 * std::cos(idl * posU) + 0.09831 * std::cos(2.0 * idl * posU) - 0.00122 * std::cos(3.0 * idl * posU);
			break;
		default:
			wc = 1.0;
			break;
		}
		pos *= mpt::numbers::pi;
		si = std::sin(cut * pos) / pos;
	}
	return (wc * si);
}

void CWindowedFIR::InitTable(double WFIRCutoff, uint8 WFIRType)
{
	const double pcllen = (double)(1 << WFIR_FRACBITS);  // number of precalculated lines for 0..1 (-1..0)
	const double norm = 1.0 / (2.0 * pcllen);
	const double cut = WFIRCutoff;
	for(int pcl = 0; pcl < WFIR_LUTLEN; pcl++)
	{
		double gain = 0.0, coefs[WFIR_WIDTH];
		const double ofs = (pcl - pcllen) * norm;
		const int idx = pcl << WFIR_LOG2WIDTH;
		for(int cc = 0; cc < WFIR_WIDTH; cc++)
		{
			gain += (coefs[cc] = coef(cc, ofs, cut, WFIR_WIDTH, WFIRType));
		}
		gain = 1.0 / gain;
		for(int cc = 0; cc < WFIR_WIDTH; cc++)
		{
#ifdef MPT_INTMIXER
			double coef = std::floor(0.5 + WFIR_QUANTSCALE * coefs[cc] * gain);
			lut[idx + cc] = (signed short)((coef < -WFIR_QUANTSCALE) ? -WFIR_QUANTSCALE : ((coef > WFIR_QUANTSCALE) ? WFIR_QUANTSCALE : coef));
#else
			double coef = coefs[cc] * gain;
			lut[idx + cc] = (float)coef;
#endif // MPT_INTMIXER
		}
	}
}


OPENMPT_NAMESPACE_END
