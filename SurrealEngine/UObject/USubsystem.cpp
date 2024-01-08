
#include "Precomp.h"
#include "USubsystem.h"
#include "Engine.h"
#include "Package/PackageManager.h"

std::string USurrealRenderDevice::GetPropertyAsString(const NameString& propertyName) const
{
	if (propertyName == "Class")
		return "class'" + Class + "'";
	if (propertyName == "Translucency")
		return Translucency.ToString();
	else if (propertyName == "VolumetricLighting")
		return VolumetricLighting.ToString();
	else if (propertyName == "ShinySurfaces")
		return ShinySurfaces.ToString();
	else if (propertyName == "Coronas")
		return Coronas.ToString();
	else if (propertyName == "HighDetailActors")
		return HighDetailActors.ToString();

	engine->LogMessage("Queried unknown property for SurrealRenderDevice: " + propertyName.ToString());
	return {};
}

void USurrealRenderDevice::SetPropertyFromString(const NameString& propertyName, const std::string& value)
{
	if (propertyName == "Translucency")
		Translucency.FromString(value);
	else if (propertyName == "VolumetricLighting")
		VolumetricLighting.FromString(value);
	else if (propertyName == "ShinySurfaces")
		ShinySurfaces.FromString(value);
	else if (propertyName == "Coronas")
		Coronas.FromString(value);
	else if (propertyName == "HighDetailActors")
		HighDetailActors.FromString(value);
	else
		engine->LogMessage("Setting unknown property for SurrealRenderDevice: " + propertyName.ToString());

	engine->packages->SetIniValue("System", Class, propertyName, value);
}

void USurrealRenderDevice::LoadProperties(const NameString& from)
{	
	NameString name_from = from;
	
	if (from == "")
		name_from = NameString(Class);

	Translucency.FromIniFile(*engine->packages->GetIniFile("System"), name_from, "Translucency");
	//Translucency = std::atoi(engine->packages->GetIniValue("System", name_from, "Translucency", Translucency.ToString()).c_str());
	VolumetricLighting.FromIniFile(*engine->packages->GetIniFile("System"), name_from, "VolumetricLighting");
	ShinySurfaces.FromIniFile(*engine->packages->GetIniFile("System"), name_from, "ShinySurfaces");
	Coronas.FromIniFile(*engine->packages->GetIniFile("System"), name_from, "Coronas");
	HighDetailActors.FromIniFile(*engine->packages->GetIniFile("System"), name_from, "HighDetailActors");
}

void USurrealRenderDevice::SaveProperties()
{
	engine->packages->SetIniValue("System", Class, "Translucency", Translucency.ToString());
	engine->packages->SetIniValue("System", Class, "VolumetricLighting", VolumetricLighting.ToString());
	engine->packages->SetIniValue("System", Class, "ShinySurfaces", ShinySurfaces.ToString());
	engine->packages->SetIniValue("System", Class, "Coronas", Coronas.ToString());
	engine->packages->SetIniValue("System", Class, "HighDetailActors", HighDetailActors.ToString());
}

/////////////////////////////////////////////////////////////////////////////

std::string USurrealAudioDevice::GetPropertyAsString(const NameString& propertyName) const
{
	if (propertyName == "Class")
		return "class'" + Class + "'";
	else if (propertyName == "UseFilter")
		return UseFilter.ToString();
	else if (propertyName == "UseSurround")
		return UseSurround.ToString();
	else if (propertyName == "UseStereo")
		return UseStereo.ToString();
	else if (propertyName == "UseCDMusic")
		return UseCDMusic.ToString();
	else if (propertyName == "UseDigitalMusic")
		return UseDigitalMusic.ToString();
	else if (propertyName == "UseSpatial")
		return UseSpatial.ToString();
	else if (propertyName == "UseReverb")
		return UseReverb.ToString();
	else if (propertyName == "Use3dHardware")
		return Use3dHardware.ToString();
	else if (propertyName == "LowSoundQuality")
		return LowSoundQuality.ToString();
	else if (propertyName == "ReverseStereo")
		return ReverseStereo.ToString();
	else if (propertyName == "Latency")
		return Latency.ToString();
	else if (propertyName == "OutputRate")
		return OutputRate.ToString();
	else if (propertyName == "Channels")
		return Channels.ToString();
	else if (propertyName == "MusicVolume")
		return MusicVolume.ToString();
	else if (propertyName == "SoundVolume")
		return SoundVolume.ToString();
	else if (propertyName == "AmbientFactor")
		return AmbientFactor.ToString();

	engine->LogMessage("Queried unknown property for SurrealAudioDevice: " + propertyName.ToString());
	return {};
}

