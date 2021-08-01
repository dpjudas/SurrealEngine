#pragma once

#include <memory>
#include <vector>
#include "Math/vec.h"
#include "Math/mat.h"

class AudioSource;
class AudioSound;

class AudioMixer
{
public:
	static std::unique_ptr<AudioMixer> Create();

	virtual ~AudioMixer() = default;
	virtual AudioSound* AddSound(std::unique_ptr<AudioSource> source) = 0;
	virtual float GetSoundDuration(AudioSound* sound) = 0;
	virtual void PlaySound(void* owner, int slot, AudioSound* sound, const vec3& location, float volume, float radius, float pitch) = 0;
	virtual void PlayMusic(std::unique_ptr<AudioSource> source) = 0;
	virtual void SetViewport(const mat4& worldToView) = 0;
	virtual void Update() = 0;
};
