
#pragma once

#include <memory>

class AudioSource;

class AudioPlayer
{
public:
	static std::unique_ptr<AudioPlayer> Create(std::unique_ptr<AudioSource> source);

	virtual ~AudioPlayer() = default;
	virtual int GetMixingFrequency() const = 0;
};
