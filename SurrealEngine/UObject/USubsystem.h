#pragma once

#include "UObject.h"
#include "UClient.h"
#include "Package/IniProperty.h"
#include "USound.h"

class USubsystem : public UObject
{
public:
	using UObject::UObject;

	virtual void LoadProperties(const NameString& from = "") {}
	virtual void SaveProperties() {}
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
	void SaveProperties() override;

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
	void SaveProperties() override;

	std::string GetPropertyAsString(const NameString& propertyName) const override;
	void SetPropertyFromString(const NameString& propertyName, const std::string& value) override;
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
	void SaveProperties();

	std::string GetPropertyAsString(const NameString& propertyName) const override;
	void SetPropertyFromString(const NameString& propertyName, const std::string& value) override;
};
