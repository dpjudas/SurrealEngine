
#include "Precomp.h"
#include "audioplayer.h"
#include "audiosource.h"
#include <stdexcept>
#include <thread>
#include <mutex>
#include <algorithm>

#define WIN32_MEAN_AND_LEAN
#include <Windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <mmreg.h>
#undef min
#undef max

class AudioPlayerImpl : public AudioPlayer
{
public:
	AudioPlayerImpl(std::unique_ptr<AudioSource> audiosource) : source(std::move(audiosource))
	{
		CoInitialize(0);

		wait_timeout = mixing_latency * 2;

		IMMDeviceEnumerator* device_enumerator = nullptr;
		HRESULT result = CoCreateInstance(__uuidof(MMDeviceEnumerator), 0, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&device_enumerator);
		if (FAILED(result))
			throw std::runtime_error("Unable to create IMMDeviceEnumerator instance");

		result = device_enumerator->GetDefaultAudioEndpoint(eRender, eMultimedia, &mmdevice);
		device_enumerator->Release();
		if (FAILED(result))
			throw std::runtime_error("IDeviceEnumerator.GetDefaultAudioEndpoint failed");

		result = mmdevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, 0, (void**)&audio_client);
		if (FAILED(result))
		{
			mmdevice->Release();
			throw std::runtime_error("IMMDevice.Activate failed");
		}

		WAVEFORMATEXTENSIBLE wave_format;
		wave_format.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
		wave_format.Format.nChannels = 2;
		wave_format.Format.nBlockAlign = 2 * sizeof(float);
		wave_format.Format.wBitsPerSample = 8 * sizeof(float);
		wave_format.Format.cbSize = 22;
		wave_format.Samples.wValidBitsPerSample = wave_format.Format.wBitsPerSample;
		wave_format.dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT;
		wave_format.SubFormat = KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;

		wave_format.Format.nSamplesPerSec = mixing_frequency;
		wave_format.Format.nAvgBytesPerSec = wave_format.Format.nSamplesPerSec * wave_format.Format.nBlockAlign;

		WAVEFORMATEX* closest_match = 0;
		result = audio_client->IsFormatSupported(AUDCLNT_SHAREMODE_SHARED, (WAVEFORMATEX*)&wave_format, &closest_match);
		if (FAILED(result))
		{
			audio_client->Release();
			mmdevice->Release();
			audio_client = nullptr;
			mmdevice = nullptr;
			throw std::runtime_error("IAudioClient.IsFormatSupported failed");
		}

		// We could not get the exact format we wanted. Try to use the frequency that the closest matching format is using:
		if (result == S_FALSE)
		{
			mixing_frequency = closest_match->nSamplesPerSec;
			wave_format.Format.nSamplesPerSec = mixing_frequency;
			wave_format.Format.nAvgBytesPerSec = wave_format.Format.nSamplesPerSec * wave_format.Format.nBlockAlign;

			CoTaskMemFree(closest_match);
			closest_match = 0;
		}

