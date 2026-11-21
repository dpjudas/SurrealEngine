
#include "Precomp.h"
#include "NullAudioDevice.h"
#include "Utils/Exception.h"
#include "Utils/Logger.h"
#include "Packages/Engine/Resources/USound.h"
#include <chrono>

NullAudioDevice::NullAudioDevice()
{
	channels.resize(32);
	using namespace std::chrono;
	currentTime = duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}

int NullAudioDevice::GetTotalChannels()
{
	return (int)channels.size();
}

void NullAudioDevice::AddSound(USound* sound)
{
	sound->handle = new NullSound(sound);
}

void NullAudioDevice::RemoveSound(USound* sound)
{
	delete static_cast<NullSound*>(sound->handle);
}

bool NullAudioDevice::IsPlaying(int channel)
{
	if (channel < 0 || channel >= channels.size())
		return false;

	return channels[channel].IsPlaying(currentTime);
}

void NullAudioDevice::PlayMusic(std::unique_ptr<AudioSource> source)
{
	// No-op
}

void NullAudioDevice::PlaySound(int channel, USound* sound, vec3& location, float volume, float radius, float pitch)
{
	if (!std::isfinite(volume) || volume < 0.0f || !std::isfinite(pitch) || channel >= channels.size())
		Exception::Throw("Invalid PlaySound arguments");

	auto& source = channels[channel];

	if (source.IsPlaying(currentTime))
	{
		LogMessage("Attempted to play sound on active channel " + std::to_string(channel));
		return;
	}

	source.Play(static_cast<NullSound*>(sound->handle), currentTime);
}

void NullAudioDevice::UpdateSound(int channel, vec3& location, float volume, float radius, float pitch)
{
	if (!std::isfinite(volume) || volume < 0.0f || !std::isfinite(pitch) || channel >= channels.size())
		Exception::Throw("Invalid PlaySound arguments");

	// Do nothing
}

void NullAudioDevice::StopSound(int channel)
{
	if (channel >= channels.size())
		Exception::Throw("Invalid StopSound arguments");

	channels[channel].Stop();
}

void NullAudioDevice::SetMusicVolume(float volume)
{
	// No-op
}

void NullAudioDevice::Update()
{
	using namespace std::chrono;
	currentTime = duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}
