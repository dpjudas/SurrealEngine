#pragma once

#include "UAudioSubsystem.h"
#include "Packages/Engine/Resources/USound.h"
#include "Audio/AudioDevice.h"

class UActor;
class UMusic;
class UViewport;
class AudioDevice;

struct PlayingSound
{
	PlayingSound() = default;
	PlayingSound(UActor* Actor, int Id, USound* Sound, vec3 Location, float Volume, float Radius, float Pitch, float Priority) : Actor(Actor), Id(Id), Sound(Sound), Location(Location), Volume(Volume), Radius(Radius), Pitch(Pitch), Priority(Priority) {}

	int Id = 0;
	bool IsActive = false;
	float Priority = 0.0f;
	UActor* Actor = nullptr;
	USound* Sound = nullptr;
	vec3 Location = { 0.0f, 0.0f, 0.0f };
	float Volume = 1.0f;
	float Radius = 1.0f;
	float Pitch = 1.0f;
	float StartTime = 0.0f;
};

class USurrealAudioDevice : public UAudioSubsystem
{
public:
	using UAudioSubsystem::UAudioSubsystem;

	std::string Class = "Engine.SurrealAudioDevice";
	bool UseFilter = true;
	bool UseSurround = true;
	bool UseStereo = true;
	bool UseCDMusic = false;
	bool UseDigitalMusic = true;
	bool UseSpatial = true;
	bool UseReverb = true;
	bool Use3dHardware = true;
	bool LowSoundQuality = false;
	bool ReverseStereo = false;
	int Latency = 40;
	AudioFrequency OutputRate = 44100;
	int Channels = 16;
	uint8_t MusicVolume = 160;
	uint8_t SoundVolume = 200;
	float AmbientFactor = 0.7f;

	void LoadProperties(const NameString& from = "") override;
	void SaveConfig() override;

	std::string GetPropertyAsString(const NameString& propertyName) const override;
	void SetPropertyFromString(const NameString& propertyName, const std::string& value) override;

	void InitDevice();
	void ShutdownDevice();
	void SetViewport(UViewport* InViewport);
	UViewport* GetViewport() { return m_Viewport; }

	void Update(const mat4& listener);

	bool PlaySound(UActor* Actor, int Id, USound* Sound, vec3 Location, float Volume, float Radius, float Pitch, bool isTalk);
	void StopSound(UActor* Actor, int Id);
	void ActorDestroyed(UActor* Actor);
	void StopSounds();

	void BreakpointTriggered();
	void AddStats(Array<std::string>& lines);

	AudioDevice* GetDevice() { return m_Device.get(); }

private:
	void StartAmbience();
	void UpdateAmbience();
	void UpdateSounds(const mat4& listener);

	void UpdateLipSync(PlayingSound& Playing);
	void ClearLipSync(PlayingSound& Playing);
	void UpdateMusic();
	void StopSound(size_t index);

	static float SoundPriority(UViewport* Viewport, vec3 Location, float Volume, float Radius);

	UViewport* m_Viewport = nullptr;

	std::unique_ptr<AudioDevice> m_Device;
	Array<PlayingSound> PlayingSounds;
	UMusic* CurrentSong = nullptr;
	int CurrentSection = 255;
	int FreeSlot = 0x07ffffff;
};
