
#include "Precomp.h"
#include "AudioDevice.h"
#include "AudioSource.h"
#include "Engine.h"
#include "Native/NObject.h"
#include "UObject/UActor.h"
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
		// TODO:
	}

	void AudioDevice::PlayMusic(std::unique_ptr<AudioSource> source)
	{
		playbackMutex.lock();
		music = std::move(source);
		musicUpdate = true;
		playbackMutex.unlock();
	}

	int PlaySound(int channel, USound* sound, vec3& location, float volume, float radius, float pitch)
	{
		if (!std::isfinite(volume) || volume < 0.0f || !std::isfinite(pitch) || channel >= alSources.size())
			throw std::runtime_error("Invalid PlaySound arguments");

		ALuint source = alSources[channel];
		ALint state;
		alGetSourcei(source, AL_SOURCE_STATE, &state);
		if (state == AL_PLAYING)
			throw std::runtime_error("Attempt to play sound on an active channel");

		int format = AL_FORMAT_MONO_FLOAT32;
		if (sound->GetChannels() == 2)
			format = AL_FORMAT_STEREO_FLOAT32;

		alBufferData(alBuffers[channel], format, sound->samples.data(), sound->samples.size(), sound->frequency);

		//alSource3f(source, AL_POSITION, location.x, location.y, location.z);
		//alSourcef(source, AL_GAIN, volume);
		//alSourcef(source, AL_MAX_DISTANCE, radius);
		//alSourcef(source, AL_PITCH, pitch);
		
		alSourceQueueBuffers(source, 1, &alBuffers[channel]);
		alSourcePlay(source);

		return channel;
	}

	void UpdateSound(int channel, USound* sound, vec3& location, float volume, float radius, float pitch)
	{
		if (!std::isfinite(volume) || !std::isfinite(pitch) || channel >= alSources.size())
			throw std::runtime_error("Invalid UpdateSound arguments");

		ALuint source = alSources[channel];
		//ALint state;
		//alGetSourcei(source, AL_SOURCE_STATE, &state);
		//if (state == AL_PLAYING)
		//{
			//alSource3f(source, AL_POSITION, location.x, location.y, location.z);
			//alSourcef(source, AL_GAIN, volume);
			//alSourcef(source, AL_MAX_DISTANCE, radius);
			//alSourcef(source, AL_PITCH, pitch);
		//}
	}

	void StopSound(int channel) override
	{
		if (channel >= alSources.size())
			throw std::runtime_error("Invalid StopSound arguments");

		ALuint source = alSources[channel];
		alSourceUnqueueBuffers(source, 1, &alBuffers[channel]);
		alSourceStop(source);
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

		ALenum error;
		for (int i = 0; i < musicBufferCount; i++)
		{
			alBufferData(alMusicBuffers[i], format, musicQueue.Pop(), musicBufferSize*4, music->GetFrequency());
			if ((error = alGetError()) != AL_NO_ERROR)
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

	void SetMusicVolume(float volume) override
	{
		alSourcef(alMusicSource, AL_GAIN, volume);
	}

	void SetSoundVolume(float volume) override
	{
		alListenerf(AL_GAIN, volume);
	}

	void Update() override
	{
		UActor* listener = engine->CameraActor;

		// Update listener properties
		vec3& location = listener->Location();
		vec3& velocity = listener->Velocity();

		// XXX: should we be using native functions here like this?
		vec3 at, left, up;
		NObject::GetAxes(listener->Rotation(), at, left, up);

		ALfloat orientation[6] =
		{
			at.x, at.y, at.z,
			up.x, up.y, up.z
		};

		//alListener3f(AL_POSITION, location.x, location.y, location.z);
		//alListener3f(AL_VELOCITY, velocity.x, velocity.y, velocity.z);
		//alListenerfv(AL_ORIENTATION, orientation);
	}

	void UpdateMusicLoop()
	{
		while (!bExit)
		{
			playbackMutex.lock();
			if (musicUpdate)
			{
				musicUpdate = false;
				if (bMusicPlaying)
				{
					alSourceStop(alMusicSource);
					alSourceUnqueueBuffers(alMusicSource, alMusicBuffers.size(), &alMusicBuffers[0]);
					bMusicPlaying = false;
				}
			}

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