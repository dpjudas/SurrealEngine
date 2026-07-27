
#include "Precomp.h"
#include "USurrealClient.h"
#include "Engine.h"
#include "Package/PackageManager.h"
#include "Package/IniProperty.h"

void USurrealClient::LoadProperties(const NameString& from)
{
	NameString name_from = from;

	if (from == "")
		name_from = NameString(Class);

	if (engine->LaunchInfo.ue1Version > 219) // This seems to set WindowedViewportX and WindowedViewportY to zero for KHG!!!!
	{
		StartupFullscreen = IniPropertyConverter<bool>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "StartupFullscreen", StartupFullscreen);
		WindowedViewportX = IniPropertyConverter<int>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "WindowedViewportX", WindowedViewportX);
		WindowedViewportY = IniPropertyConverter<int>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "WindowedViewportY", WindowedViewportY);
		WindowedColorBits = IniPropertyConverter<int>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "WindowedColorBits", WindowedColorBits);
		FullscreenViewportX = IniPropertyConverter<int>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "FullscreenViewportX", FullscreenViewportX);
		FullscreenViewportY = IniPropertyConverter<int>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "FullscreenViewportY", FullscreenViewportY);
		FullscreenColorBits = IniPropertyConverter<int>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "FullscreenColorBits", FullscreenColorBits);
		Brightness = IniPropertyConverter<float>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "Brightness", Brightness);
		UseJoystick = IniPropertyConverter<bool>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "UseJoystick", UseJoystick);
		UseDirectInput = IniPropertyConverter<bool>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "UseDirectInput", UseDirectInput);
		MinDesiredFrameRate = IniPropertyConverter<int>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "MinDesiredFrameRate", MinDesiredFrameRate);
		Decals = IniPropertyConverter<bool>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "Decals", Decals);
		NoDynamicLights = IniPropertyConverter<bool>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "NoDynamicLights", NoDynamicLights);
		TextureDetail = IniPropertyConverter<std::string>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "TextureDetail", TextureDetail);
		SkinDetail = IniPropertyConverter<std::string>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "SkinDetail", SkinDetail);
	}
}

void USurrealClient::SaveConfig()
{
	if (engine->LaunchInfo.ue1Version > 219)
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

	LogMessage("Queried unknown property for Surreal.ViewportManager: " + propertyName.ToString());
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
		LogMessage("Setting unknown property for Surreal.ViewportManager: " + propertyName.ToString());

	engine->packages->SetIniValue("System", Class, propertyName, value);
}
