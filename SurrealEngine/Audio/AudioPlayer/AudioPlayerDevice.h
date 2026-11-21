#pragma once

#include "Audio/AudioDevice.h"
#include "Audio/AudioSource.h"

class AudioMixer;
class AudioSound;

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

private:
	std::unique_ptr<AudioMixer> mixer;
	std::vector<AudioSound*> channels;
};

class RawAudioSource : public AudioSource
{
public:
	RawAudioSource(const float* samples, size_t size, int channels, int frequency);

	int GetFrequency() override { return frequency; }
	int GetChannels() override { return channels; }
	int GetSamples() override { return (int)samplecount; }
	void SeekToSample(uint64_t position) override;

	size_t ReadSamples(float* output, size_t numSamples) override;

private:
	const float* samples = nullptr;
	uint64_t samplecount = 0;
	int channels = 0;
	int frequency = 0;
	uint64_t pos = 0;
};
