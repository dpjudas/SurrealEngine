#pragma once

#include "AudioPlayer.h"
#include <thread>
#include <mutex>
#include <alsa/asoundlib.h>

class AlsaAudioPlayer : public AudioPlayer
{
public:
	AlsaAudioPlayer(std::unique_ptr<AudioSource> audiosource);
	~AlsaAudioPlayer();

	int GetMixingFrequency() const override { return mixing_frequency; }

private:
	void mix_fragment();
	void write_fragment();
	void mixer_thread_main();
	void start_mixer_thread();
	void stop_mixer_thread();

	std::unique_ptr<AudioSource> source;

	snd_pcm_t* handle = nullptr;
	snd_pcm_uframes_t frames_in_period = {};
	snd_pcm_uframes_t frames_in_buffer = {};

	int mixing_frequency = 48000;
	int mixing_latency = 50;

	std::thread mixer_thread;
	std::mutex mixer_mutex;
	bool mixer_stop_flag = false;

	uint32_t fragment_size = 0;
	float* next_fragment = nullptr;
};
