
#include "Precomp.h"
#include "AudioSound.h"
#include "Audio/AudioSource.h"

AudioSound::AudioSound(int mixing_frequency, std::unique_ptr<AudioSource> source, const AudioLoopInfo& inloopinfo)
	: loopinfo(inloopinfo)
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
		throw std::runtime_error("Only mono sounds are supported");
	}
}
