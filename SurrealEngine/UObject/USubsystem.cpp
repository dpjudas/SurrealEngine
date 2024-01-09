
#include "Precomp.h"
#include "USubsystem.h"
#include "Engine.h"
#include "Package/PackageManager.h"

std::string USurrealRenderDevice::GetPropertyAsString(const NameString& propertyName) const
{
	if (propertyName == "Class")
		return "class'" + Class + "'";
	if (propertyName == "Translucency")
		return IniPropertyConverter<bool>::ToString(Translucency);
	else if (propertyName == "VolumetricLighting")
		return IniPropertyConverter<bool>::ToString(VolumetricLighting);
	else if (propertyName == "ShinySurfaces")
		return IniPropertyConverter<bool>::ToString(ShinySurfaces);
	else if (propertyName == "Coronas")
		return IniPropertyConverter<bool>::ToString(Coronas);
	else if (propertyName == "HighDetailActors")
		return IniPropertyConverter<bool>::ToString(HighDetailActors);

	engine->LogMessage("Queried unknown property for SurrealRenderDevice: " + propertyName.ToString());
	return {};
}

void USurrealRenderDevice::SetPropertyFromString(const NameString& propertyName, const std::string& value)
{
	if (propertyName == "Translucency")
		Translucency = IniPropertyConverter<bool>::FromString(value);
	else if (propertyName == "VolumetricLighting")
		VolumetricLighting = IniPropertyConverter<bool>::FromString(value);
	else if (propertyName == "ShinySurfaces")
		ShinySurfaces = IniPropertyConverter<bool>::FromString(value);
	else if (propertyName == "Coronas")
		Coronas = IniPropertyConverter<bool>::FromString(value);
	else if (propertyName == "HighDetailActors")
		HighDetailActors = IniPropertyConverter<bool>::FromString(value);
	else
		engine->LogMessage("Setting unknown property for SurrealRenderDevice: " + propertyName.ToString());

	engine->packages->SetIniValue("System", Class, propertyName, value);
}

void USurrealRenderDevice::LoadProperties(const NameString& from)
{	
	NameString name_from = from;
	
	if (from == "")
		name_from = NameString(Class);

	Translucency = IniPropertyConverter<bool>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "Translucency");
	VolumetricLighting = IniPropertyConverter<bool>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "VolumetricLighting");
	ShinySurfaces = IniPropertyConverter<bool>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "ShinySurfaces");
	Coronas = IniPropertyConverter<bool>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "Coronas");
	HighDetailActors = IniPropertyConverter<bool>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "HighDetailActors");
}

void USurrealRenderDevice::SaveProperties()
{
	engine->packages->SetIniValue("System", Class, "Translucency", IniPropertyConverter<bool>::ToString(Translucency));
	engine->packages->SetIniValue("System", Class, "VolumetricLighting", IniPropertyConverter<bool>::ToString(VolumetricLighting));
	engine->packages->SetIniValue("System", Class, "ShinySurfaces", IniPropertyConverter<bool>::ToString(ShinySurfaces));
	engine->packages->SetIniValue("System", Class, "Coronas", IniPropertyConverter<bool>::ToString(Coronas));
	engine->packages->SetIniValue("System", Class, "HighDetailActors", IniPropertyConverter<bool>::ToString(HighDetailActors));
}

/////////////////////////////////////////////////////////////////////////////

std::string USurrealAudioDevice::GetPropertyAsString(const NameString& propertyName) const
{
	if (propertyName == "Class")
		return "class'" + Class + "'";
	else if (propertyName == "UseFilter")
		return IniPropertyConverter<bool>::ToString(UseFilter);
	else if (propertyName == "UseSurround")
		return IniPropertyConverter<bool>::ToString(UseSurround);
	else if (propertyName == "UseStereo")
		return IniPropertyConverter<bool>::ToString(UseStereo);
	else if (propertyName == "UseCDMusic")
		return IniPropertyConverter<bool>::ToString(UseCDMusic);
	else if (propertyName == "UseDigitalMusic")
		return IniPropertyConverter<bool>::ToString(UseDigitalMusic);
	else if (propertyName == "UseSpatial")
		return IniPropertyConverter<bool>::ToString(UseSpatial);
	else if (propertyName == "UseReverb")
		return IniPropertyConverter<bool>::ToString(UseReverb);
	else if (propertyName == "Use3dHardware")
		return IniPropertyConverter<bool>::ToString(Use3dHardware);
	else if (propertyName == "LowSoundQuality")
		return IniPropertyConverter<bool>::ToString(LowSoundQuality);
	else if (propertyName == "ReverseStereo")
		return IniPropertyConverter<bool>::ToString(ReverseStereo);
	else if (propertyName == "Latency")
		return IniPropertyConverter<int>::ToString(Latency);
	else if (propertyName == "OutputRate")
		return IniPropertyConverter<int>::ToString(OutputRate);
	else if (propertyName == "Channels")
		return IniPropertyConverter<int>::ToString(Channels);
	else if (propertyName == "MusicVolume")
		return IniPropertyConverter<uint8_t>::ToString(MusicVolume);
	else if (propertyName == "SoundVolume")
		return IniPropertyConverter<uint8_t>::ToString(SoundVolume);
	else if (propertyName == "AmbientFactor")
		return IniPropertyConverter<float>::ToString(AmbientFactor);

	engine->LogMessage("Queried unknown property for SurrealAudioDevice: " + propertyName.ToString());
	return {};
}

