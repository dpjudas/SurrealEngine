
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

#define UU_PER_METER 43

class ALSoundSource
{
public:
	ALSoundSource()
	{
		alGenSources(1, &id);
		if (alGetError() != AL_NO_ERROR)
			throw std::runtime_error("Failed to generate AL source");

		alSourcef(id, AL_ROLLOFF_FACTOR, 1.1f);
	}

	~ALSoundSource()
	{
		alDeleteSources(1, &id);
	}

	void Play()
	{
		alSourcePlay(id);
		if (alGetError() != AL_NO_ERROR)
			throw std::runtime_error("Failed to play AL source");
	}

	void Stop()
	{
		alSourceStop(id);
	}

	bool IsPlaying()
	{
		if (!alIsSource(id))
			return false;

		ALint state;
		alGetSourcei(id, AL_SOURCE_STATE, &state);
		return state == AL_PLAYING;
	}

	USound* GetSound()
	{
		return sound;
	}

	void SetDopplerFactor(float newDopplerFactor)
	{
		if (dopplerFactor != newDopplerFactor)
		{
			dopplerFactor = newDopplerFactor;
			alSourcef(id, AL_DOPPLER_FACTOR, dopplerFactor);
		}
	}

	void SetPosition(vec3& newPosition)
	{
		position.x = newPosition.x;
		position.y = newPosition.y;
		position.z = -newPosition.z;
		alSourcefv(id, AL_POSITION, &position[0]);
	}

	void SetRadius(float newRadius)
	{
		if (radius != newRadius)
		{
			radius = newRadius;
			alSourcef(id, AL_MAX_DISTANCE, radius);
			alSourcef(id, AL_REFERENCE_DISTANCE, 0.1 * radius);
		}
	}

	void SetVelocity(vec3& newVelocity)
	{
		velocity.x = newVelocity.x;
		velocity.y = newVelocity.y;
		velocity.z = -newVelocity.z;
		alSourcefv(id, AL_VELOCITY, &velocity[0]);
	}

	void SetSound(USound* newSound)
	{
		if (sound != newSound)
		{
			sound = newSound;
			alSourcei(id, AL_BUFFER, 0);
			alGetError();
			alSourcei(id, AL_BUFFER, reinterpret_cast<ALint>(sound->handle));

			if (sound->loopInfo.Looped)
				alSourcei(id, AL_LOOPING, AL_TRUE);
			else
				alSourcei(id, AL_LOOPING, AL_FALSE);
		}
	}

	void SetSpatial(bool bSpatial)
	{
		if (bIs3d != bSpatial)
		{
			bIs3d = bSpatial;
			alSourcei(id, AL_SOURCE_SPATIALIZE_SOFT, bIs3d);
		}
	}

	void SetVolume(float newVolume)
	{
		if (volume != newVolume)
		{
			volume = newVolume;
			alSourcef(id, AL_GAIN, volume);
			alSourcef(id, AL_MAX_GAIN, volume);
		}
	}

	void SetPitch(float newPitch)
	{
		if (pitch != newPitch)
		{
			pitch = newPitch;
			alSourcef(id, AL_PITCH, pitch);
		}
	}

	void DoLoop()
	{
		if (sound->loopInfo.Looped)
		{
			ALint offset;
			alGetSourcei(id, AL_SAMPLE_OFFSET, &offset);
			if (offset >= sound->loopInfo.LoopEnd)
				alSourcei(id, AL_SAMPLE_OFFSET, sound->loopInfo.LoopStart);
		}
	}

	ALuint id = -1;

private:
	UActor* actor = nullptr;
	USound* sound = nullptr;
	vec3 position;
	vec3 velocity;
	float radius = 0.0f;
	float volume = 0.0f;
	float pitch = 0.0f;
	float dopplerFactor = 0.0f;
	bool bIs3d = false;
};

