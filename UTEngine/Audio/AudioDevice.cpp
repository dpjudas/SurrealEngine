
#include "Precomp.h"
#include "AudioDevice.h"
#include "AudioSource.h"
#include <mutex>
#include <stdexcept>
#include <map>
#include <cmath>

class AudioDeviceImpl;

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

AudioSound::AudioSound(std::unique_ptr<AudioSource> source, const AudioLoopInfo& inloopinfo) : loopinfo(inloopinfo)
{
	frequency = source->GetFrequency();
	duration = samples.size() / (float)frequency;

	if (source->GetChannels() == 1)
	{
		// XXX: resize twice??
		samples.resize(source->GetSamples());
		samples.resize(source->ReadSamples(samples.data(), samples.size()));

		int sampleCount = source->GetSamples();
		if (sampleCount > 0)
		{
			loopinfo.LoopStart = std::max(std::min(loopinfo.LoopStart, (uint64_t)sampleCount - 1), (uint64_t)0);
			loopinfo.LoopEnd = std::max(std::min(loopinfo.LoopEnd, (uint64_t)sampleCount), (uint64_t)0);
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

#ifdef WIN32

#define WIN32_MEAN_AND_LEAN
#include <Windows.h>
#include <xaudio2.h>

class XAudio2AudioDevice : public AudioDevice
{
public:
	XAudio2AudioDevice(int inFrequency, int numVoices)
	{
		frequency = inFrequency;

		// init XAudio2
		HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
		if (FAILED(hr))
			throw std::runtime_error("CoInitializeEx failed");

		if (FAILED(hr = XAudio2Create(&xaudio2, 0, XAUDIO2_DEFAULT_PROCESSOR)))
			throw std::runtime_error("Failed to create XAudio2 instance");

		if (FAILED(hr = xaudio2->CreateMasteringVoice(&masterVoice)))
			throw std::runtime_error("Failed to create XAudio2 mastering voice");

		if (FAILED(hr = xaudio2->CreateSubmixVoice(&soundSubmixVoice, 1, frequency)))
			throw std::runtime_error("Failed to create XAudio2 submix voice");

		// Setup wave format, floating point PCM, mono channel for now
		WAVEFORMATEX voiceFormat;
		voiceFormat.wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
		voiceFormat.nChannels = 1; // TODO: Support stereo sounds as well
		voiceFormat.nSamplesPerSec = frequency;
		voiceFormat.cbSize = 0;

		// Point source voices to submix for volume control, global effects, etc
		XAUDIO2_SEND_DESCRIPTOR sendDesc;
		sendDesc.Flags = 0;
		sendDesc.pOutputVoice = soundSubmixVoice;

		XAUDIO2_VOICE_SENDS voiceSends;
		voiceSends.SendCount = 1;
		voiceSends.pSends = &sendDesc;

		voices.resize(numVoices);
		for (int i = 0; i < numVoices; i++)
		{
			// TODO: probably need to make use of the effect chain to implement different audio effects
			IXAudio2SourceVoice* voice;
			hr = xaudio2->CreateSourceVoice
			(
				&voice,
				&voiceFormat, 
				0, 2.0f, 0,
				&voiceSends
			);

			if (FAILED(hr))
				throw std::runtime_error("Failed to create source voices");

			voices.push_back(voice);
		}
	}

	~XAudio2AudioDevice()
	{
		musicVoice->DestroyVoice();
		masterVoice->DestroyVoice();
		xaudio2->Release();
	}

	AudioSound* AddSound(std::unique_ptr<AudioSource> source, const AudioLoopInfo& loopinfo) override
	{
		sounds.push_back(std::make_unique<AudioSound>(std::move(source), loopinfo));
		return sounds.back().get();
	}

	void RemoveSound(AudioSound* sound) override
	{
	}

	int PlaySound(int channel, AudioSound* sound, float volume, float pan, float pitch) override
	{
		if (!std::isfinite(volume) || !std::isfinite(pan) || !std::isfinite(pitch) || channel >= voices.size())
			throw std::runtime_error("Invalid PlaySound arguments");

		ActiveSound s;
		s.channel = channel;
		s.play = true;
		s.update = false;
		s.sound = sound;
		s.volume = volume;
		s.pan = pan;
		s.pitch = pitch;
		activeSounds.push_back(std::move(s));
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
		activeSounds.push_back(std::move(s));
	}

	void StopSound(int channel) override
	{
		ActiveSound s;
		s.channel = channel;
		s.play = false;
		s.update = false;
		activeSounds.push_back(std::move(s));
	}

	bool SoundFinished(int channel) override
	{
		auto it = channelplaying.find(channel);
		return it == channelplaying.end() || it->second == 0;
	}

	void PlayMusic(std::unique_ptr<AudioSource> source) override
	{
		music = std::move(source);
		musicupdate = true;
	}

	void SetMusicVolume(float volume) override
	{
		musicVoice->SetVolume(volume);
	}

	void SetSoundVolume(float volume) override
	{
		// TODO:
	}

	void Update() override
	{
		for (ActiveSound s : activeSounds)
		{
			if (!s.update)
			{
				IXAudio2SourceVoice* voice = voices[s.channel];
				voice->Stop();
			}
		}
	}

	std::vector<std::unique_ptr<AudioSound>> sounds;
	std::vector<IXAudio2SourceVoice*> voices;
	std::vector<ActiveSound> activeSounds;
	std::map<int, int> channelplaying;
	std::unique_ptr<AudioSource> music;
	bool musicupdate = false;

	IXAudio2* xaudio2 = nullptr;
	IXAudio2MasteringVoice* masterVoice = nullptr;
	IXAudio2SubmixVoice* soundSubmixVoice = nullptr;
	IXAudio2SourceVoice* musicVoice = nullptr;

	//int frequency = 48000;

	//struct
	//{
	//	std::vector<ActiveSound> sounds;
	//	std::unique_ptr<AudioSource> music;
	//	bool musicupdate = false;
	//	float soundvolume = 1.0f;
	//	float musicvolume = 1.0f;
	//} client, transfer;
	//std::vector<int> channelstopped;
	//
	//std::mutex mutex;
	//int nextid = 1;
};

#endif
/*
class OpenALAudioDevice : public AudioDevice
{
	OpenALAudioDevice()
	{

	}

	~OpenALAudioDevice()
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
	}

	void PlayMusic(std::unique_ptr<AudioSource> source) override
	{
	}

	void SetMusicVolume(float volume) override
	{
	}

	void SetSoundVolume(float volume) override
	{
	}

	void Update() override
	{

	}
};*/