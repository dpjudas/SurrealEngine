#pragma once

#include <memory>
#include <vector>

class AudioSource;
class AudioSound;

class AudioMixer
{
public:
	static std::unique_ptr<AudioMixer> Create();

	virtual ~AudioMixer() = default;
	virtual AudioSound* AddSound(std::unique_ptr<AudioSource> source, bool looped = false, uint64_t loopStart = 0, uint64_t loopEnd = 0) = 0;
	virtual void RemoveSound(AudioSound* sound) = 0;
	virtual float GetSoundDuration(AudioSound* sound) = 0;
	virtual int PlaySound(int channel, AudioSound* sound, float volume, float pan, float pitch) = 0;
	virtual void UpdateSound(int channel, AudioSound* sound, float volume, float pan, float pitch) = 0;
	virtual void StopSound(int channel) = 0;
	virtual bool SoundFinished(int channel) = 0;
	virtual void PlayMusic(std::unique_ptr<AudioSource> source) = 0;
	virtual void SetMusicVolume(float volume) = 0;
	virtual void SetSoundVolume(float volume) = 0;
	virtual void Update() = 0;
};
