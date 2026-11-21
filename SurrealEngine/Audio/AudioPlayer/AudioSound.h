#pragma once

#include "AudioMixer.h"

class AudioSource;

class AudioSound
{
public:
	AudioSound(int mixing_frequency, std::unique_ptr<AudioSource> source, bool looped = false, uint64_t loopStart = 0, uint64_t loopEnd = 0);

	std::vector<float> samples;
	float duration = 0.0f;
	struct
	{
		bool Looped = false;
		uint64_t LoopStart = 0;
		uint64_t LoopEnd = 0;
	} loopinfo;
};
