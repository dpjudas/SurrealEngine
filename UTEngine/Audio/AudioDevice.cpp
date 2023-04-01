
#include "Precomp.h"
#include "AudioDevice.h"
#include "AudioSource.h"
#include <mutex>
#include <stdexcept>
#include <map>
#include <cmath>

class AudioDeviceImpl;

class AudioSound
{
public:
	AudioSound(int mixing_frequency, std::unique_ptr<AudioSource> source, const AudioLoopInfo& inloopinfo) : loopinfo(inloopinfo)
	{
		duration = samples.size() / (float)mixing_frequency;

		if (source->GetChannels() == 1)
		{
			uint64_t oldSampleCount = source->GetSamples();
			if (source->GetFrequency() != mixing_frequency)
			{
				source = AudioSource::CreateResampler(mixing_frequency, std::move(source));
			}

			samples.resize(source->GetSamples());
			samples.resize(source->ReadSamples(samples.data(), samples.size()));

			// Remove any audio pops at end of the sound that the resampler might have created
			if (samples.size() >= 16)
			{
				for (size_t i = 1; i < 16; i++)
				{
					float t = i / 16.0f;
					size_t pos = samples.size() - i;
					samples[pos] = samples[pos] * t;
				}
			}

			if (oldSampleCount > 0)
			{
				uint64_t newSampleCount = samples.size();
				loopinfo.LoopStart = std::max(std::min(loopinfo.LoopStart * newSampleCount / oldSampleCount, newSampleCount - 1), (uint64_t)0);
				loopinfo.LoopEnd = std::max(std::min(loopinfo.LoopEnd * newSampleCount / oldSampleCount, newSampleCount), (uint64_t)0);
			}
			else
			{
				loopinfo = {};
			}
		}
		else
		{
			// TODO: should support stereo sounds too
			throw std::runtime_error("Only mono sounds are supported");
		}
	}

	std::vector<float> samples;
	float duration = 0.0f;
	AudioLoopInfo loopinfo;
};

class ActiveSound
{
public:
	int channel = 0;
	bool play = false;
	bool update = false;
	AudioSound* sound = nullptr;
	float volume = 0.0f;
	float pan = 0.0f;
	float pitch = 0.0f;

	double pos = 0;
};

#ifdef WIN32

#define WIN32_MEAN_AND_LEAN
#include <Windows.h>
#include <xaudio2.h>

class XAudio2Device : public AudioDevice
{
public:
	XAudio2Device()
	{

	}

	~XAudio2Device()
	{
	}

	AudioSound* AddSound(std::unique_ptr<AudioSource> source, const AudioLoopInfo& loopinfo) override
	{
		sounds.push_back(std::make_unique<AudioSound>(frequency, std::move(source), loopinfo));
		return sounds.back().get();
	}

	void RemoveSound(AudioSound* sound) override
	{
	}

	float GetSoundDuration(AudioSound* sound) override
	{
		return sound->duration;
	}

	int PlaySound(int channel, AudioSound* sound, float volume, float pan, float pitch) override
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

	void UpdateSound(int channel, AudioSound* sound, float volume, float pan, float pitch) override
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

	void StopSound(int channel) override
	{
		ActiveSound s;
		s.channel = channel;
		s.play = false;
		s.update = false;
		client.sounds.push_back(std::move(s));
	}

	bool SoundFinished(int channel) override
	{
		auto it = channelplaying.find(channel);
		return it == channelplaying.end() || it->second == 0;
	}

	void PlayMusic(std::unique_ptr<AudioSource> source) override
	{
		//if (source && source->GetChannels() != 2)
		//	throw std::runtime_error("Only stereo music is supported");
		//
		//if (source && source->GetFrequency() != frequency)
		//{
		//	source = AudioSource::CreateResampler(frequency, std::move(source));
		//}
		client.music = std::move(source);
		client.musicupdate = true;
	}

	void SetMusicVolume(float volume) override
	{
		client.musicvolume = volume;
	}

	void SetSoundVolume(float volume) override
	{
		client.soundvolume = volume;
	}

	void Update() override
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

	int frequency = 48000;

	std::vector<std::unique_ptr<AudioSound>> sounds;
	std::map<int, int> channelplaying;

	struct
	{
		std::vector<ActiveSound> sounds;
		std::unique_ptr<AudioSource> music;
		bool musicupdate = false;
		float soundvolume = 1.0f;
		float musicvolume = 1.0f;
	} client, transfer;
	std::vector<int> channelstopped;

	std::mutex mutex;
	int nextid = 1;
};

#endif

class OpenALDevice : public AudioDevice
{
	// TODO:
};