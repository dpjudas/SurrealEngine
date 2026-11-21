#pragma once

#include "Audio/AudioDevice.h"

class AudioPlayerDevice : public AudioDevice
{
public:
	AudioPlayerDevice();
	~AudioPlayerDevice();

	void AddSound(USound* sound) override;
	void RemoveSound(USound* sound) override;
	bool IsPlaying(int channel) override;
	int GetTotalChannels() override;
	void PlaySound(int channel, USound* sound, vec3& location, float volume, float radius, float pitch) override;
	void PlayMusic(std::unique_ptr<AudioSource> source) override;
	void UpdateSound(int channel, vec3& location, float volume, float radius, float pitch) override;
	void StopSound(int channel) override;
	void SetMusicVolume(float volume) override;
	void Update() override;
};