void USurrealAudioDevice::SetPropertyFromString(const NameString& propertyName, const std::string& value)
{
	if (propertyName == "UseFilter")
		UseFilter.FromString(value);
	else if (propertyName == "UseSurround")
		UseSurround.FromString(value);
	else if (propertyName == "UseStereo")
		UseStereo.FromString(value);
	else if (propertyName == "UseCDMusic")
		UseCDMusic.FromString(value);
	else if (propertyName == "UseDigitalMusic")
		UseDigitalMusic.FromString(value);
	else if (propertyName == "UseSpatial")
		UseSpatial.FromString(value);
	else if (propertyName == "UseReverb")
		UseReverb.FromString(value);
	else if (propertyName == "Use3dHardware")
		Use3dHardware.FromString(value);
	else if (propertyName == "LowSoundQuality")
		LowSoundQuality.FromString(value);
	else if (propertyName == "ReverseStereo")
		ReverseStereo.FromString(value);
	else if (propertyName == "Latency")
		Latency.FromString(value);
	else if (propertyName == "OutputRate")
		OutputRate.FromString(value);
	else if (propertyName == "Channels")
		Channels.FromString(value);
	else if (propertyName == "MusicVolume")
		MusicVolume.FromString(value);
	else if (propertyName == "SoundVolume")
		SoundVolume.FromString(value);
	else if (propertyName == "AmbientFactor")
		AmbientFactor.FromString(value);
	else
		engine->LogMessage("Setting unknown property for SurrealAudioDevice: " + propertyName.ToString());

	engine->packages->SetIniValue("System", Class, propertyName, value);
}

void USurrealAudioDevice::LoadProperties(const NameString& from)
{
	NameString name_from = from;

	if (from == "")
		name_from = NameString(Class);

	UseFilter.FromIniFile(*engine->packages->GetIniFile("System"), name_from, "UseFilter");
	UseSurround.FromIniFile(*engine->packages->GetIniFile("System"), name_from, "UseSurround");
	UseStereo.FromIniFile(*engine->packages->GetIniFile("System"), name_from, "UseStereo");
	UseCDMusic.FromIniFile(*engine->packages->GetIniFile("System"), name_from, "UseCDMusic");
	UseDigitalMusic.FromIniFile(*engine->packages->GetIniFile("System"), name_from, "UseDigitalMusic");
	UseSpatial.FromIniFile(*engine->packages->GetIniFile("System"), name_from, "UseSpatial");
	UseReverb.FromIniFile(*engine->packages->GetIniFile("System"), name_from, "UseReverb");
	Use3dHardware.FromIniFile(*engine->packages->GetIniFile("System"), name_from, "Use3dHardware");
	LowSoundQuality.FromIniFile(*engine->packages->GetIniFile("System"), name_from, "LowSoundQuality");
	ReverseStereo.FromIniFile(*engine->packages->GetIniFile("System"), name_from, "ReverseStereo");
	Latency.FromIniFile(*engine->packages->GetIniFile("System"), name_from, "Latency");
	OutputRate.FromIniFile(*engine->packages->GetIniFile("System"), name_from, "OutputRate");
	Channels.FromIniFile(*engine->packages->GetIniFile("System"), name_from, "Channels");
	MusicVolume.FromIniFile(*engine->packages->GetIniFile("System"), name_from, "MusicVolume");
	SoundVolume.FromIniFile(*engine->packages->GetIniFile("System"), name_from, "SoundVolume");
	AmbientFactor.FromIniFile(*engine->packages->GetIniFile("System"), name_from, "AmbientFactor");
	/*
	UseFilter = std::atoi(engine->packages->GetIniValue("System", name_from, "UseFilter", UseFilter.ToString()).c_str());
	UseSurround = std::atoi(engine->packages->GetIniValue("System", name_from, "UseSurround", UseSurround.ToString()).c_str());
	UseStereo = std::atoi(engine->packages->GetIniValue("System", name_from, "UseStereo", UseStereo.ToString()).c_str());
	UseCDMusic = std::atoi(engine->packages->GetIniValue("System", name_from, "UseCDMusic", UseCDMusic.ToString()).c_str());
	UseDigitalMusic = std::atoi(engine->packages->GetIniValue("System", name_from, "UseDigitalMusic", UseDigitalMusic.ToString()).c_str());
	UseSpatial = std::atoi(engine->packages->GetIniValue("System", name_from, "UseSpatial", UseSpatial.ToString()).c_str());
	UseReverb = std::atoi(engine->packages->GetIniValue("System", name_from, "UseReverb", UseReverb.ToString()).c_str());
	Use3dHardware = std::atoi(engine->packages->GetIniValue("System", name_from, "Use3dHardware", Use3dHardware.ToString()).c_str());
	LowSoundQuality = std::atoi(engine->packages->GetIniValue("System", name_from, "LowSoundQuality", LowSoundQuality.ToString()).c_str());
	ReverseStereo = std::atoi(engine->packages->GetIniValue("System", name_from, "ReverseStereo", ReverseStereo.ToString()).c_str());
	Latency = std::atoi(engine->packages->GetIniValue("System", name_from, "Latency", Latency.ToString()).c_str());
	OutputRate = std::atoi(engine->packages->GetIniValue("System", name_from, "OutputRate", OutputRate.ToString()).c_str());
	Channels = std::atoi(engine->packages->GetIniValue("System", name_from, "Channels", Channels.ToString()).c_str());
	MusicVolume = std::atoi(engine->packages->GetIniValue("System", name_from, "MusicVolume", MusicVolume.ToString()).c_str());
	SoundVolume = std::atoi(engine->packages->GetIniValue("System", name_from, "SoundVolume", SoundVolume.ToString()).c_str());
	AmbientFactor = (float)std::atof(engine->packages->GetIniValue("System", name_from, "AmbientFactor", AmbientFactor.ToString()).c_str());
	*/
}

