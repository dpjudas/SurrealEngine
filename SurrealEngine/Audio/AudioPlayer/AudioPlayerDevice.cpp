
#include "Precomp.h"
#include "AudioPlayerDevice.h"

AudioPlayerDevice::AudioPlayerDevice()
{
}

AudioPlayerDevice::~AudioPlayerDevice()
{
}

void AudioPlayerDevice::AddSound(USound* sound)
{
}

void AudioPlayerDevice::RemoveSound(USound* sound)
{
}

bool AudioPlayerDevice::IsPlaying(int channel)
{
	return false;
}

int AudioPlayerDevice::GetTotalChannels()
{
	return 0;
}

void AudioPlayerDevice::PlaySound(int channel, USound* sound, vec3& location, float volume, float radius, float pitch)
{
}

void AudioPlayerDevice::PlayMusic(std::unique_ptr<AudioSource> source)
{
}

void AudioPlayerDevice::UpdateSound(int channel, vec3& location, float volume, float radius, float pitch)
{
}

void AudioPlayerDevice::StopSound(int channel)
{
}

void AudioPlayerDevice::SetMusicVolume(float volume)
{
}

void AudioPlayerDevice::Update()
{
}
