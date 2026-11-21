#pragma once

#include "AudioMixer.h"
#include "ActiveSound.h"
#include <map>

class AudioPlayer;

class AudioMixerImpl : public AudioMixer
{
public:
	AudioMixerImpl();
	~AudioMixerImpl();

	AudioSound* AddSound(std::unique_ptr<AudioSource> source, bool looped, uint64_t loopStart, uint64_t loopEnd) override;
	void RemoveSound(AudioSound* sound) override;
	float GetSoundDuration(AudioSound* sound) override;
	int PlaySound(int channel, AudioSound* sound, float volume, float pan, float pitch) override;
	void UpdateSound(int channel, AudioSound* sound, float volume, float pan, float pitch) override;
	void StopSound(int channel) override;
	bool SoundFinished(int channel) override;
	void PlayMusic(std::unique_ptr<AudioSource> source) override;
	void SetMusicVolume(float volume) override;
	void SetSoundVolume(float volume) override;
	void Update() override;

private:
	int mixing_frequency = 48000;

	std::vector<std::unique_ptr<AudioSound>> sounds;
	std::unique_ptr<AudioPlayer> player;
	std::map<int, int> channelplaying;

	struct
	{
		std::vector<ActiveSound> sounds;
		std::unique_ptr<AudioSource> music;
		bool musicupdate = false;
		float soundvolume = 1.0f;
		float musicvolume = 1.0f;
	} client, transfer;
	std::vector<int> channelstopped;

	std::mutex mutex;
	int nextid = 1;

	friend class AudioMixerSource;
};
