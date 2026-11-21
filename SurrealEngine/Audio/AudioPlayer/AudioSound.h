#pragma once

#include "AudioMixer.h"

class AudioSource;

class AudioSound
{
public:
	AudioSound(int mixing_frequency, std::unique_ptr<AudioSource> source, const AudioLoopInfo& inloopinfo);

	std::vector<float> samples;
	float duration = 0.0f;
	AudioLoopInfo loopinfo;
};
