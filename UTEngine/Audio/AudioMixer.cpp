
#include "Precomp.h"
#include "AudioMixer.h"
#include "AudioSource.h"
#include "AudioPlayer.h"
#include <mutex>
#include <stdexcept>
#include <map>
#include <cmath>

class AudioMixerImpl;

class AudioSound
{
public:
	AudioSound(int mixing_frequency, std::unique_ptr<AudioSource> source, const AudioLoopInfo& inloopinfo) : loopinfo(inloopinfo)
	{
		duration = samples.size() / 48000.0f;

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

class AudioMixerSource : public AudioSource
{
public:
	AudioMixerSource(AudioMixerImpl* mixer) : mixer(mixer) { }

	int GetFrequency() override;
	int GetChannels() override { return 2; }
	int GetSamples() override { return -1; }
	void SeekToSample(uint64_t position) override { }
	size_t ReadSamples(float* output, size_t samples) override;

	void TransferFromClient();
	void CopyMusic(float* output, size_t samples);
	void MixSounds(float* output, size_t samples);

	void GetVolume(const ActiveSound& sound, float& leftVolume, float& rightVolume);

	AudioMixerImpl* mixer = nullptr;
	std::map<int, ActiveSound> sounds;
	std::vector<int> stoppedsounds;
	std::unique_ptr<AudioSource> music;
	float soundvolume = 1.0f;
	float musicvolume = 1.0f;
};

class AudioMixerImpl : public AudioMixer
{
public:
	AudioMixerImpl()
	{
		player = AudioPlayer::Create(std::make_unique<AudioMixerSource>(this));
	}

	~AudioMixerImpl()
	{
		player.reset();
	}

	AudioSound* AddSound(std::unique_ptr<AudioSource> source, const AudioLoopInfo& loopinfo) override
	{
		sounds.push_back(std::make_unique<AudioSound>(mixing_frequency, std::move(source), loopinfo));
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
		if (source && source->GetChannels() != 2)
			throw std::runtime_error("Only stereo music is supported");

		if (source && source->GetFrequency() != mixing_frequency)
		{
			source = AudioSource::CreateResampler(mixing_frequency, std::move(source));
		}
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

	int mixing_frequency = 48000;

	std::vector<std::unique_ptr<AudioSound>> sounds;
	std::unique_ptr<AudioPlayer> player;
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

/////////////////////////////////////////////////////////////////////////////

std::unique_ptr<AudioMixer> AudioMixer::Create()
{
	return std::make_unique<AudioMixerImpl>();
}

/////////////////////////////////////////////////////////////////////////////

int AudioMixerSource::GetFrequency()
{
	return mixer->mixing_frequency;
}

size_t AudioMixerSource::ReadSamples(float* output, size_t samples)
{
	TransferFromClient();
	CopyMusic(output, samples);
	MixSounds(output, samples);
	return samples;
}

void AudioMixerSource::TransferFromClient()
{
	std::unique_lock<std::mutex> lock(mixer->mutex);
	for (ActiveSound& s : mixer->transfer.sounds)
	{
		if (!s.update) // if play or stop
		{
			auto it = sounds.find(s.channel);
			if (it != sounds.end())
			{
				stoppedsounds.push_back(s.channel);
				sounds.erase(it);
			}
		}

		if (s.play)
		{
			sounds[s.channel] = s;
		}
		else if (s.update)
		{
			auto it = sounds.find(s.channel);
			if (it != sounds.end())
			{
				it->second.volume = s.volume;
				it->second.pan = s.pan;
				it->second.pitch = s.pitch;
			}
		}
	}
	mixer->transfer.sounds.clear();

	if (mixer->transfer.musicupdate)
		music = std::move(mixer->transfer.music);
	mixer->transfer.musicupdate = false;

	mixer->channelstopped.insert(mixer->channelstopped.end(), stoppedsounds.begin(), stoppedsounds.end());
	stoppedsounds.clear();

	soundvolume = mixer->transfer.soundvolume;
	musicvolume = mixer->transfer.musicvolume;
}

void AudioMixerSource::CopyMusic(float* output, size_t samples)
{
	size_t pos = 0;

	if (music && music->GetChannels() == 2) // Only support stereo music for now
	{
		pos = music->ReadSamples(output, samples);
	}

	for (size_t i = pos; i < samples; i++)
	{
		output[i] = 0.0f;
	}

	for (size_t i = 0; i < samples; i++)
	{
		output[i] *= musicvolume;
	}
}

void AudioMixerSource::MixSounds(float* output, size_t samples)
{
	samples /= 2;

	auto it = sounds.begin();
	while (it != sounds.end())
	{
		ActiveSound& sound = it->second;
		const float* src = sound.sound->samples.data();
		double srcpos = sound.pos;
		double srcpitch = sound.pitch;
		int srcsize = (int)sound.sound->samples.size();
		int srcmax = srcsize - 1;
		if (srcmax < 0)
		{
			stoppedsounds.push_back(sound.channel);
			it = sounds.erase(it);
			continue;
		}

		float leftVolume, rightVolume;
		GetVolume(sound, leftVolume, rightVolume);

		if (sound.sound->loopinfo.Looped)
		{
			double loopStart = (double)sound.sound->loopinfo.LoopStart;
			double loopEnd = (double)sound.sound->loopinfo.LoopEnd;
			double loopLen = loopEnd - loopStart;

			for (size_t i = 0; i < samples; i++)
			{
				double p0 = srcpos;
				double p1 = srcpos + 1;
				if (p0 >= loopEnd) p0 -= loopLen;
				if (p1 >= loopEnd) p1 -= loopLen;

				int pos = (int)p0;
				int pos2 = (int)p1;
				float t = (float)(p0 - pos);

				pos = std::min(pos, srcmax);
				pos2 = std::min(pos2, srcmax);

				float value = (src[pos] * t + src[pos2] * (1.0f - t));
				output[i << 1] += value * leftVolume;
				output[(i << 1) + 1] += value * rightVolume;

				srcpos += srcpitch;
				while (srcpos >= loopEnd)
					srcpos -= loopLen;
			}

			sound.pos = srcpos;
			++it;
		}
		else
		{
			if (leftVolume > 0.0f || rightVolume > 0.0f)
			{
				for (size_t i = 0; i < samples; i++)
				{
					int pos = (int)srcpos;
					int pos2 = pos + 1;
					float t = (float)(srcpos - pos);

					pos = std::min(pos, srcmax);
					pos2 = std::min(pos2, srcmax);

					float value = (src[pos] * t + src[pos2] * (1.0f - t));
					output[i << 1] += value * leftVolume;
					output[(i << 1) + 1] += value * rightVolume;

					srcpos += srcpitch;
				}
			}
			else
			{
				srcpos += srcpitch * samples;
			}

			if (srcpos < sound.sound->samples.size())
			{
				sound.pos = srcpos;
				++it;
			}
			else
			{
				stoppedsounds.push_back(sound.channel);
				it = sounds.erase(it);
			}
		}
	}
}

void AudioMixerSource::GetVolume(const ActiveSound& sound, float& leftVolume, float& rightVolume)
{
	auto clamp = [](float x, float minval, float maxval) { return std::max(std::min(x, maxval), minval); };
	leftVolume = clamp(sound.volume * std::min(1.0f - sound.pan, 1.0f) * soundvolume, 0.0f, 1.0f);
	rightVolume = clamp(sound.volume * std::min(1.0f + sound.pan, 1.0f) * soundvolume, 0.0f, 1.0f);
}
