#pragma once

class USound;
class NullSound;

class NullAudioSource
{
public:
	void Play(NullSound* sound, uint64_t currentTime);
	void Stop();
	bool IsPlaying(uint64_t currentTime) const;

private:
	NullSound* sound = nullptr;
	uint64_t startTime = 0;
};

class NullSound
{
public:
	NullSound(USound* sound);

	uint64_t length = 0;
	bool looping = false;
};
