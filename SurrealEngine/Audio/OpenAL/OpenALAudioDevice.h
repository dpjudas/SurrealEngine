#pragma once

#include "Audio/AudioDevice.h"
#include "ALSoundSource.h"
#include "RingQueue.h"
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>
#include <mutex>
#include <thread>
#include <memory>

class OpenALAudioDevice : public AudioDevice
{
public:
	OpenALAudioDevice(int inFrequency, int numVoices, int inMusicBufferCount, int inMusicBufferSize);
	~OpenALAudioDevice();

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
	std::string getALErrorString();

	void PlayMusicBuffer(AudioSource* music);
	void UpdateMusicBuffer(AudioSource* music);

	void MusicThreadMain();

	ALCdevice* alDevice = nullptr;
	ALCcontext* alContext = nullptr;
	ALenum alError = 0;
	ALuint alMusicSource = 0;
	Array<ALuint> alMusicBuffers;
	Array<ALSoundSource> sources;
	ALint monoSources = 0;
	ALint stereoSources = 0;

	int frequency = 48000;
	Array<USound*> sounds;

	// Note: variables changed in musicThreadData *must* be done within a mutex lock to be thread safe
	struct
	{
		std::mutex mutex;
		std::thread thread;
		bool exitFlag = false;
		std::unique_ptr<AudioSource> music;
		bool musicUpdate = false;
	} musicThreadData;

	RingQueue<float*> musicQueue;
	int musicBufferCount = 0;
	int musicBufferSize = 0;
	Array<float> musicBuffer;
	float currentMusicVolume = 0.0f;
	float targetMusicVolume = 0.0f;
	float fadeRate = 0.0f;
};
