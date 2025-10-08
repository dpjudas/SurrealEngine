/*
 * EQ.h
 * ----
 * Purpose: Mixing code for equalizer.
 * Notes  : Ugh... This should really be removed at some point.
 * Authors: Olivier Lapicque
 *          OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#pragma once

#include "openmpt/all/BuildSettings.hpp"

#include "openmpt/base/Types.hpp"
#include "openmpt/soundbase/MixSample.hpp"

#include <array>

#include <cstddef>

OPENMPT_NAMESPACE_BEGIN

#ifndef NO_EQ

inline constexpr std::size_t MAX_EQ_CHANNELS = 4;
inline constexpr std::size_t MAX_EQ_BANDS = 6;

struct EQBANDSTATE
{
	float x1 = 0.0f;
	float x2 = 0.0f;
	float y1 = 0.0f;
	float y2 = 0.0f;
};

struct EQBANDSETTINGS
{
	float a0;
	float a1;
	float a2;
	float b1;
	float b2;
	float Gain;
	float CenterFrequency;
};

class CEQ
{
private:
	std::array<std::array<EQBANDSTATE, MAX_EQ_BANDS>, MAX_EQ_CHANNELS> m_ChannelState;
	std::array<EQBANDSETTINGS, MAX_EQ_BANDS> m_Bands;
	template <typename TMixSample>
	void ProcessTemplate(TMixSample *frontBuffer, TMixSample *rearBuffer, std::size_t countFrames, std::size_t numChannels);
public:
	CEQ();
	void Initialize(bool bReset, uint32 MixingFreq);
	void Process(MixSampleInt *frontBuffer, MixSampleInt *rearBuffer, std::size_t countFrames, std::size_t numChannels);
	void Process(MixSampleFloat *frontBuffer, MixSampleFloat *rearBuffer, std::size_t countFrames, std::size_t numChannels);
	void SetEQGains(const uint32 *pGains, const uint32 *pFreqs, bool bReset, uint32 MixingFreq);
};

#endif // !NO_EQ

OPENMPT_NAMESPACE_END