void USurrealAudioDevice::SetPropertyFromString(const NameString& propertyName, const std::string& value)
{
	if (propertyName == "UseFilter")
		UseFilter = IniPropertyConverter<bool>::FromString(value);
	else if (propertyName == "UseSurround")
		UseSurround = IniPropertyConverter<bool>::FromString(value);
	else if (propertyName == "UseStereo")
		UseStereo = IniPropertyConverter<bool>::FromString(value);
	else if (propertyName == "UseCDMusic")
		UseCDMusic = IniPropertyConverter<bool>::FromString(value);
	else if (propertyName == "UseDigitalMusic")
		UseDigitalMusic = IniPropertyConverter<bool>::FromString(value);
	else if (propertyName == "UseSpatial")
		UseSpatial = IniPropertyConverter<bool>::FromString(value);
	else if (propertyName == "UseReverb")
		UseReverb = IniPropertyConverter<bool>::FromString(value);
	else if (propertyName == "Use3dHardware")
		Use3dHardware = IniPropertyConverter<bool>::FromString(value);
	else if (propertyName == "LowSoundQuality")
		LowSoundQuality = IniPropertyConverter<bool>::FromString(value);
	else if (propertyName == "ReverseStereo")
		ReverseStereo = IniPropertyConverter<bool>::FromString(value);
	else if (propertyName == "Latency")
		Latency = IniPropertyConverter<int>::FromString(value);
	else if (propertyName == "OutputRate")
		OutputRate = IniPropertyConverter<int>::FromString(value);
	else if (propertyName == "Channels")
		Channels = IniPropertyConverter<int>::FromString(value);
	else if (propertyName == "MusicVolume")
		MusicVolume = IniPropertyConverter<uint8_t>::FromString(value);
	else if (propertyName == "SoundVolume")
		SoundVolume = IniPropertyConverter<uint8_t>::FromString(value);
	else if (propertyName == "AmbientFactor")
		AmbientFactor = IniPropertyConverter<float>::FromString(value);
	else
		engine->LogMessage("Setting unknown property for SurrealAudioDevice: " + propertyName.ToString());

	engine->packages->SetIniValue("System", Class, propertyName, value);
}

void USurrealAudioDevice::LoadProperties(const NameString& from)
{
	NameString name_from = from;

	if (from == "")
		name_from = NameString(Class);

	UseFilter = IniPropertyConverter<bool>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "UseFilter");
	UseSurround = IniPropertyConverter<bool>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "UseSurround");
	UseStereo = IniPropertyConverter<bool>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "UseStereo");
	UseCDMusic = IniPropertyConverter<bool>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "UseCDMusic");
	UseDigitalMusic = IniPropertyConverter<bool>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "UseDigitalMusic");
	UseSpatial = IniPropertyConverter<bool>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "UseSpatial");
	UseReverb = IniPropertyConverter<bool>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "UseReverb");
	Use3dHardware = IniPropertyConverter<bool>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "Use3dHardware");
	LowSoundQuality = IniPropertyConverter<bool>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "LowSoundQuality");
	ReverseStereo = IniPropertyConverter<bool>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "ReverseStereo");
	Latency = IniPropertyConverter<int>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "Latency");
	OutputRate = IniPropertyConverter<int>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "OutputRate");
	Channels = IniPropertyConverter<int>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "Channels");
	MusicVolume = IniPropertyConverter<uint8_t>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "MusicVolume");
	SoundVolume = IniPropertyConverter<uint8_t>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "SoundVolume");
	AmbientFactor = IniPropertyConverter<float>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "AmbientFactor");
}

