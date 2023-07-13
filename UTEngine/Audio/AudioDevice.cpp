
#include "Precomp.h"
#include "AudioDevice.h"
#include "AudioSource.h"
#include "UObject/USound.h"
#include <mutex>
#include <stdexcept>
#include <map>
#include <cmath>
#include <queue>
#include <thread>
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>

class AudioDeviceImpl;

class OpenALAudioDevice : public AudioDevice
{
public:
	static void startPlaybackThread(OpenALAudioDevice* device)
	{
		device->UpdateMusicLoop();
	}

	OpenALAudioDevice(int inFrequency, int numVoices, int inMusicBufferCount, int inMusicBufferSize)
	{
		frequency = inFrequency;
		bMusicPlaying = false;
		musicBufferCount = inMusicBufferCount;
		musicBufferSize = inMusicBufferSize;

		musicBuffer = new float[musicBufferSize * musicBufferCount];
		musicQueue.Resize(musicBufferCount);

		for (int i = 0; i < musicBufferCount; i++)
		{
			musicQueue.Push(&musicBuffer[musicBufferSize * i]);
			musicQueue.Pop();
		}

		// Init OpenAL
		// TODO: Add device enumeration
		alDevice = alcOpenDevice(NULL);
		if (alDevice == nullptr)
			throw std::runtime_error("Failed to initialize OpenAL device");

		// Create dummy context to determine supported maximum source count
		alContext = alcCreateContext(alDevice, NULL);
		if (alContext == nullptr)
			throw std::runtime_error("Failed to initialize dummy OpenAL context");

		ALCint numAttribs = 0;
		alcGetIntegerv(alDevice, ALC_ATTRIBUTES_SIZE, 1, &numAttribs);

		std::vector<ALCint> attribs(numAttribs);
		alcGetIntegerv(alDevice, ALC_ALL_ATTRIBUTES, numAttribs, &attribs[0]);

		// Get maximum source counts
		ALCint maxMonoSources = 0, maxStereoSources = 0;
		for (int i = 0; i < numAttribs; i++)
		{
			if (attribs[i] == ALC_MONO_SOURCES)
				maxMonoSources = attribs[i + 1];
			else if (attribs[i] == ALC_STEREO_SOURCES)
				maxStereoSources = attribs[i + 1];
		}

		alcDestroyContext(alContext);

		const ALCint ctxAttribs[] =
		{
			ALC_FREQUENCY, inFrequency,
			ALC_MONO_SOURCES, maxMonoSources,
			ALC_STEREO_SOURCES, maxStereoSources,
			ALC_REFRESH, 30,
			ALC_SYNC, ALC_FALSE
		};

		alContext = alcCreateContext(alDevice, NULL);
		if (alContext == nullptr)
			throw std::runtime_error("Failed to initialize OpenAL context");

		if (alcMakeContextCurrent(alContext) == ALC_FALSE)
			throw std::runtime_error("Failed to make OpenAL context current");

		ALfloat listenerOri[] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
		alListener3f(AL_POSITION, 0, 0, 0.0f);
		alListener3f(AL_VELOCITY, 0, 0, 0);
		alListenerfv(AL_ORIENTATION, listenerOri);

		// init sound sources/buffers
		alSources.resize(maxMonoSources);
		alBuffers.resize(maxMonoSources);
		alGenSources(maxMonoSources, &alSources[0]);
		alGenBuffers(maxMonoSources, &alBuffers[0]);

		// init music source/buffer
		alGenSources(1, &alMusicSource);

		alMusicBuffers.resize(musicBufferCount);
		alGenBuffers(musicBufferCount, &alMusicBuffers[0]);

		// init playback thread
		bExit = false;
		playbackThread = std::make_unique<std::thread>(startPlaybackThread, this);
	}

	~OpenALAudioDevice()
	{
		bExit = true;
		playbackThread->join();

		alDeleteBuffers(alMusicBuffers.size(), &alMusicBuffers[0]);
		alDeleteBuffers(alBuffers.size(), &alBuffers[0]);

		alDeleteSources(1, &alMusicSource);
		alDeleteSources(alSources.size(), &alSources[0]);

		alcDestroyContext(alContext);
		alcCloseDevice(alDevice);

		delete musicBuffer;
	}

	void AudioDevice::AddSound(USound* sound)
	{
		sounds.push_back(sound);
	}

	void AudioDevice::RemoveSound(USound* sound)
	{
	}

	void AudioDevice::PlayMusic(std::unique_ptr<AudioSource> source)
	{
		music = std::move(source);
		musicUpdate = true;
	}

	int PlaySound(int channel, USound* sound, float volume, float pan, float pitch) override
	{
		if (!std::isfinite(volume) || !std::isfinite(pan) || !std::isfinite(pitch) || channel >= alSources.size())
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

		return channel;
	}

	void UpdateSound(int channel, USound* sound, float volume, float pan, float pitch) override
	{
		if (!std::isfinite(volume) || !std::isfinite(pan) || !std::isfinite(pitch) || channel >= alSources.size())
			throw std::runtime_error("Invalid UpdateSound arguments");

		ActiveSound s;
		s.channel = channel;
		s.play = false;
		s.update = true;
		s.sound = sound;
		s.volume = volume;
		s.pan = pan;
		s.pitch = pitch;

		//SetupPanning(voice, s);
		//voice->SetFrequencyRatio(s.pitch);
		//voice->SetVolume(s.volume);
	}

