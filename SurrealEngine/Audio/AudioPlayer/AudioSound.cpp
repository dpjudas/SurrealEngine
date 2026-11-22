
#include "Precomp.h"
#include "AudioSound.h"
#include "Audio/AudioSource.h"

AudioSound::AudioSound(int mixing_frequency, std::unique_ptr<AudioSource> source, bool looped, uint64_t loopStart, uint64_t loopEnd)
{
	if (source->GetChannels() != 1)
		throw std::runtime_error("Only mono sounds are supported");

#if 1 // Seems AudioSource::CreateResampler cuts off the end of samples

	std::vector<float> srcbuf;
	srcbuf.resize(source->GetSamples());
	source->ReadSamples(srcbuf.data(), srcbuf.size());

	double srcpitch = source->GetFrequency() / (double)mixing_frequency;
	samples.resize((size_t)((double)srcbuf.size() / srcpitch) + 1);

	const float* src = srcbuf.data();
	int srcsize = (int)srcbuf.size();
	int srcmax = srcsize - 1;
	double srcpos = 0;

	float* output = samples.data();
	size_t outputsize = samples.size();
	for (size_t i = 0; i < outputsize; i++)
	{
		double p0 = srcpos;
		double p1 = srcpos + 1;

		int pos = (int)p0;
		int pos2 = (int)p1;
		float t = (float)(p0 - pos);

		pos = std::min(pos, srcmax);
		pos2 = std::min(pos2, srcmax);

		output[i] = (src[pos2] * t + src[pos] * (1.0f - t));
		srcpos += srcpitch;
	}

	uint64_t oldSampleCount = srcbuf.size();
	uint64_t newSampleCount = samples.size();
	if (looped && oldSampleCount > 0 && newSampleCount > 0)
	{
		loopinfo.LoopStart = std::max(std::min(loopStart * newSampleCount / oldSampleCount, newSampleCount - 1), (uint64_t)0);
		loopinfo.LoopEnd = std::max(std::min(loopEnd * newSampleCount / oldSampleCount, newSampleCount), (uint64_t)0);
		loopinfo.Looped = looped && loopinfo.LoopStart < loopinfo.LoopEnd;
	}
	else
	{
		loopinfo = {};
	}

#else
	uint64_t oldSampleCount = source->GetSamples();
	if (source->GetFrequency() != mixing_frequency)
	{
		source = AudioSource::CreateResampler(mixing_frequency, std::move(source));
	}

	samples.resize(source->GetSamples() * source->GetChannels());
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
		loopinfo.Looped = looped;
		loopinfo.LoopStart = std::max(std::min(loopStart * newSampleCount / oldSampleCount, newSampleCount - 1), (uint64_t)0);
		loopinfo.LoopEnd = std::max(std::min(loopEnd * newSampleCount / oldSampleCount, newSampleCount), (uint64_t)0);
	}
	else
	{
		loopinfo = {};
	}
#endif

	duration = (float)(samples.size() / source->GetChannels() / (double)mixing_frequency);
}
