#pragma once

#include "UObject.h"
#include "UClient.h"
#include "Package/IniProperty.h"

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
	IniProperty<bool> Translucency = true;
	IniProperty<bool> VolumetricLighting = true;
	IniProperty<bool> ShinySurfaces = true;
	IniProperty<bool> Coronas = true;
	IniProperty<bool> HighDetailActors = true;

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
	IniProperty<bool> UseFilter = true;
	IniProperty<bool> UseSurround = true;
	IniProperty<bool> UseStereo = true;
	IniProperty<bool> UseCDMusic = false;
	IniProperty<bool> UseDigitalMusic = true;
	IniProperty<bool> UseSpatial = true;
	IniProperty<bool> UseReverb = true;
	IniProperty<bool> Use3dHardware = true;
	IniProperty<bool> LowSoundQuality = false;
	IniProperty<bool> ReverseStereo = false;
	IniProperty<int> Latency = 40;
	IniProperty<int> OutputRate = 44100;
	IniProperty<int> Channels = 16;
	IniProperty<uint8_t> MusicVolume = 160;
	IniProperty<uint8_t> SoundVolume = 200;
	IniProperty<float> AmbientFactor = 0.7f;

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
	IniProperty<bool> StartupFullscreen = false;
	IniProperty<int> WindowedViewportX = 1920;
	IniProperty<int> WindowedViewportY = 1080;
	IniProperty<int> WindowedColorBits = 32;
	IniProperty<int> FullscreenViewportX = 0;
	IniProperty<int> FullscreenViewportY = 0;
	IniProperty<int> FullscreenColorBits = 32;
	IniProperty<float> Brightness = 0.5f;
	IniProperty<bool> UseJoystick = false;
	IniProperty<bool> UseDirectInput = true;
	IniProperty<int> MinDesiredFrameRate = 200;
	IniProperty<bool> Decals = true;
	IniProperty<bool> NoDynamicLights = false;
	IniProperty<std::string> TextureDetail = "High";
	IniProperty<std::string> SkinDetail = "High";

	void LoadProperties(const NameString& from = "");
	void SaveProperties();

	std::string GetPropertyAsString(const NameString& propertyName) const override;
	void SetPropertyFromString(const NameString& propertyName, const std::string& value) override;
};
