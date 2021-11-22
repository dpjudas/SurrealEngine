#pragma once

#include "AudioMixer.h"
#include "Math/vec.h"
#include "Math/mat.h"

class UActor;
class USound;
class UViewport;
class UMusic;

struct PlayingSound
{
	PlayingSound() = default;
	PlayingSound(UActor* Actor, int Id, USound* Sound, vec3 Location, float Volume, float Radius, float Pitch, float Priority) : Actor(Actor), Id(Id), Sound(Sound), Location(Location), Volume(Volume), Radius(Radius), Pitch(Pitch), Priority(Priority) { }

	int Id = 0;
	int Channel = 0;
	float Priority = 0.0f;
	UActor* Actor = nullptr;
	USound* Sound = nullptr;
	vec3 Location = { 0.0f, 0.0f, 0.0f };
	float Volume = 1.0f;
	float Radius = 1.0f;
	float Pitch = 1.0f;
	float CurrentVolume = 0.0f;
};

class AudioSubsystem
{
public:
	AudioSubsystem();

	void SetViewport(UViewport* Viewport);
	UViewport* GetViewport();

	void Update(const mat4& listener);

	bool PlaySound(UActor* Actor, int Id, USound* Sound, vec3 Location, float Volume, float Radius, float Pitch);
	void NoteDestroy(UActor* Actor);

	AudioMixer* GetMixer() { return Mixer.get(); }

private:
	void StartAmbience();
	void UpdateAmbience();
	void UpdateSounds(const mat4& listener);
	void UpdateMusic();
	void StopSound(size_t index);

	static float SoundPriority(UViewport* Viewport, vec3 Location, float Volume, float Radius);

	UViewport* Viewport = nullptr;

	bool UseDigitalMusic = true;
	bool ReverseStereo = false;
	int Channels = 32;
	uint8_t MusicVolume = 200;
	uint8_t SoundVolume = 255;
	float AmbientFactor = 0.7f;
	float DopplerSpeed = 9000.0f;

	std::unique_ptr<AudioMixer> Mixer;
	std::vector<PlayingSound> PlayingSounds;
	UMusic* CurrentSong = nullptr;
	int CurrentSection = 255;
	int FreeSlot = 0x07ffffff;
	bool AudioStats = false;
};
