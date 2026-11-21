
#include "Precomp.h"
#include "AudioMixerImpl.h"
#include "AudioMixerSource.h"
#include "AudioPlayer.h"
#include "AudioSound.h"
#include <cmath>

AudioMixerImpl::AudioMixerImpl()
{
	player = AudioPlayer::Create(std::make_unique<AudioMixerSource>(this));
}

AudioMixerImpl::~AudioMixerImpl()
{
	player.reset();
}

AudioSound* AudioMixerImpl::AddSound(std::unique_ptr<AudioSource> source, bool looped, uint64_t loopStart, uint64_t loopEnd)
{
	sounds.push_back(std::make_unique<AudioSound>(mixing_frequency, std::move(source), looped, loopStart, loopEnd));
	return sounds.back().get();
}

void AudioMixerImpl::RemoveSound(AudioSound* sound)
{
}

float AudioMixerImpl::GetSoundDuration(AudioSound* sound)
{
	return sound->duration;
}

int AudioMixerImpl::PlaySound(int channel, AudioSound* sound, float volume, float pan, float pitch)
{
	if (!std::isfinite(volume) || !std::isfinite(pan) || !std::isfinite(pitch))
		throw std::runtime_error("Invalid PlaySound arguments");

	ActiveSound s;
	s.channel = channel;
	s.play = true;
	s.update = false;
	s.sound = sound;
	s.volume = volume;
	s.pan = pan;
	s.pitch = pitch;
	client.sounds.push_back(std::move(s));
	channelplaying[channel]++;
	return channel;
}

void AudioMixerImpl::UpdateSound(int channel, AudioSound* sound, float volume, float pan, float pitch)
{
	if (!std::isfinite(volume) || !std::isfinite(pan) || !std::isfinite(pitch))
		throw std::runtime_error("Invalid UpdateSound arguments");

	ActiveSound s;
	s.channel = channel;
	s.play = false;
	s.update = true;
	s.sound = sound;
	s.volume = volume;
	s.pan = pan;
	s.pitch = pitch;
	client.sounds.push_back(std::move(s));
}

void AudioMixerImpl::StopSound(int channel)
{
	ActiveSound s;
	s.channel = channel;
	s.play = false;
	s.update = false;
	client.sounds.push_back(std::move(s));
}

bool AudioMixerImpl::SoundFinished(int channel)
{
	auto it = channelplaying.find(channel);
	return it == channelplaying.end() || it->second == 0;
}

void AudioMixerImpl::PlayMusic(std::unique_ptr<AudioSource> source)
{
	if (source && source->GetChannels() != 2)
		throw std::runtime_error("Only stereo music is supported");

	if (source && source->GetFrequency() != mixing_frequency)
	{
		source = AudioSource::CreateResampler(mixing_frequency, std::move(source));
	}
	client.music = std::move(source);
	client.musicupdate = true;
}

void AudioMixerImpl::SetMusicVolume(float volume)
{
	client.musicvolume = volume;
}

void AudioMixerImpl::SetSoundVolume(float volume)
{
	client.soundvolume = volume;
}

void AudioMixerImpl::Update()
{
	std::unique_lock<std::mutex> lock(mutex);
	for (ActiveSound& s : client.sounds)
		transfer.sounds.push_back(s);
	client.sounds.clear();
	if (client.musicupdate)
	{
		transfer.music = std::move(client.music);
		transfer.musicupdate = true;
	}
	client.music = {};
	client.musicupdate = false;

	transfer.musicvolume = client.musicvolume;
	transfer.soundvolume = client.soundvolume;

	for (int c : channelstopped)
	{
		auto it = channelplaying.find(c);
		it->second--;
		if (it->second == 0)
			channelplaying.erase(it);
	}
	channelstopped.clear();
}
