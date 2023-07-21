
#pragma once

#include <memory>
#include <vector>

class AudioSource;

class AudioPlayer
{
public:
	static std::unique_ptr<AudioPlayer> Create(std::unique_ptr<AudioSource> source);

	virtual ~AudioPlayer() = default;

	int GetMixingFrequency() const
	{
		return mixing_frequency;
	}

protected:
	int mixing_frequency = 48000;
};
