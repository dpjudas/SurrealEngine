
#include "Precomp.h"
#include "USubsystem.h"
#include "Engine.h"
#include "Package/PackageManager.h"

std::string USurrealRenderDevice::GetPropertyAsString(const NameString& propertyName) const
{
	if (propertyName == "Class")
		return "class'" + Class + "'";
	if (propertyName == "Translucency")
		return Translucency ? "1" : "0";
	else if (propertyName == "VolumetricLighting")
		return VolumetricLighting ? "1" : "0";
	else if (propertyName == "ShinySurfaces")
		return ShinySurfaces ? "1" : "0";
	else if (propertyName == "Coronas")
		return Coronas ? "1" : "0";
	else if (propertyName == "HighDetailActors")
		return HighDetailActors ? "1" : "0";

	engine->LogMessage("Queried unknown property for SurrealRenderDevice: " + propertyName.ToString());
	return {};
}

void USurrealRenderDevice::SetPropertyFromString(const NameString& propertyName, const std::string& value)
{
	if (propertyName == "Translucency")
		Translucency = std::atoi(value.c_str());
	else if (propertyName == "VolumetricLighting")
		VolumetricLighting = std::atoi(value.c_str());
	else if (propertyName == "ShinySurfaces")
		ShinySurfaces = std::atoi(value.c_str());
	else if (propertyName == "Coronas")
		Coronas = std::atoi(value.c_str());
	else if (propertyName == "HighDetailActors")
		HighDetailActors = std::atoi(value.c_str());
	else
		engine->LogMessage("Setting unknown property for SurrealRenderDevice: " + propertyName.ToString());

	engine->packages->SetIniValue("System", Class, propertyName, value);
}

/////////////////////////////////////////////////////////////////////////////

std::string USurrealAudioDevice::GetPropertyAsString(const NameString& propertyName) const
{
	if (propertyName == "Class")
		return "class'" + Class + "'";
	else if (propertyName == "UseFilter")
		return UseFilter ? "1" : "0";
	else if (propertyName == "UseSurround")
		return UseSurround ? "1" : "0";
	else if (propertyName == "UseStereo")
		return UseStereo ? "1" : "0";
	else if (propertyName == "UseCDMusic")
		return UseCDMusic ? "1" : "0";
	else if (propertyName == "UseDigitalMusic")
		return UseDigitalMusic ? "1" : "0";
	else if (propertyName == "UseSpatial")
		return UseSpatial ? "1" : "0";
	else if (propertyName == "UseReverb")
		return UseReverb ? "1" : "0";
	else if (propertyName == "Use3dHardware")
		return Use3dHardware ? "1" : "0";
	else if (propertyName == "LowSoundQuality")
		return LowSoundQuality ? "1" : "0";
	else if (propertyName == "ReverseStereo")
		return ReverseStereo ? "1" : "0";
	else if (propertyName == "Latency")
		return std::to_string(Latency);
	else if (propertyName == "OutputRate")
		return std::to_string(OutputRate);
	else if (propertyName == "Channels")
		return std::to_string(Channels);
	else if (propertyName == "MusicVolume")
		return std::to_string(MusicVolume);
	else if (propertyName == "SoundVolume")
		return std::to_string(SoundVolume);
	else if (propertyName == "AmbientFactor")
		return std::to_string(AmbientFactor);

	engine->LogMessage("Queried unknown property for SurrealAudioDevice: " + propertyName.ToString());
	return {};
}

void USurrealAudioDevice::SetPropertyFromString(const NameString& propertyName, const std::string& value)
{
	if (propertyName == "UseFilter")
		UseFilter = std::atoi(value.c_str());
	else if (propertyName == "UseSurround")
		UseSurround = std::atoi(value.c_str());
	else if (propertyName == "UseStereo")
		UseStereo = std::atoi(value.c_str());
	else if (propertyName == "UseCDMusic")
		UseCDMusic = std::atoi(value.c_str());
	else if (propertyName == "UseDigitalMusic")
		UseDigitalMusic = std::atoi(value.c_str());
	else if (propertyName == "UseSpatial")
		UseSpatial = std::atoi(value.c_str());
	else if (propertyName == "UseReverb")
		UseReverb = std::atoi(value.c_str());
	else if (propertyName == "Use3dHardware")
		Use3dHardware = std::atoi(value.c_str());
	else if (propertyName == "LowSoundQuality")
		LowSoundQuality = std::atoi(value.c_str());
	else if (propertyName == "ReverseStereo")
		ReverseStereo = std::atoi(value.c_str());
	else if (propertyName == "Latency")
		Latency = std::atoi(value.c_str());
	else if (propertyName == "OutputRate")
		OutputRate = std::atoi(value.c_str());
	else if (propertyName == "Channels")
		Channels = std::atoi(value.c_str());
	else if (propertyName == "MusicVolume")
		MusicVolume = std::atoi(value.c_str());
	else if (propertyName == "SoundVolume")
		SoundVolume = std::atoi(value.c_str());
	else if (propertyName == "AmbientFactor")
		AmbientFactor = (float)std::atof(value.c_str());
	else
		engine->LogMessage("Setting unknown property for SurrealAudioDevice: " + propertyName.ToString());

	engine->packages->SetIniValue("System", Class, propertyName, value);
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
