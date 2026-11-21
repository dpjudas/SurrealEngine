
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
	// To do: handle location
	float pan = 0.0f;
	channels[channel] = static_cast<AudioSound*>(sound->handle);
	mixer->PlaySound(channel, channels[channel], volume, pan, pitch);
}

void AudioPlayerDevice::PlayMusic(std::unique_ptr<AudioSource> source)
{
	mixer->PlayMusic(std::move(source));
}

void AudioPlayerDevice::UpdateSound(int channel, vec3& location, float volume, float radius, float pitch)
{
	// To do: handle location
	float pan = 0.0f;
	mixer->UpdateSound(channel, channels[channel], volume, pan, pitch);
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
		// Update listener properties
		vec3& location = listener->Location();
		vec3& velocity = listener->Velocity();

		vec3 at, left, up;
		Coords::Rotation(listener->Rotation()).GetAxes(at, left, up);

		//ALfloat listenerOri[6] = { up.x, up.y, -up.z, -at.x, -at.y, at.z };
		//alListener3f(AL_POSITION, location.x, location.y, -location.z);
		//alListener3f(AL_VELOCITY, velocity.x, velocity.y, -velocity.z);
		//alListenerfv(AL_ORIENTATION, listenerOri);

		// To do: handle location
	}

	mixer->Update();
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
