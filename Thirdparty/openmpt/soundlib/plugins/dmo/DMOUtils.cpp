/*
 * DMOUtils.cpp
 * ------------
 * Purpose: Utility functions shared by DMO plugins
 * Notes  : none
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"

#include "DMOUtils.h"

#ifndef NO_PLUGINS
#include "../../Sndfile.h"
#endif // !NO_PLUGINS

OPENMPT_NAMESPACE_BEGIN

#ifndef NO_PLUGINS

namespace DMO
{

// Computes (log2(x) + 1) * 2 ^ (shiftL - shiftR) (x = -2^31...2^31)
float logGain(float x, int32 shiftL, int32 shiftR)
{
	uint32 intSample;
	if(x <= static_cast<float>(int32_min) || x > static_cast<float>(int32_max))
		intSample = static_cast<uint32>(int32_min);
	else
		intSample = static_cast<uint32>(static_cast<int32>(x));

	const uint32 sign = intSample & 0x80000000;
	if(sign)
		intSample = (~intSample) + 1;

	// Multiply until overflow (or edge shift factor is reached)
	while(shiftL > 0 && intSample < 0x80000000)
	{
		intSample += intSample;
		shiftL--;
	}
	// Unsign clipped sample
	if(intSample >= 0x80000000)
	{
		intSample &= 0x7FFFFFFF;
		shiftL++;
	}
	intSample = (shiftL << (31 - shiftR)) | (intSample >> shiftR);
	if(sign)
		intSample = ~intSample | sign;
	return static_cast<float>(static_cast<int32>(intSample));
}

} // namespace DMO

#else
MPT_MSVC_WORKAROUND_LNK4221(Distortion)

#endif // !NO_PLUGINS

OPENMPT_NAMESPACE_END
