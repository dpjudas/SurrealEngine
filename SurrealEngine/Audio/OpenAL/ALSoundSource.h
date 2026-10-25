#pragma once

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>
#include "Math/vec.h"

class UActor;
class USound;

class ALSoundSource
{
public:
	ALSoundSource();
	~ALSoundSource();

	void Play();
	void Stop();
	bool IsPlaying();
	USound* GetSound();
	void SetDopplerFactor(float newDopplerFactor);
	void SetPosition(vec3& newPosition);
	void SetRadius(float newRadius);
	void SetVelocity(vec3& newVelocity);
	void SetSound(USound* newSound);
	void SetSpatial(bool bSpatial);
	void SetVolume(float newVolume);
	void SetGlobalVolume(float newGlobalVolume);
	void SetPitch(float newPitch);
	void DoLoop();

	ALuint id = -1;

private:
	UActor* actor = nullptr;
	USound* sound = nullptr;
	vec3 position = vec3(0.0f);
	vec3 velocity = vec3(0.0f);
	float radius = 0.0f;
	float volume = 0.0f;
	float globalVolume = 1.0f; // Comes from Audio Subsystem
	float pitch = 0.0f;
	float dopplerFactor = 0.0f;
	bool bIs3d = false;
};