// TODO list:
//  Sound looping
//  Positional audio
//  Music fade in/out
//  Music crossfade
//  EFX effects
//  Support for real EAX hardware
//  (maybe) A3D style sound tracing

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

		const ALCint ctxAttribs[] =
		{
			ALC_FREQUENCY, inFrequency,
			ALC_REFRESH, 60,
			ALC_SYNC, ALC_FALSE
		};

		alContext = alcCreateContext(alDevice, NULL);
		if (alContext == nullptr)
			throw std::runtime_error("Failed to initialize OpenAL context");

		if (alcMakeContextCurrent(alContext) == ALC_FALSE)
			throw std::runtime_error("Failed to make OpenAL context current");

		// init listener state
		ALfloat listenerOri[] = { 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, -1.0f };
		alListener3f(AL_POSITION, 0, 0, 0.0f);
		alListener3f(AL_VELOCITY, 0, 0, 0);
		alListenerfv(AL_ORIENTATION, listenerOri);
		alListenerf(AL_METERS_PER_UNIT, 1.f / UU_PER_METER);

		alDistanceModel(AL_LINEAR_DISTANCE_CLAMPED);
		alSpeedOfSound(343.3f / (1.0f / UU_PER_METER));

		// Init sound sources
		alcGetIntegerv(alDevice, ALC_MONO_SOURCES, 1, &monoSources);
		alcGetIntegerv(alDevice, ALC_STEREO_SOURCES, 1, &stereoSources);

		// TODO: how do we prioritize mono vs stereo source count?
		sources.resize(monoSources);

		// init music source/buffer
		alGenSources(1, &alMusicSource);
		alSourcei(alMusicSource, AL_SOURCE_SPATIALIZE_SOFT, AL_FALSE);

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
		alDeleteSources(1, &alMusicSource);

		alcDestroyContext(alContext);
		alcCloseDevice(alDevice);

		delete musicBuffer;
	}

	void AudioDevice::AddSound(USound* sound)
	{
		sounds.push_back(sound);

		ALenum format = AL_FORMAT_MONO_FLOAT32;
		if (sound->channels == 2)
			format = AL_FORMAT_STEREO_FLOAT32;

		ALuint id;
		alGenBuffers(1, &id);
		alBufferData(id, format, sound->samples.data(), sound->samples.size()*sizeof(sound->samples[0]), sound->frequency);
		alError = alGetError();
		if (alError != AL_NO_ERROR)
			throw std::runtime_error("Failed to buffer sound data for " + sound->Name.ToString());

		sound->handle = reinterpret_cast<void*>(id);
	}

	void AudioDevice::RemoveSound(USound* sound)
	{
		auto it = sounds.begin();
		while (it != sounds.end())
		{
			if (*it == sound)
			{
				// TOOD: find sources playing this sound and stop them?
				sounds.erase(it);
				alDeleteBuffers(1, reinterpret_cast<const ALuint*>(&sound->handle));
				return;
			}
		}
	}

	bool IsPlaying(int channel)
	{
		ALSoundSource& source = sources[channel];
		return source.IsPlaying();
	}

	void PlayMusic(std::unique_ptr<AudioSource> source)
	{
		playbackMutex.lock();
		music = std::move(source);
		musicUpdate = true;
		playbackMutex.unlock();
	}

	int PlaySound(int channel, USound* sound, vec3& location, float volume, float radius, float pitch)
	{
		if (!std::isfinite(volume) || volume < 0.0f || !std::isfinite(pitch) || channel >= sources.size())
			throw std::runtime_error("Invalid PlaySound arguments");

		ALSoundSource& source = sources[channel];
		if (source.IsPlaying())
		{
			engine->LogMessage("Attempted to play sound on active channel " + channel);
			return 0;
		}

		vec3 dummy = { 0.0f,0.0f,0.0f };

		source.SetSound(sound);
		source.SetPosition(dummy);
		source.SetVolume(volume / 255.0f);
		source.SetRadius(radius);
		source.SetPitch(pitch);
		source.SetSpatial(true);
		source.Play();

		return channel;
	}

	void UpdateSound(int channel, USound* sound, vec3& location, float volume, float radius, float pitch)
	{
		if (!std::isfinite(volume) || volume < 0.0f || !std::isfinite(pitch) || channel >= sources.size())
			throw std::runtime_error("Invalid PlaySound arguments");

		ALSoundSource& source = sources[channel];

		source.SetPosition(location);
		source.SetVolume(volume / 255.0f);
		source.SetRadius(radius);
		source.SetPitch(pitch);

		source.DoLoop();
	}

	void StopSound(int channel) override
	{
		if (channel >= sources.size())
			throw std::runtime_error("Invalid StopSound arguments");

		sources[channel].Stop();
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

		vec3 at, left, up;
		listener->Rotation().GetAxes(at, left, up);

		ALfloat listenerOri[6] = {up.x, up.y, -up.z, -at.x, -at.y, at.z};
		alListener3f(AL_POSITION, location.x, location.y, -location.z);
		alListener3f(AL_VELOCITY, velocity.x, velocity.y, -velocity.z);
		alListenerfv(AL_ORIENTATION, listenerOri);
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
	ALenum alError;
	ALuint alMusicSource;
	std::vector<ALuint> alMusicBuffers;
	std::vector<ALSoundSource> sources;
	ALint monoSources;
	ALint stereoSources;
};

std::unique_ptr<AudioDevice> AudioDevice::Create(int frequency, int numVoices, int musicBufferCount, int musicBufferSize)
{
	return std::make_unique<OpenALAudioDevice>(frequency, numVoices, musicBufferCount, musicBufferSize);
}