void USurrealAudioDevice::SaveProperties()
{
	engine->packages->SetIniValue("System", Class, "UseFilter", UseFilter.ToString());
	engine->packages->SetIniValue("System", Class, "UseSurround", UseSurround.ToString());
	engine->packages->SetIniValue("System", Class, "UseStereo", UseStereo.ToString());
	engine->packages->SetIniValue("System", Class, "UseCDMusic", UseCDMusic.ToString());
	engine->packages->SetIniValue("System", Class, "UseDigitalMusic", UseDigitalMusic.ToString());
	engine->packages->SetIniValue("System", Class, "UseSpatial", UseSpatial.ToString());
	engine->packages->SetIniValue("System", Class, "UseReverb", UseReverb.ToString());
	engine->packages->SetIniValue("System", Class, "Use3dHardware", Use3dHardware.ToString());
	engine->packages->SetIniValue("System", Class, "LowSoundQuality", LowSoundQuality.ToString());
	engine->packages->SetIniValue("System", Class, "ReverseStereo", ReverseStereo.ToString());
	engine->packages->SetIniValue("System", Class, "Latency", Latency.ToString());
	engine->packages->SetIniValue("System", Class, "OutputRate", OutputRate.ToString());
	engine->packages->SetIniValue("System", Class, "Channels", Channels.ToString());
	engine->packages->SetIniValue("System", Class, "MusicVolume", MusicVolume.ToString());
	engine->packages->SetIniValue("System", Class, "SoundVolume", SoundVolume.ToString());
	engine->packages->SetIniValue("System", Class, "AmbientFactor", AmbientFactor.ToString());
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

std::string USurrealClient::GetPropertyAsString(const NameString& propertyName) const
{
	if (propertyName == "Class")
		return "class'" + Class + "'";
	else if (propertyName == "StartupFullscreen")
		return StartupFullscreen ? "1" : "0";
	else if (propertyName == "WindowedViewportX")
		return std::to_string(WindowedViewportX);
	else if (propertyName == "WindowedViewportY")
		return std::to_string(WindowedViewportY);
	else if (propertyName == "WindowedColorBits")
		return std::to_string(WindowedColorBits);
	else if (propertyName == "FullscreenViewportX")
		return std::to_string(FullscreenViewportX);
	else if (propertyName == "FullscreenViewportY")
		return std::to_string(FullscreenViewportY);
	else if (propertyName == "FullscreenColorBits")
		return std::to_string(FullscreenColorBits);
	else if (propertyName == "Brightness")
		return std::to_string(Brightness);
	else if (propertyName == "UseJoystick")
		return UseJoystick ? "1" : "0";
	else if (propertyName == "UseDirectInput")
		return UseDirectInput ? "1" : "0";
	else if (propertyName == "MinDesiredFrameRate")
		return std::to_string(MinDesiredFrameRate);
	else if (propertyName == "Decals")
		return Decals ? "1" : "0";
	else if (propertyName == "NoDynamicLights")
		return NoDynamicLights ? "1" : "0";
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
		WindowedViewportX = std::atoi(value.c_str());
	else if (propertyName == "WindowedViewportY")
		WindowedViewportY = std::atoi(value.c_str());
	else if (propertyName == "WindowedColorBits")
		WindowedColorBits = std::atoi(value.c_str());
	else if (propertyName == "FullscreenViewportX")
		FullscreenViewportX = std::atoi(value.c_str());
	else if (propertyName == "FullscreenViewportY")
		FullscreenViewportY = std::atoi(value.c_str());
	else if (propertyName == "FullscreenColorBits")
		FullscreenColorBits = std::atoi(value.c_str());
	else if (propertyName == "Brightness")
		Brightness = (float)std::atof(value.c_str());
	else if (propertyName == "UseJoystick")
		UseJoystick = std::atoi(value.c_str());
	else if (propertyName == "UseDirectInput")
		UseDirectInput = std::atoi(value.c_str());
	else if (propertyName == "MinDesiredFrameRate")
		MinDesiredFrameRate = std::atoi(value.c_str());
	else if (propertyName == "Decals")
		Decals = std::atoi(value.c_str());
	else if (propertyName == "NoDynamicLights")
		NoDynamicLights = std::atoi(value.c_str());
	else if (propertyName == "TextureDetail")
		TextureDetail = value;
	else if (propertyName == "SkinDetail")
		SkinDetail = value;
	else
		engine->LogMessage("Setting unknown property for Surreal.ViewportManager: " + propertyName.ToString());

	engine->packages->SetIniValue("System", Class, propertyName, value);
}