void USurrealAudioDevice::SaveProperties()
{
	engine->packages->SetIniValue("System", Class, "UseFilter", IniPropertyConverter<bool>::ToString(UseFilter));
	engine->packages->SetIniValue("System", Class, "UseSurround", IniPropertyConverter<bool>::ToString(UseSurround));
	engine->packages->SetIniValue("System", Class, "UseStereo", IniPropertyConverter<bool>::ToString(UseStereo));
	engine->packages->SetIniValue("System", Class, "UseCDMusic", IniPropertyConverter<bool>::ToString(UseCDMusic));
	engine->packages->SetIniValue("System", Class, "UseDigitalMusic", IniPropertyConverter<bool>::ToString(UseDigitalMusic));
	engine->packages->SetIniValue("System", Class, "UseSpatial", IniPropertyConverter<bool>::ToString(UseSpatial));
	engine->packages->SetIniValue("System", Class, "UseReverb", IniPropertyConverter<bool>::ToString(UseReverb));
	engine->packages->SetIniValue("System", Class, "Use3dHardware", IniPropertyConverter<bool>::ToString(Use3dHardware));
	engine->packages->SetIniValue("System", Class, "LowSoundQuality", IniPropertyConverter<bool>::ToString(LowSoundQuality));
	engine->packages->SetIniValue("System", Class, "ReverseStereo", IniPropertyConverter<bool>::ToString(ReverseStereo));
	engine->packages->SetIniValue("System", Class, "Latency", IniPropertyConverter<int>::ToString(Latency));
	engine->packages->SetIniValue("System", Class, "OutputRate", IniPropertyConverter<int>::ToString(OutputRate));
	engine->packages->SetIniValue("System", Class, "Channels", IniPropertyConverter<int>::ToString(Channels));
	engine->packages->SetIniValue("System", Class, "MusicVolume", IniPropertyConverter<uint8_t>::ToString(MusicVolume));
	engine->packages->SetIniValue("System", Class, "SoundVolume", IniPropertyConverter<uint8_t>::ToString(SoundVolume));
	engine->packages->SetIniValue("System", Class, "AmbientFactor", IniPropertyConverter<float>::ToString(AmbientFactor));
}

/////////////////////////////////////////////////////////////////////////////

std::string USurrealNetworkDevice::GetPropertyAsString(const NameString& propertyName) const
{
	if (propertyName == "Class")
		return "class'" + Class + "'";

	engine->LogMessage("Queried unknown property for SurrealNetworkDevice: " + propertyName.ToString());
	return {};
}

void USurrealNetworkDevice::SetPropertyFromString(const NameString& propertyName, const std::string& value)
{
	engine->LogMessage("Setting unknown property for SurrealNetworkDevice: " + propertyName.ToString());
	engine->packages->SetIniValue("System", Class, propertyName, value);
}

/////////////////////////////////////////////////////////////////////////////

void USurrealClient::LoadProperties(const NameString& from)
{
	NameString name_from = from;

	if (from == "")
		name_from = NameString(Class);

	StartupFullscreen = IniPropertyConverter<bool>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "StartupFullscreen");
	WindowedViewportX = IniPropertyConverter<int>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "WindowedViewportX");
	WindowedViewportY = IniPropertyConverter<int>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "WindowedViewportY");
	WindowedColorBits = IniPropertyConverter<int>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "WindowedColorBits");
	FullscreenViewportX = IniPropertyConverter<int>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "FullscreenViewportX");
	FullscreenViewportY = IniPropertyConverter<int>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "FullscreenViewportY");
	FullscreenColorBits = IniPropertyConverter<int>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "FullscreenColorBits");
	Brightness = IniPropertyConverter<float>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "Brightness");
	UseJoystick = IniPropertyConverter<bool>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "UseJoystick");
	UseDirectInput = IniPropertyConverter<bool>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "UseDirectInput");
	MinDesiredFrameRate = IniPropertyConverter<int>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "MinDesiredFrameRate");
	Decals = IniPropertyConverter<bool>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "Decals");
	NoDynamicLights = IniPropertyConverter<bool>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "NoDynamicLights");
	TextureDetail = IniPropertyConverter<std::string>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "TextureDetail");
	SkinDetail = IniPropertyConverter<std::string>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "SkinDetail");
}

void USurrealClient::SaveProperties()
{
	engine->packages->SetIniValue("System", Class, "StartupFullscreen", IniPropertyConverter<bool>::ToString(StartupFullscreen));
	engine->packages->SetIniValue("System", Class, "WindowedViewportX", IniPropertyConverter<int>::ToString(WindowedViewportX));
	engine->packages->SetIniValue("System", Class, "WindowedViewportY", IniPropertyConverter<int>::ToString(WindowedViewportY));
	engine->packages->SetIniValue("System", Class, "WindowedColorBits", IniPropertyConverter<int>::ToString(WindowedColorBits));
	engine->packages->SetIniValue("System", Class, "FullscreenViewportX", IniPropertyConverter<int>::ToString(FullscreenViewportX));
	engine->packages->SetIniValue("System", Class, "FullscreenViewportY", IniPropertyConverter<int>::ToString(FullscreenViewportY));
	engine->packages->SetIniValue("System", Class, "FullscreenColorBits", IniPropertyConverter<int>::ToString(FullscreenColorBits));
	engine->packages->SetIniValue("System", Class, "Brightness", IniPropertyConverter<float>::ToString(Brightness));
	engine->packages->SetIniValue("System", Class, "UseJoystick", IniPropertyConverter<bool>::ToString(UseJoystick));
	engine->packages->SetIniValue("System", Class, "UseDirectInput", IniPropertyConverter<bool>::ToString(UseDirectInput));
	engine->packages->SetIniValue("System", Class, "MinDesiredFrameRate", IniPropertyConverter<int>::ToString(MinDesiredFrameRate));
	engine->packages->SetIniValue("System", Class, "Decals", IniPropertyConverter<bool>::ToString(Decals));
	engine->packages->SetIniValue("System", Class, "NoDynamicLights", IniPropertyConverter<bool>::ToString(NoDynamicLights));
	engine->packages->SetIniValue("System", Class, "TextureDetail", TextureDetail);
	engine->packages->SetIniValue("System", Class, "SkinDetail", SkinDetail);
}

