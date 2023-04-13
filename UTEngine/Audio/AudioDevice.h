#pragma once

#include <memory>
#include <vector>

class AudioSource;
class USound;

class AudioDevice
{
public:
	static std::unique_ptr<AudioDevice> Create(int frequency, int numVoices);

	virtual ~AudioDevice() = default;
	virtual void AddSound(USound* sound) = 0;
	virtual void RemoveSound(USound* sound) = 0;
	virtual int PlaySound(int channel, USound* sound, float volume, float pan, float pitch) = 0;
	virtual void UpdateSound(int channel, USound* sound, float volume, float pan, float pitch) = 0;
	virtual void StopSound(int channel) = 0;
	virtual void PlayMusic(std::unique_ptr<AudioSource> source) = 0;
	virtual void SetMusicVolume(float volume) = 0;
	virtual void SetSoundVolume(float volume) = 0;
	virtual void Update() = 0;

protected:
	int frequency = 48000;
};
