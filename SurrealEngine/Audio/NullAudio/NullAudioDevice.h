#pragma once

#include "Audio/AudioDevice.h"
#include "NullAudioSource.h"
#include "Utils/Array.h"

// A dummy audio device that does no sound output
class NullAudioDevice : public AudioDevice
{
public:
	NullAudioDevice();
	int GetTotalChannels() override;
	void AddSound(USound* sound) override;
	void RemoveSound(USound* sound) override;
	bool IsPlaying(int channel) override;
	void PlayMusic(std::unique_ptr<AudioSource> source) override;
	void PlaySound(int channel, USound* sound, vec3& location, float volume, float radius, float pitch) override;
	void UpdateSound(int channel, vec3& location, float volume, float radius, float pitch) override;
	void StopSound(int channel) override;
	void SetMusicVolume(float volume) override;
	void Update() override;

private:
	Array<NullAudioSource> channels;
	uint64_t currentTime = 0;
};
