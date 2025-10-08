/*
* Paula.h
* -------
* Purpose: Emulating the Amiga's sound chip, Paula, by implementing resampling using band-limited steps (BLEPs)
* Notes  : (currently none)
* Authors: OpenMPT Devs
*          Antti S. Lankila
* The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
*/

#pragma once

#include "openmpt/all/BuildSettings.hpp"

#include "Snd_defs.h"
#include "Mixer.h"

OPENMPT_NAMESPACE_BEGIN

namespace Paula
{

inline constexpr int PAULA_HZ = 3546895;
inline constexpr int MINIMUM_INTERVAL = 4;  // Tradeoff between quality and speed (lower = less aliasing)
inline constexpr int BLEP_SCALE = 17;  // TODO: Should be 0 for float mixer
inline constexpr int BLEP_SIZE = 2048;

using BlepArray = std::array<mixsample_t, BLEP_SIZE>;


class BlepTables
{
	enum AmigaFilter
	{
		A500Off = 0,
		A500On,
		A1200Off,
		A1200On,
		Unfiltered,
		NumFilterTypes
	};

	std::array<Paula::BlepArray, AmigaFilter::NumFilterTypes> WinSincIntegral;

public:
	void InitTables();
	const Paula::BlepArray &GetAmigaTable(Resampling::AmigaFilter amigaType, bool enableFilter) const;
};


class State
{
	// MAX_BLEPS configures how many BLEPs (volume steps) are being kept track of per channel,
	// and thus directly influences how much memory this feature wastes per virtual channel.
	// Paula::BLEP_SIZE / Paula::MINIMUM_INTERVAL would be a safe maximum,
	// but even a sample (alternating at +1/-1, thus causing a step on every frame) played at 200 kHz,
	// which is way out of spec for the Amiga, will only get close to 128 active BLEPs with Paula::MINIMUM_INTERVAL == 4.
	// Hence 128 is chosen as a tradeoff between quality and memory consumption.
	static constexpr uint16 MAX_BLEPS = 128;

	struct Blep
	{
		int16 level;
		uint16 age;
	};

public:
	SamplePosition remainder, stepRemainder;
	int numSteps;  // Number of full-length steps
private:
	uint16 activeBleps = 0, firstBlep = 0;  // Count of simultaneous bleps to keep track of
	int16 globalOutputLevel = 0;            // The instantenous value of Paula output
	Blep blepState[MAX_BLEPS];

public:
	State(uint32 sampleRate = 48000);

	void Reset();
	void InputSample(int16 sample);
	int OutputSample(const BlepArray &WinSincIntegral);
	void Clock(int cycles);
};

}

OPENMPT_NAMESPACE_END
