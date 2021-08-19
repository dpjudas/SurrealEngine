
#include "Precomp.h"
#include "AudioMixer.h"
#include "AudioSource.h"
#include "AudioPlayer.h"
#include <mutex>
#include <stdexcept>

class AudioMixerImpl;

class AudioSound
{
public:
	AudioSound(int mixing_frequency, std::unique_ptr<AudioSource> source)
	{
		duration = samples.size() / 48000.0f;

		if (source->GetChannels() == 1)
		{
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
		}
		else
		{
			throw std::runtime_error("Only mono sounds are supported");
		}
	}

	std::vector<float> samples;
	float duration = 0.0f;
};

class ActiveSound
{
public:
	void* owner = nullptr;
	int slot = 0;
	AudioSound* sound = nullptr;
	vec3 location = vec3(0.0f);
	float volume = 0.0f;
	float radius = 0.0f;
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
	std::map<std::pair<void*, int>, ActiveSound> sounds;
	mat4 viewport = mat4::identity();
	std::unique_ptr<AudioSource> music;
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

	AudioSound* AddSound(std::unique_ptr<AudioSource> source) override
	{
		sounds.push_back(std::make_unique<AudioSound>(mixing_frequency, std::move(source)));
		return sounds.back().get();
	}

	float GetSoundDuration(AudioSound* sound) override
	{
		return sound->duration;
	}

	void PlaySound(void* owner, int slot, AudioSound* sound, const vec3& location, float volume, float radius, float pitch) override
	{
		ActiveSound s;
		s.owner = owner;
		s.slot = slot;
		s.sound = sound;
		s.location = location;
		s.volume = volume;
		s.radius = radius;
		s.pitch = pitch;
		client.sounds.push_back(std::move(s));
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

	void SetViewport(const mat4& worldToView) override
	{
		client.viewport = worldToView;
	}

	void Update() override
	{
		std::unique_lock<std::mutex> lock(mutex);
		for (ActiveSound& s : client.sounds)
			transfer.sounds.push_back(s);
		client.sounds.clear();
		transfer.viewport = client.viewport;
		if (client.musicupdate)
		{
			transfer.music = std::move(client.music);
			transfer.musicupdate = true;
		}
		client.music = {};
		client.musicupdate = false;
	}

	int mixing_frequency = 48000;

	std::vector<std::unique_ptr<AudioSound>> sounds;
	std::unique_ptr<AudioPlayer> player;

	struct
	{
		std::vector<ActiveSound> sounds;
		mat4 viewport;
		std::unique_ptr<AudioSource> music;
		bool musicupdate = false;
	} client, transfer;

	std::mutex mutex;
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
		sounds[{ s.owner, s.slot }] = s;
	mixer->transfer.sounds.clear();
	viewport = mixer->transfer.viewport;
	if (mixer->transfer.musicupdate)
		music = std::move(mixer->transfer.music);
	mixer->transfer.musicupdate = false;
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
}

void AudioMixerSource::MixSounds(float* output, size_t samples)
{
	samples /= 2;

	for (auto it = sounds.begin(); it != sounds.end();)
	{
		ActiveSound& sound = it->second;
		const float* src = sound.sound->samples.data();
		double srcpos = sound.pos;
		double srcpitch = sound.pitch;
		int srcsize = (int)sound.sound->samples.size();
		int srcmax = srcsize - 1;
		if (srcmax < 0)
			continue;

		float leftVolume, rightVolume;
		GetVolume(sound, leftVolume, rightVolume);

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
			it = sounds.erase(it);
		}
	}
}

void AudioMixerSource::GetVolume(const ActiveSound& sound, float& leftVolume, float& rightVolume)
{
	if (sound.radius > 0.0f)
	{
		vec3 location = (viewport * vec4(sound.location, 1.0f)).xyz();

		// To do: use a less shitty HRTF equation than this 1990's version!!

		float angle = std::atan2(location.x, std::abs(location.z));

		// Reduce spatialization effect when getting close to the center
		float despatializedRadius = sound.radius * 0.1f;
		float dist2 = dot(location, location);
		if (dist2 < despatializedRadius * despatializedRadius)
			angle *= std::sqrt(dist2) / despatializedRadius;

		float pan = clamp(angle * 2.0f / 3.14f, -1.0f, 1.0f);
		float attenuation = std::max(1.0f - length(location) / sound.radius, 0.0f);
		attenuation *= attenuation;

		leftVolume = clamp(sound.volume * attenuation * std::min(1.0f - pan, 1.0f), 0.0f, 1.0f);
		rightVolume = clamp(sound.volume * attenuation * std::min(1.0f + pan, 1.0f), 0.0f, 1.0f);
	}
	else
	{
		leftVolume = clamp(sound.volume, 0.0f, 1.0f);
		rightVolume = clamp(sound.volume, 0.0f, 1.0f);
	}
}
