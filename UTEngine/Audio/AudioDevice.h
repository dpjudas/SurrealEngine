#pragma once

#include <memory>
#include <vector>

class AudioSource;

class AudioLoopInfo
{
public:
	bool Looped = false;
	uint64_t LoopStart = 0;
	uint64_t LoopEnd = 0;
};

class AudioSound
{
public:
	AudioSound(std::unique_ptr<AudioSource> source, const AudioLoopInfo& inloopinfo);

	std::vector<float> samples;
	float duration = 0.0f;
	int frequency = 0;
	AudioLoopInfo loopinfo;
};

class AudioDevice
{
public:
	static std::unique_ptr<AudioDevice> Create(int frequency);

	virtual ~AudioDevice() = default;
	virtual AudioSound* AddSound(std::unique_ptr<AudioSource> source, const AudioLoopInfo& loopinfo = {}) = 0;
	virtual void RemoveSound(AudioSound* sound) = 0;
	virtual int PlaySound(int channel, AudioSound* sound, float volume, float pan, float pitch) = 0;
	virtual void UpdateSound(int channel, AudioSound* sound, float volume, float pan, float pitch) = 0;
	virtual void StopSound(int channel) = 0;
	virtual bool SoundFinished(int channel) = 0;
	virtual void PlayMusic(std::unique_ptr<AudioSource> source) = 0;
	virtual void SetMusicVolume(float volume) = 0;
	virtual void SetSoundVolume(float volume) = 0;
	virtual void Update() = 0;

protected:
	int frequency = 48000;
};
