
#include "Precomp.h"
#if !defined(WIN32) && !defined(__APPLE__)
#include "AlsaAudioPlayer.h"
#include "../AudioSource.h"
#include <stdexcept>
#include <thread>
#include <mutex>
#include <algorithm>

AlsaAudioPlayer::AlsaAudioPlayer(std::unique_ptr<AudioSource> audiosource) : source(std::move(audiosource))
{
	int rc = snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0);
	if (rc < 0)
	{
		handle = nullptr;
		throw std::runtime_error("Could not open sound device");
	}

	snd_pcm_hw_params_t* hwparams;
	snd_pcm_hw_params_alloca(&hwparams);
	snd_pcm_hw_params_any(handle, hwparams);
	snd_pcm_hw_params_set_access(handle, hwparams, SND_PCM_ACCESS_RW_INTERLEAVED);
	snd_pcm_hw_params_set_format(handle, hwparams, SND_PCM_FORMAT_FLOAT);
	snd_pcm_hw_params_set_channels(handle, hwparams, 2);
	snd_pcm_hw_params_set_rate_near(handle, hwparams, (unsigned int*)&mixing_frequency, nullptr);

	frames_in_buffer = mixing_latency * mixing_frequency / 1000;
	snd_pcm_hw_params_set_buffer_size_near(handle, hwparams, &frames_in_buffer);
	frames_in_period = frames_in_buffer / 4;
	snd_pcm_hw_params_set_period_size_near(handle, hwparams, &frames_in_period, nullptr);

	rc = snd_pcm_hw_params(handle, hwparams);
	if (rc < 0)
	{
		snd_pcm_close(handle);
		handle = nullptr;
		throw std::runtime_error("Could not initialize sound device");
	}

	snd_pcm_hw_params_get_period_size(hwparams, &frames_in_period, nullptr);

	fragment_size = frames_in_period;

	if (source->GetFrequency() != mixing_frequency)
	{
		source = AudioSource::CreateResampler(mixing_frequency, std::move(source));
	}

	next_fragment = new float[2 * fragment_size];
	start_mixer_thread();
}

AlsaAudioPlayer::~AlsaAudioPlayer()
{
	stop_mixer_thread();

	delete[] next_fragment;
	next_fragment = nullptr;

	if (handle)
	{
		snd_pcm_close(handle);
		handle = nullptr;
	}
}

void AlsaAudioPlayer::mix_fragment()
{
	if (source)
	{
		size_t count = (size_t)fragment_size * 2;
		float* output = next_fragment;

		int channels = source->GetChannels();
		if (channels == 2)
		{
			size_t samplesread = source->ReadSamples(output, count);
			output += samplesread;
			count -= samplesread;
		}
		else if (channels == 1)
		{
			size_t samplesread = source->ReadSamples(output, count / 2);
			for (size_t i = 0; i < samplesread; i++)
			{
				size_t src = samplesread - 1 - i;
				size_t dst = src * 2;
				output[dst] = output[dst + 1] = output[src];
			}
			output += samplesread * 2;
			count -= samplesread * 2;
		}

		while (count > 0)
		{
			*(output++) = 0.0f;
			count--;
		}
	}
}

void AlsaAudioPlayer::write_fragment()
{
	switch (snd_pcm_state(handle))
	{
	case SND_PCM_STATE_XRUN:
	case SND_PCM_STATE_SUSPENDED:
		snd_pcm_prepare(handle);
		break;
	case SND_PCM_STATE_PAUSED:
		snd_pcm_pause(handle, 0);
		break;
	default:
		break;
	}

	int result = snd_pcm_writei(handle, next_fragment, frames_in_period);
	if (result < 0)
	{
		std::unique_lock<std::mutex> lock(mixer_mutex);
		mixer_stop_flag = true;
	}
}

void AlsaAudioPlayer::mixer_thread_main()
{
	std::unique_lock<std::mutex> lock(mixer_mutex);
	while (!mixer_stop_flag)
	{
		lock.unlock();
		mix_fragment();
		write_fragment();
		lock.lock();
	}
}

void AlsaAudioPlayer::start_mixer_thread()
{
	std::unique_lock<std::mutex> lock(mixer_mutex);
	mixer_stop_flag = false;
	lock.unlock();
	mixer_thread = std::thread([this]() { mixer_thread_main(); });
}

void AlsaAudioPlayer::stop_mixer_thread()
{
	std::unique_lock<std::mutex> lock(mixer_mutex);
	mixer_stop_flag = true;
	lock.unlock();
	mixer_thread.join();
}
#endif
