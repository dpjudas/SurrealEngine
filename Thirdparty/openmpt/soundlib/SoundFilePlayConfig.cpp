/*
 * SoundFilePlayConfig.cpp
 * -----------------------
 * Purpose: Configuration of sound levels, pan laws, etc... for various mix configurations.
 * Notes  : (currently none)
 * Authors: Olivier Lapicque
 *          OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"
#include "SoundFilePlayConfig.h"
#include "Mixer.h"

OPENMPT_NAMESPACE_BEGIN

CSoundFilePlayConfig::CSoundFilePlayConfig()
{
	setVSTiVolume(1.0f);
	SetMixLevels(MixLevels::Compatible);
}

void CSoundFilePlayConfig::SetMixLevels(MixLevels mixLevelType)
{
	switch (mixLevelType)
	{
		// Olivier's version gives us floats in [-0.5; 0.5] and slightly saturates VSTis.
		case MixLevels::Original:
			setVSTiAttenuation(1.0f);  // no attenuation
			setIntToFloat(1.0f/static_cast<float>(1<<28));
			setFloatToInt(static_cast<float>(1<<28));
			setGlobalVolumeAppliesToMaster(false);
			setUseGlobalPreAmp(true);
			setPanningMode(PanningMode::Undetermined);
			setDisplayDBValues(false);
			setNormalSamplePreAmp(256.0f);
			setNormalVSTiVol(100.0f);
			setNormalGlobalVol(128.0f);
			setExtraSampleAttenuation(MIXING_ATTENUATION);
			break;

		// Ericus' version gives us floats in [-0.06;0.06] and requires attenuation to
		// avoid massive VSTi saturation.
		case MixLevels::v1_17RC1:
			setVSTiAttenuation(32.0f);
			setIntToFloat(1.0f/static_cast<float>(0x07FFFFFFF));
			setFloatToInt(static_cast<float>(0x07FFFFFFF));
			setGlobalVolumeAppliesToMaster(false);
			setUseGlobalPreAmp(true);
			setPanningMode(PanningMode::Undetermined);
			setDisplayDBValues(false);
			setNormalSamplePreAmp(256.0f);
			setNormalVSTiVol(100.0f);
			setNormalGlobalVol(128.0f);
			setExtraSampleAttenuation(MIXING_ATTENUATION);
			break;

		// 1.17RC2 gives us floats in [-1.0; 1.0] and hopefully plays VSTis at
		// the right volume... but we attenuate by 2x to approx. match sample volume.
	
		case MixLevels::v1_17RC2:
			setVSTiAttenuation(2.0f);
			setIntToFloat(1.0f/MIXING_SCALEF);
			setFloatToInt(MIXING_SCALEF);
			setGlobalVolumeAppliesToMaster(true);
			setUseGlobalPreAmp(true);
			setPanningMode(PanningMode::Undetermined);
			setDisplayDBValues(false);
			setNormalSamplePreAmp(256.0f);
			setNormalVSTiVol(100.0f);
			setNormalGlobalVol(128.0f);
			setExtraSampleAttenuation(MIXING_ATTENUATION);
			break;

		// 1.17RC3 ignores the horrible global, system-specific pre-amp,
		// treats panning as balance to avoid saturation on loud sample (and because I think it's better :),
		// and allows display of attenuation in decibels.
		default:
		case MixLevels::v1_17RC3:
			setVSTiAttenuation(1.0f);
			setIntToFloat(1.0f/MIXING_SCALEF);
			setFloatToInt(MIXING_SCALEF);
			setGlobalVolumeAppliesToMaster(true);
			setUseGlobalPreAmp(false);
			setPanningMode(PanningMode::SoftPanning);
			setDisplayDBValues(true);
			setNormalSamplePreAmp(128.0f);
			setNormalVSTiVol(128.0f);
			setNormalGlobalVol(256.0f);
			setExtraSampleAttenuation(0);
			break;

		// A mixmode that is intended to be compatible to legacy trackers (IT/FT2/etc).
		// This is basically derived from mixmode 1.17 RC3, with panning mode and volume levels changed.
		// Sample attenuation is the same as in Schism Tracker (more attenuation than with RC3, thus VSTi attenuation is also higher).
		case MixLevels::Compatible:
		case MixLevels::CompatibleFT2:
			setVSTiAttenuation(0.75f);
			setIntToFloat(1.0f/MIXING_SCALEF);
			setFloatToInt(MIXING_SCALEF);
			setGlobalVolumeAppliesToMaster(true);
			setUseGlobalPreAmp(false);
			setPanningMode(mixLevelType == MixLevels::Compatible ? PanningMode::NoSoftPanning : PanningMode::FT2Panning);
			setDisplayDBValues(true);
			setNormalSamplePreAmp(mixLevelType == MixLevels::Compatible ? 256.0f : 192.0f);
			setNormalVSTiVol(mixLevelType == MixLevels::Compatible ? 256.0f : 192.0f);
			setNormalGlobalVol(256.0f);
			setExtraSampleAttenuation(1);
			break;

	}
}


OPENMPT_NAMESPACE_END