std::string USurrealClient::GetPropertyAsString(const NameString& propertyName) const
{
	if (propertyName == "Class")
		return "class'" + Class + "'";
	else if (propertyName == "StartupFullscreen")
		return IniPropertyConverter<bool>::ToString(StartupFullscreen);
	else if (propertyName == "WindowedViewportX")
		return IniPropertyConverter<int>::ToString(WindowedViewportX);
	else if (propertyName == "WindowedViewportY")
		return IniPropertyConverter<int>::ToString(WindowedViewportY);
	else if (propertyName == "WindowedColorBits")
		return IniPropertyConverter<int>::ToString(WindowedColorBits);
	else if (propertyName == "FullscreenViewportX")
		return IniPropertyConverter<int>::ToString(FullscreenViewportX);
	else if (propertyName == "FullscreenViewportY")
		return IniPropertyConverter<int>::ToString(FullscreenViewportY);
	else if (propertyName == "FullscreenColorBits")
		return IniPropertyConverter<int>::ToString(FullscreenColorBits);
	else if (propertyName == "Brightness")
		return IniPropertyConverter<float>::ToString(Brightness);
	else if (propertyName == "UseJoystick")
		return IniPropertyConverter<bool>::ToString(UseJoystick);
	else if (propertyName == "UseDirectInput")
		return IniPropertyConverter<bool>::ToString(UseDirectInput);
	else if (propertyName == "MinDesiredFrameRate")
		return IniPropertyConverter<int>::ToString(MinDesiredFrameRate);
	else if (propertyName == "Decals")
		return IniPropertyConverter<bool>::ToString(Decals);
	else if (propertyName == "NoDynamicLights")
		return IniPropertyConverter<bool>::ToString(NoDynamicLights);
	else if (propertyName == "TextureDetail")
		return TextureDetail;
	else if (propertyName == "SkinDetail")
		return SkinDetail;

	engine->LogMessage("Queried unknown property for Surreal.ViewportManager: " + propertyName.ToString());
	return {};
}

void USurrealClient::SetPropertyFromString(const NameString& propertyName, const std::string& value)
{
	if (propertyName == "WindowedViewportX")
		WindowedViewportX = IniPropertyConverter<int>::FromString(value);
	else if (propertyName == "WindowedViewportY")
		WindowedViewportY = IniPropertyConverter<int>::FromString(value);
	else if (propertyName == "WindowedColorBits")
		WindowedColorBits = IniPropertyConverter<int>::FromString(value);
	else if (propertyName == "FullscreenViewportX")
		FullscreenViewportX = IniPropertyConverter<int>::FromString(value);
	else if (propertyName == "FullscreenViewportY")
		FullscreenViewportY = IniPropertyConverter<int>::FromString(value);
	else if (propertyName == "FullscreenColorBits")
		FullscreenColorBits = IniPropertyConverter<int>::FromString(value);
	else if (propertyName == "Brightness")
		Brightness = IniPropertyConverter<float>::FromString(value);
	else if (propertyName == "UseJoystick")
		UseJoystick = IniPropertyConverter<bool>::FromString(value);
	else if (propertyName == "UseDirectInput")
		UseDirectInput = IniPropertyConverter<bool>::FromString(value);
	else if (propertyName == "MinDesiredFrameRate")
		MinDesiredFrameRate = IniPropertyConverter<int>::FromString(value);
	else if (propertyName == "Decals")
		Decals = IniPropertyConverter<bool>::FromString(value);
	else if (propertyName == "NoDynamicLights")
		NoDynamicLights = IniPropertyConverter<bool>::FromString(value);
	else if (propertyName == "TextureDetail")
		TextureDetail = value;
	else if (propertyName == "SkinDetail")
		SkinDetail = value;
	else
		engine->LogMessage("Setting unknown property for Surreal.ViewportManager: " + propertyName.ToString());

	engine->packages->SetIniValue("System", Class, propertyName, value);
}
