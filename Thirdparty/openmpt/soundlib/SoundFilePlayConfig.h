/*
 * SoundFilePlayConfig.h
 * ---------------------
 * Purpose: Configuration of sound levels, pan laws, etc... for various mix configurations.
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#pragma once

#include "openmpt/all/BuildSettings.hpp"

OPENMPT_NAMESPACE_BEGIN

enum class TempoMode : uint8
{
	Classic     = 0,
	Alternative = 1,
	Modern      = 2,
	NumModes
};

enum class MixLevels : uint8
{
	Original      = 0,
	v1_17RC1      = 1,
	v1_17RC2      = 2,
	v1_17RC3      = 3,
	Compatible    = 4,
	CompatibleFT2 = 5,
	NumMixLevels
};

enum class PanningMode : uint8
{
	Undetermined,
	SoftPanning,
	NoSoftPanning,
	FT2Panning,
};

// Class used to store settings for a song file.
class CSoundFilePlayConfig
{
public:
	CSoundFilePlayConfig();

	void SetMixLevels(MixLevels mixLevelType);

//getters/setters
	bool getGlobalVolumeAppliesToMaster() const { return m_globalVolumeAppliesToMaster; }
	void setGlobalVolumeAppliesToMaster(bool inGlobalVolumeAppliesToMaster) { m_globalVolumeAppliesToMaster=inGlobalVolumeAppliesToMaster; }

	// user-controllable VSTi gain factor.
	float getVSTiVolume() const { return m_VSTiVolume; }
	void  setVSTiVolume(float inVSTiVolume) { m_VSTiVolume = inVSTiVolume; }

	// default VSTi gain factor, different depending on the MPT version we're "emulating"
	float getVSTiAttenuation() const { return m_VSTiAttenuation; }
	void  setVSTiAttenuation(float inVSTiAttenuation) { m_VSTiAttenuation = inVSTiAttenuation; }

	float getIntToFloat() const { return m_IntToFloat; }
	void  setIntToFloat(float inIntToFloat) { m_IntToFloat = inIntToFloat; }

	float getFloatToInt() const { return m_FloatToInt; }
	void  setFloatToInt(float inFloatToInt) { m_FloatToInt = inFloatToInt; }

	bool getUseGlobalPreAmp() const { return m_ignorePreAmp; }
	void setUseGlobalPreAmp(bool inUseGlobalPreAmp) { m_ignorePreAmp = inUseGlobalPreAmp; }

	PanningMode getPanningMode() const { return m_forceSoftPanning; }
	void setPanningMode(PanningMode inForceSoftPanning) { m_forceSoftPanning = inForceSoftPanning; }

	bool getDisplayDBValues() const { return m_displayDBValues; }
	void setDisplayDBValues(bool in) { m_displayDBValues = in; }

	// Values at which volumes are unchanged
	float getNormalSamplePreAmp() const { return m_normalSamplePreAmp; }
	void setNormalSamplePreAmp(float in) { m_normalSamplePreAmp = in; }
	float getNormalVSTiVol() const { return m_normalVSTiVol; }
	void setNormalVSTiVol(float in) { m_normalVSTiVol = in; }
	float getNormalGlobalVol() const { return m_normalGlobalVol; }
	void setNormalGlobalVol(float in) { m_normalGlobalVol = in; }

	// Extra sample attenuation in bits
	int getExtraSampleAttenuation() const { return m_extraAttenuation; }
	void setExtraSampleAttenuation(int attn) { m_extraAttenuation = attn; }

protected:

	float m_IntToFloat;
	float m_FloatToInt;
	float m_VSTiAttenuation;
	float m_VSTiVolume;

	float m_normalSamplePreAmp;
	float m_normalVSTiVol;
	float m_normalGlobalVol;

	int m_extraAttenuation;
	PanningMode m_forceSoftPanning;
	bool m_globalVolumeAppliesToMaster;
	bool m_ignorePreAmp;
	bool m_displayDBValues;
};

OPENMPT_NAMESPACE_END
