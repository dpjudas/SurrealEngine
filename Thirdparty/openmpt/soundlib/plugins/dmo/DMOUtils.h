/*
 * DMOUtils.h
 * ----------
 * Purpose: Utility functions shared by DMO plugins
 * Notes  : none
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


OPENMPT_NAMESPACE_BEGIN

#ifndef NO_PLUGINS

namespace DMO
{

// Computes (log2(x) + 1) * 2 ^ (shiftL - shiftR) (x = -2^31...2^31)
float logGain(float x, int32 shiftL, int32 shiftR);

}

#endif // !NO_PLUGINS

OPENMPT_NAMESPACE_END