		result = audio_client->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_EVENTCALLBACK, mixing_latency * (REFERENCE_TIME)1000, 0, (WAVEFORMATEX*)&wave_format, 0);
		if (FAILED(result))
		{
			audio_client->Release();
			mmdevice->Release();
			audio_client = nullptr;
			mmdevice = nullptr;
			throw std::runtime_error("IAudioClient.Initialize failed");
		}

		result = audio_client->GetService(__uuidof(IAudioRenderClient), (void**)&audio_render_client);
		if (FAILED(result))
		{
			audio_client->Release();
			mmdevice->Release();
			audio_client = nullptr;
			mmdevice = nullptr;
			throw std::runtime_error("IAudioClient.GetService(IAudioRenderClient) failed");
		}

		audio_buffer_ready_event = CreateEvent(0, TRUE, TRUE, 0);
		if (audio_buffer_ready_event == INVALID_HANDLE_VALUE)
		{
			audio_render_client->Release();
			audio_client->Release();
			mmdevice->Release();
			audio_render_client = nullptr;
			audio_client = nullptr;
			mmdevice = nullptr;
			throw std::runtime_error("CreateEvent failed");
		}

		result = audio_client->SetEventHandle(audio_buffer_ready_event);
		if (FAILED(result))
		{
			audio_render_client->Release();
			audio_client->Release();
			mmdevice->Release();
			audio_render_client = nullptr;
			audio_client = nullptr;
			mmdevice = nullptr;
			throw std::runtime_error("IAudioClient.SetEventHandle failed");
		}

		result = audio_client->GetBufferSize(&fragment_size);
		if (FAILED(result))
		{
			audio_render_client->Release();
			audio_client->Release();
			mmdevice->Release();
			audio_render_client = nullptr;
			audio_client = nullptr;
			mmdevice = nullptr;
			throw std::runtime_error("IAudioClient.GetBufferSize failed");
		}

		if (source->GetFrequency() != mixing_frequency)
		{
			source = AudioSource::CreateResampler(mixing_frequency, std::move(source));
		}

		next_fragment = new float[2 * fragment_size];
		start_mixer_thread();
	}

	~AudioPlayerImpl()
	{
		if (audio_render_client)
		{
			stop_mixer_thread();
			if (is_playing)
				audio_client->Stop();
			audio_render_client->Release();
			audio_client->Release();
			mmdevice->Release();
			CloseHandle(audio_buffer_ready_event);
			delete[] next_fragment;
			audio_render_client = nullptr;
			audio_client = nullptr;
			mmdevice = nullptr;
			audio_buffer_ready_event = INVALID_HANDLE_VALUE;
			next_fragment = nullptr;
		}
	}

	void mix_fragment()
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

	void write_fragment()
	{
		UINT32 write_pos = 0;
		while (write_pos < fragment_size)
		{
			WaitForSingleObject(audio_buffer_ready_event, wait_timeout);

			UINT32 num_padding_frames = 0;
			audio_client->GetCurrentPadding(&num_padding_frames);

			UINT32 buffer_available = fragment_size - num_padding_frames;
			UINT32 buffer_needed = fragment_size - write_pos;

			if (buffer_available < buffer_needed)
				ResetEvent(audio_buffer_ready_event);

			UINT32 buffer_size = std::min(buffer_needed, buffer_available);
			if (buffer_size > 0)
			{
				BYTE* buffer = 0;
				HRESULT result = audio_render_client->GetBuffer(buffer_size, &buffer);
				if (SUCCEEDED(result))
				{
					memcpy(buffer, next_fragment + write_pos * 2, sizeof(float) * 2 * buffer_size);
					result = audio_render_client->ReleaseBuffer(buffer_size, 0);

					if (!is_playing)
					{
						result = audio_client->Start();
						if (SUCCEEDED(result))
							is_playing = true;
					}
				}

				write_pos += buffer_size;
			}
		}
	}

	void mixer_thread_main()
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

	void start_mixer_thread()
	{
		std::unique_lock<std::mutex> lock(mixer_mutex);
		mixer_stop_flag = false;
		lock.unlock();
		mixer_thread = std::thread([this]() { mixer_thread_main(); });
	}

	void stop_mixer_thread()
	{
		std::unique_lock<std::mutex> lock(mixer_mutex);
		mixer_stop_flag = true;
		lock.unlock();
		mixer_thread.join();
	}

	std::unique_ptr<AudioSource> source;

	IMMDevice* mmdevice = nullptr;
	IAudioClient* audio_client = nullptr;
	IAudioRenderClient* audio_render_client = nullptr;
	HANDLE audio_buffer_ready_event = INVALID_HANDLE_VALUE;
	bool is_playing = false;
	UINT32 fragment_size = 0;
	int wait_timeout = 0;
	float* next_fragment = nullptr;

	int mixing_frequency = 48000;
	int mixing_latency = 50;

	std::thread mixer_thread;
	std::mutex mixer_mutex;
	bool mixer_stop_flag = false;
};

std::unique_ptr<AudioPlayer> AudioPlayer::Create(std::unique_ptr<AudioSource> source)
{
	return std::make_unique<AudioPlayerImpl>(std::move(source));
}
