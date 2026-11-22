
#include "Precomp.h"
#include "AudioPlayerDevice.h"
#include "AudioMixer.h"
#include "Packages/Engine/Actors/UActor.h"
#include "Packages/Engine/Resources/USound.h"
#include "Engine.h"
#include <algorithm>
#include <cstring>

AudioPlayerDevice::AudioPlayerDevice()
{
	mixer = AudioMixer::Create();
	channels.resize(16);
}

AudioPlayerDevice::~AudioPlayerDevice()
{
}

void AudioPlayerDevice::AddSound(USound* sound)
{
	auto source = std::make_unique<RawAudioSource>(sound->samples.data(), sound->samples.size(), 1, sound->frequency);
	sound->handle = mixer->AddSound(std::move(source), sound->loopInfo.Looped, sound->loopInfo.LoopStart, sound->loopInfo.LoopEnd);
}

void AudioPlayerDevice::RemoveSound(USound* sound)
{
	mixer->RemoveSound(static_cast<AudioSound*>(sound->handle));
}

bool AudioPlayerDevice::IsPlaying(int channel)
{
	return !mixer->SoundFinished(channel);
}

int AudioPlayerDevice::GetTotalChannels()
{
	return (int)channels.size();
}

void AudioPlayerDevice::PlaySound(int channel, USound* sound, vec3& location, float volume, float radius, float pitch)
{
	channels[channel].sound = static_cast<AudioSound*>(sound->handle);
	channels[channel].location = location;
	channels[channel].volume = volume;
	channels[channel].radius = radius;
	channels[channel].pitch = pitch;
	mixer->PlaySound(channel, channels[channel].sound, EffectiveVolume(channel), EffectivePan(channel), EffectivePitch(channel));
}

void AudioPlayerDevice::PlayMusic(std::unique_ptr<AudioSource> source)
{
	mixer->PlayMusic(std::move(source));
}

void AudioPlayerDevice::UpdateSound(int channel, vec3& location, float volume, float radius, float pitch)
{
	channels[channel].location = location;
	channels[channel].volume = volume;
	channels[channel].radius = radius;
	channels[channel].pitch = pitch;
	mixer->UpdateSound(channel, channels[channel].sound, EffectiveVolume(channel), EffectivePan(channel), EffectivePitch(channel));
}

void AudioPlayerDevice::StopSound(int channel)
{
	mixer->StopSound(channel);
}

void AudioPlayerDevice::SetMusicVolume(float volume)
{
	mixer->SetMusicVolume(volume);
}

void AudioPlayerDevice::Update()
{
	UActor* listener = engine->CameraActor;
	if (listener)
	{
		viewport.location = listener->Location();
		viewport.velocity = listener->Velocity();
		Coords::Rotation(listener->Rotation()).GetAxes(viewport.at, viewport.left, viewport.up);
	}

	for (int channel = 0, count = (int)channels.size(); channel < count; channel++)
	{
		if (channels[channel].sound)
			mixer->UpdateSound(channel, channels[channel].sound, EffectiveVolume(channel), EffectivePan(channel), EffectivePitch(channel));
	}

	mixer->Update();
}

float AudioPlayerDevice::EffectiveVolume(int channel) const
{
	if (channels[channel].radius > 0.0f)
	{
		vec3 L = channels[channel].location - viewport.location;
		float distsqr = dot(L, L);
		float radiussqr = channels[channel].radius * channels[channel].radius;
		float attenuation = std::max(1.0f - distsqr / radiussqr, 0.0f);
		return channels[channel].volume * attenuation;
	}
	else
	{
		return channels[channel].volume;
	}
}

float AudioPlayerDevice::EffectivePan(int channel) const
{
	if (channels[channel].radius > 0.0f)
	{
		vec3 L = normalize(channels[channel].location - viewport.location);
		return std::max(std::min(dot(L, viewport.left), 1.0f), -1.0f);
	}
	else
	{
		return 0.0f;
	}
}

float AudioPlayerDevice::EffectivePitch(int channel) const
{
	if (channels[channel].radius > 0.0f)
	{
		float speed = dot(normalize(channels[channel].location - viewport.location), viewport.velocity);
		speed *= 0.00005f;
		if (speed >= 0.0f)
			speed = std::min(1.0f + speed, 2.0f);
		else
			speed = 1.0f / std::min(1.0f - speed, 2.0f);
		return channels[channel].pitch * speed;
	}
	else
	{
		return channels[channel].pitch;
	}
}

/////////////////////////////////////////////////////////////////////////////

RawAudioSource::RawAudioSource(const float* samples, size_t size, int channels, int frequency)
	: samples(samples), samplecount(size), channels(channels), frequency(frequency)
{
}

void RawAudioSource::SeekToSample(uint64_t position)
{
	pos = std::min(position, samplecount);
}

size_t RawAudioSource::ReadSamples(float* output, size_t numSamples)
{
	size_t count = (size_t)std::min(samplecount - pos, (uint64_t)numSamples);
	std::memcpy(output, samples + pos, count * sizeof(float));
	pos += count;
	return count;
}
