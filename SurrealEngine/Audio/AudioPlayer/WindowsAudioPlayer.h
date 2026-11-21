#pragma once

#include "AudioPlayer.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <mmreg.h>

class WindowsAudioPlayer : public AudioPlayer
{
public:
	WindowsAudioPlayer(std::unique_ptr<AudioSource> audiosource);
	~WindowsAudioPlayer();

	int GetMixingFrequency() const override { return mixing_frequency; }

private:
	void mix_fragment();
	void write_fragment();
	void mixer_thread_main();
	void start_mixer_thread();
	void stop_mixer_thread();

	std::unique_ptr<AudioSource> source;

	IMMDevice* mmdevice = nullptr;
	IAudioClient* audio_client = nullptr;
	IAudioRenderClient* audio_render_client = nullptr;
	HANDLE audio_buffer_ready_event = INVALID_HANDLE_VALUE;
	bool is_playing = false;
	UINT32 fragment_size = 0;
	int wait_timeout = 0;
	float* next_fragment = nullptr;
	int channels = 0;

	int mixing_frequency = 48000;
	int mixing_latency = 50;

	std::thread mixer_thread;
	std::mutex mixer_mutex;
	bool mixer_stop_flag = false;
};
