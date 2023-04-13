
#include "Precomp.h"
#include "AudioDevice.h"
#include "AudioSource.h"
#include "UObject/USound.h"
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
	USound* sound = nullptr;
	float volume = 0.0f;
	float pan = 0.0f;
	float pitch = 0.0f;

	double pos = 0;
};

#ifdef WIN32

#define WIN32_MEAN_AND_LEAN
#include <Windows.h>
#include <xaudio2.h>
#include <x3daudio.h>

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

		// Setup music wave format, floating point PCM, 2-channel
		WAVEFORMATEX musicFormat;
		musicFormat.wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
		musicFormat.nChannels = 2;
		musicFormat.nSamplesPerSec = frequency;
		musicFormat.nAvgBytesPerSec = frequency * 4 * 2; // frequency * (wBitsPerSample / 8) * nChannels 
		musicFormat.nBlockAlign = 8; // (nChannels * wBitsPerSample) / 8
		musicFormat.wBitsPerSample = 32;
		musicFormat.cbSize = 0;

		if (FAILED(hr = xaudio2->CreateSourceVoice(&musicVoice, &musicFormat)))
			throw std::runtime_error("Failed to create source voice for music");

		musicbuffer = new float[512];

		// Setup wave format, floating point PCM, mono channel for now
		WAVEFORMATEX voiceFormat;
		voiceFormat.wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
		voiceFormat.nChannels = 1; // TODO: Support stereo sounds as well
		voiceFormat.nSamplesPerSec = frequency;
		voiceFormat.nAvgBytesPerSec = frequency * 4; // frequency * (wBitsPerSample / 8) * nChannels 
		voiceFormat.nBlockAlign = 4; // (nChannels * wBitsPerSample) / 8
		voiceFormat.wBitsPerSample = 32;
		voiceFormat.cbSize = 0;

		// Point source voices to submix for volume control, global effects, etc
		XAUDIO2_SEND_DESCRIPTOR sendDesc;
		sendDesc.Flags = 0;
		sendDesc.pOutputVoice = soundSubmixVoice;

		XAUDIO2_VOICE_SENDS voiceSends;
		voiceSends.SendCount = 1;
		voiceSends.pSends = &sendDesc;

		voices.reserve(numVoices);
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
		xaudio2->StopEngine();
		xaudio2->Release();
		delete musicbuffer;
	}

	void AddSound(USound* sound) override
	{
		sounds.push_back(sound);
	}

	void RemoveSound(USound* sound) override
	{
	}

	int PlaySound(int channel, USound* sound, float volume, float pan, float pitch) override
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
		//activeSounds.push_back(std::move(s));

		IXAudio2SourceVoice* voice = voices[channel];
		SetupPanning(voice, s);
		voice->SetFrequencyRatio(s.pitch);
		voice->SetVolume(s.volume);

		XAUDIO2_VOICE_DETAILS voiceDetails;
		voice->GetVoiceDetails(&voiceDetails);

		if (s.sound->frequency != voiceDetails.InputSampleRate)
			voice->SetSourceSampleRate(s.sound->frequency);

		XAUDIO2_BUFFER buf;
		buf.PlayBegin = 0;
		buf.PlayLength = s.sound->samples.size();
		buf.AudioBytes = buf.PlayLength * s.sound->frequency * 4;
		buf.Flags = XAUDIO2_END_OF_STREAM;
		buf.pAudioData = reinterpret_cast<BYTE*>(s.sound->samples.data());
		buf.pContext = nullptr;

		AudioLoopInfo& loopinfo = s.sound->loopinfo;
		if (loopinfo.Looped)
		{
			buf.LoopBegin = loopinfo.LoopStart;
			buf.LoopLength = loopinfo.LoopEnd;
			buf.LoopCount = XAUDIO2_LOOP_INFINITE;
		}
		else
		{
			buf.LoopBegin = buf.LoopLength = buf.LoopCount = 0;
		}

		// TODO: this is getting called too much, causing sound to not play correctly
		voice->SubmitSourceBuffer(&buf);
		voice->Start();

		return channel;
	}

	void UpdateSound(int channel, USound* sound, float volume, float pan, float pitch) override
	{
		if (!std::isfinite(volume) || !std::isfinite(pan) || !std::isfinite(pitch) || channel >= voices.size())
			throw std::runtime_error("Invalid UpdateSound arguments");

		ActiveSound s;
		s.channel = channel;
		s.play = false;
		s.update = true;
		s.sound = sound;
		s.volume = volume;
		s.pan = pan;
		s.pitch = pitch;

		IXAudio2SourceVoice* voice = voices[channel];
		SetupPanning(voice, s);
		voice->SetFrequencyRatio(s.pitch);
		voice->SetVolume(s.volume);
	}

	void StopSound(int channel) override
	{
		if (channel >= voices.size())
			throw std::runtime_error("Invalid StopSound arguments");

		voices[channel]->Stop();
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
		soundSubmixVoice->SetVolume(volume);
	}

	void SetupPanning(IXAudio2SourceVoice* voice, ActiveSound& s)
	{
		float outputMatrix[8] = { 0 };

		// TODO: Proper surround sound implementation
		float left = 0.5 - s.pan / 2;
		float right = 0.5 + s.pan / 2;

		DWORD channelMask = 0;
		masterVoice->GetChannelMask(&channelMask);

		switch (channelMask)
		{
		case SPEAKER_MONO:
			outputMatrix[0] = 1.0;
			break;
		case SPEAKER_STEREO:
		case SPEAKER_2POINT1:
		case SPEAKER_SURROUND:
			outputMatrix[0] = left;
			outputMatrix[1] = right;
			break;
		case SPEAKER_QUAD:
			outputMatrix[0] = outputMatrix[2] = left;
			outputMatrix[1] = outputMatrix[3] = right;
			break;
		case SPEAKER_4POINT1:
			outputMatrix[0] = outputMatrix[3] = left;
			outputMatrix[1] = outputMatrix[4] = right;
			break;
		case SPEAKER_5POINT1:
		case SPEAKER_7POINT1:
		case SPEAKER_5POINT1_SURROUND:
			outputMatrix[0] = outputMatrix[4] = left;
			outputMatrix[1] = outputMatrix[5] = right;
			break;
		case SPEAKER_7POINT1_SURROUND:
			outputMatrix[0] = outputMatrix[4] = outputMatrix[6] = left;
			outputMatrix[1] = outputMatrix[5] = outputMatrix[7] = right;
			break;
		}

		XAUDIO2_VOICE_DETAILS masterDetails;
		masterVoice->GetVoiceDetails(&masterDetails);
		voice->SetOutputMatrix(NULL, 2, masterDetails.InputChannels, outputMatrix);
	}

	void UpdateMusic()
	{
		// TODO: this whole thing needs to be re-done, we need to use multiple
		// buffers to read music in and use OnBufferEnd callbacks to signify when
		// a buffer is free

		//music->ReadSamples(musicbuffer, 512);
		//
		//XAUDIO2_BUFFER buf;
		//buf.Flags = 0;
		//buf.AudioBytes = 4 * 512 * music->GetFrequency();
		//buf.pAudioData = reinterpret_cast<BYTE*>(musicbuffer);
		//buf.PlayBegin = 0;
		//buf.PlayLength = 0;
		//buf.LoopBegin = 0;
		//buf.LoopLength = 0;
		//buf.LoopCount = 0;
		//buf.pContext = nullptr;
		//
		//musicVoice->SubmitSourceBuffer(&buf);
	}

	void Update() override
	{
		if (musicupdate)
		{
			musicVoice->Stop();
			if (music)
			{
				XAUDIO2_VOICE_DETAILS musicDetails;
				musicVoice->GetVoiceDetails(&musicDetails);

				uint32_t frequency = music->GetFrequency();
				if (frequency != musicDetails.InputSampleRate)
					musicVoice->SetSourceSampleRate(frequency);

				musicVoice->Start();
			}
		}

		if (music)
			UpdateMusic();
	}

	std::vector<USound*> sounds;
	std::vector<IXAudio2SourceVoice*> voices;
	std::vector<ActiveSound> activeSounds;
	std::unique_ptr<AudioSource> music;
	bool musicupdate = false;
	float* musicbuffer = nullptr;

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

std::unique_ptr<AudioDevice> AudioDevice::Create(int frequency, int numVoices)
{
	return std::make_unique<XAudio2AudioDevice>(frequency, numVoices);
}