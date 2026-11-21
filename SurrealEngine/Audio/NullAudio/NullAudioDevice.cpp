#include "NullAudioDevice.h"

#include "Utils/Exception.h"
#include "Utils/Logger.h"

NullAudioDevice::NullAudioDevice()
{
    sources.resize(32);
}

int NullAudioDevice::GetTotalChannels()
{
    return sources.size();
}

void NullAudioDevice::AddSound(USound* sound)
{
    sounds.push_back(sound);
}

void NullAudioDevice::RemoveSound(USound* sound)
{
    auto it = sounds.begin();

    while (it != sounds.end())
    {
        if (*it == sound)
        {
            sounds.erase(it);
            return;
        }
        it++;
    }
}

bool NullAudioDevice::IsPlaying(int channel)
{
    if (channel < 0 || channel >= sources.size())
        return false;

    return sources[channel].isPlaying();
}

void NullAudioDevice::PlayMusic(std::unique_ptr<AudioSource> source)
{
    // No-op
}

void NullAudioDevice::PlaySound(int channel, USound* sound, vec3& location, float volume, float radius, float pitch)
{
    if (!std::isfinite(volume) || volume < 0.0f || !std::isfinite(pitch) || channel >= sources.size())
        Exception::Throw("Invalid PlaySound arguments");

    auto source = sources[channel];

    if (source.isPlaying())
    {
        LogMessage("Attempted to play sound on active channel " + std::to_string(channel));
        return;
    }

    source.Play();
}

void NullAudioDevice::UpdateSound(int channel, vec3& location, float volume, float radius, float pitch)
{
    if (!std::isfinite(volume) || volume < 0.0f || !std::isfinite(pitch) || channel >= sources.size())
        Exception::Throw("Invalid PlaySound arguments");

    // Do nothing
}

void NullAudioDevice::StopSound(int channel)
{
    if (channel >= sources.size())
        Exception::Throw("Invalid StopSound arguments");

    sources[channel].Stop();
}

void NullAudioDevice::SetMusicVolume(float volume)
{
    // No-op
}

void NullAudioDevice::Update()
{
    // No-op
}
