#pragma once

#include "Audio/AudioSource.h"

class AudioMixerImpl;
class ActiveSound;

class AudioMixerSource : public AudioSource
{
public:
	AudioMixerSource(AudioMixerImpl* mixer);

	int GetFrequency() override;
	int GetChannels() override { return 2; }
	int GetSamples() override { return -1; }
	void SeekToSample(uint64_t position) override {}
	size_t ReadSamples(float* output, size_t samples) override;

private:
	void TransferFromClient();
	void CopyMusic(float* output, size_t samples);
	void MixSounds(float* output, size_t samples);
	void HardLimit(float* output, size_t samples);

	void GetVolume(const ActiveSound& sound, float& leftVolume, float& rightVolume);

	AudioMixerImpl* mixer = nullptr;
	std::map<int, ActiveSound> sounds;
	std::vector<int> stoppedsounds;
	std::unique_ptr<AudioSource> music;
	float soundvolume = 1.0f;
	float musicvolume = 1.0f;
};
