
#include "Precomp.h"
#include "AudioMixerSource.h"
#include "AudioMixerImpl.h"
#include "AudioSound.h"

AudioMixerSource::AudioMixerSource(AudioMixerImpl* mixer) : mixer(mixer)
{
}

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
		while (pos < samples)
		{
			size_t samplesRead = music->ReadSamples(output + pos, samples - pos);
			if (samplesRead == 0)
				break;
			pos += samplesRead;
		}
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
