#pragma once

#include "UObject.h"
#include "UActor.h"
#include "UClient.h"
#include "Package/IniProperty.h"
#include "USound.h"
#include "UMusic.h"
#include "Audio/AudioDevice.h"

struct PlayingSound
{
	PlayingSound() = default;
	PlayingSound(UActor* Actor, int Id, USound* Sound, vec3 Location, float Volume, float Radius, float Pitch, float Priority) : Actor(Actor), Id(Id), Sound(Sound), Location(Location), Volume(Volume), Radius(Radius), Pitch(Pitch), Priority(Priority) {}

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

class USubsystem : public UObject
{
public:
	using UObject::UObject;

	virtual void LoadProperties(const NameString& from = "") {}
};

class ULanguage : public UObject
{
public:
	using UObject::UObject;
};

class UEngine : public USubsystem
{
public:
	using USubsystem::USubsystem;
};

class UGameEngine : public UEngine
{
public:
	using UEngine::UEngine;
};

class UEditorEngine : public UEngine
{
public:
	using UEngine::UEngine;
};

class URenderDevice : public USubsystem
{
public:
	using USubsystem::USubsystem;
};

class UAudioSubsystem : public USubsystem
{
public:
	using USubsystem::USubsystem;
};

class UNetDriver : public USubsystem
{
public:
	using USubsystem::USubsystem;
};

class URenderBase : public USubsystem
{
public:
	using USubsystem::USubsystem;
};

/////////////////////////////////////////////////////////////////////////////

class USurrealRenderDevice : public URenderDevice
{
public:
	using URenderDevice::URenderDevice;

	std::string Class = "Engine.SurrealRenderDevice";
	bool Translucency = true;
	bool VolumetricLighting = true;
	bool ShinySurfaces = true;
	bool Coronas = true;
	bool HighDetailActors = true;

	void LoadProperties(const NameString& from = "") override;
	void SaveConfig() override;

	std::string GetPropertyAsString(const NameString& propertyName) const override;
	void SetPropertyFromString(const NameString& propertyName, const std::string& value) override;
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

	bool PlaySound(UActor* Actor, int Id, USound* Sound, vec3 Location, float Volume, float Radius, float Pitch);
	void NoteDestroy(UActor* Actor);
	void StopSounds();

	void BreakpointTriggered();
	void AddStats(Array<std::string>& lines);

	AudioDevice* GetDevice() { return m_Device.get(); }

private:
	void StartAmbience();
	void UpdateAmbience();
	void UpdateSounds(const mat4& listener);
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

class USurrealNetworkDevice : public UNetDriver
{
public:
	using UNetDriver::UNetDriver;

	std::string Class = "Engine.SurrealNetworkDevice";

	std::string GetPropertyAsString(const NameString& propertyName) const override;
	void SetPropertyFromString(const NameString& propertyName, const std::string& value);
};

class USurrealClient : public UClient
{
public:
	using UClient::UClient;

	std::string Class = "Engine.SurrealClient";
	bool StartupFullscreen = false;
	int WindowedViewportX = 1920;
	int WindowedViewportY = 1080;
	int WindowedColorBits = 32;
	int FullscreenViewportX = 0;
	int FullscreenViewportY = 0;
	int FullscreenColorBits = 32;
	float Brightness = 0.5f;
	bool UseJoystick = false;
	bool UseDirectInput = true;
	int MinDesiredFrameRate = 200;
	bool Decals = true;
	bool NoDynamicLights = false;
	std::string TextureDetail = "High";
	std::string SkinDetail = "High";

	void LoadProperties(const NameString& from = "");
	void SaveConfig() override;

	std::string GetPropertyAsString(const NameString& propertyName) const override;
	void SetPropertyFromString(const NameString& propertyName, const std::string& value) override;
};