	std::string getALErrorString()
	{
		switch (alGetError())
		{
		case AL_NO_ERROR:
			return "AL_NO_ERROR";
		case AL_INVALID_NAME:
			return "AL_INVALID_NAME";
		case AL_INVALID_ENUM:
			return "AL_INVALID_ENUM";
		case AL_INVALID_VALUE:
			return "AL_INVALID_VALUE";
		case AL_INVALID_OPERATION:
			return "AL_INVALID_OPERATION";
		case AL_OUT_OF_MEMORY:
			return "AL_OUT_OF_MEMORY";
		default:
			return "AL_UNKNOWN_ERROR";
		}
	}

	void PlayMusicBuffer() override
	{
		int format = AL_FORMAT_STEREO_FLOAT32;
		if (music->GetChannels() == 1)
			format = AL_FORMAT_MONO_FLOAT32;

		for (int i = 0; i < musicBufferCount; i++)
		{
			alBufferData(alMusicBuffers[i], format, musicQueue.Pop(), musicBufferSize*4, music->GetFrequency());
			if (alGetError() != AL_NO_ERROR)
				throw std::runtime_error("alBufferData failed in PlayMusicBuffer: " + getALErrorString());

			alSourceQueueBuffers(alMusicSource, 1, &alMusicBuffers[i]);
			if (alGetError() != AL_NO_ERROR)
				throw std::runtime_error("alSourceQueueBuffers failed in PlayMusicBuffer: " + getALErrorString());
		}

		alSourcef(alMusicSource, AL_GAIN, 1.0f);
		alSourcePlay(alMusicSource);
		if (alGetError() != AL_NO_ERROR)
			throw std::runtime_error("alSourcePlay failed in PlayMusicBuffer: " + getALErrorString());
	}

	void UpdateMusicBuffer() override
	{
		int format = AL_FORMAT_STEREO_FLOAT32;
		if (music->GetChannels() == 1)
			format = AL_FORMAT_MONO_FLOAT32;

		ALint status;
		alGetSourcei(alMusicSource, AL_BUFFERS_PROCESSED, &status);

		while (status)
		{
			ALuint buffer;
			alSourceUnqueueBuffers(alMusicSource, 1, &buffer);
			alBufferData(buffer, format, musicQueue.Pop(), musicBufferSize*4, music->GetFrequency());
			if (alGetError() != AL_NO_ERROR)
				throw std::runtime_error("alBufferData failed in UpdateMusicBuffer: " + getALErrorString());

			alSourceQueueBuffers(alMusicSource, 1, &buffer);
			if (alGetError() != AL_NO_ERROR)
				throw std::runtime_error("alSourceQueueBuffers failed in UpdateMusicBuffer: " + getALErrorString());

			status--;
		}

		alSourcef(alMusicSource, AL_GAIN, 1.0f);
		alGetSourcei(alMusicSource, AL_SOURCE_STATE, &status);
		if (status == AL_STOPPED)
		{
			alSourcePlay(alMusicSource);
			if (alGetError() != AL_NO_ERROR)
				throw std::runtime_error("alSourcePlay failed in PlayMusicBuffer: " + getALErrorString());
		}
	}

	void StopSound(int channel) override
	{
		if (channel >= alSources.size())
			throw std::runtime_error("Invalid StopSound arguments");

		//voices[channel]->Stop();
	}

	void SetMusicVolume(float volume) override
	{
		//musicVoice->SetVolume(volume);
	}

	void SetSoundVolume(float volume) override
	{
		//soundSubmixVoice->SetVolume(volume);
	}

	void Update() override
	{
		//UpdateMusic();
	}

	void UpdateMusicLoop()
	{
		while (!bExit)
		{
			playbackMutex.lock();
			if (music)
			{
				while (musicQueue.Size() < musicBufferCount)
				{
					// render a chunk of music
					music->ReadSamples(musicQueue.GetNextFree(), musicBufferSize);
					musicQueue.Push(musicQueue.GetNextFree());
				}
				playbackMutex.unlock();

				if (!bMusicPlaying)
				{
					PlayMusicBuffer();
					bMusicPlaying = true;
				}
				else if (musicQueue.Size() > 0)
				{
					UpdateMusicBuffer();
				}

				// TODO: music fade in/out
			}
			else
			{
				playbackMutex.unlock();
				bMusicPlaying = false;
			}
			using namespace std::chrono_literals;
			std::this_thread::sleep_for(5ms);
		}
	}

	ALCdevice* alDevice;
	ALCcontext* alContext;
	ALuint alMusicSource;
	ALenum alError;
	std::vector<ALuint> alSources;
	std::vector<ALuint> alBuffers;
	std::vector<ALuint> alMusicBuffers;
};

std::unique_ptr<AudioDevice> AudioDevice::Create(int frequency, int numVoices, int musicBufferCount, int musicBufferSize)
{
	return std::make_unique<OpenALAudioDevice>(frequency, numVoices, musicBufferCount, musicBufferSize);
}