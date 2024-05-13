#pragma once

#include "AudioSource.h"

#include <memory>
#include <mutex>
#include <vector>
#include <thread>
#include "Math/vec.h"

class AudioSource;
class USound;

class AudioDevice
{
public:
	static std::unique_ptr<AudioDevice> Create(int frequency, int numVoices, int musicBufferCount, int musicBufferSize);

	virtual ~AudioDevice() = default;
	virtual void AddSound(USound* sound) = 0;
	virtual void RemoveSound(USound* sound) = 0;
	virtual bool IsPlaying(int channel) = 0;
	virtual int PlaySound(int channel, USound* sound, vec3& location, float volume, float radius, float pitch) = 0;
	virtual void PlayMusic(std::unique_ptr<AudioSource> source) = 0;
	virtual void UpdateSound(int channel, USound* sound, vec3& location, float volume, float radius, float pitch) = 0;
	virtual void StopSound(int channel) = 0;
	virtual void SetMusicVolume(float volume) = 0;
	virtual void SetSoundVolume(float volume) = 0;
	virtual void Update() = 0;
};
