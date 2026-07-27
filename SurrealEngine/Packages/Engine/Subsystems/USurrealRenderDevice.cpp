
#include "Precomp.h"
#include "USurrealRenderDevice.h"
#include "Engine.h"
#include "Package/PackageManager.h"
#include "Package/IniProperty.h"

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

	LogMessage("Queried unknown property for SurrealRenderDevice: " + propertyName.ToString());
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
		LogMessage("Setting unknown property for SurrealRenderDevice: " + propertyName.ToString());

	engine->packages->SetIniValue("System", Class, propertyName, value);
}

void USurrealRenderDevice::LoadProperties(const NameString& from)
{
	NameString name_from = from;

	if (from == "")
		name_from = NameString(Class);

	Translucency = IniPropertyConverter<bool>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "Translucency", Translucency);
	VolumetricLighting = IniPropertyConverter<bool>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "VolumetricLighting", VolumetricLighting);
	ShinySurfaces = IniPropertyConverter<bool>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "ShinySurfaces", ShinySurfaces);
	Coronas = IniPropertyConverter<bool>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "Coronas", Coronas);
	HighDetailActors = IniPropertyConverter<bool>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "HighDetailActors", HighDetailActors);
}

void USurrealRenderDevice::SaveConfig()
{
	engine->packages->SetIniValue("System", Class, "Translucency", IniPropertyConverter<bool>::ToString(Translucency));
	engine->packages->SetIniValue("System", Class, "VolumetricLighting", IniPropertyConverter<bool>::ToString(VolumetricLighting));
	engine->packages->SetIniValue("System", Class, "ShinySurfaces", IniPropertyConverter<bool>::ToString(ShinySurfaces));
	engine->packages->SetIniValue("System", Class, "Coronas", IniPropertyConverter<bool>::ToString(Coronas));
	engine->packages->SetIniValue("System", Class, "HighDetailActors", IniPropertyConverter<bool>::ToString(HighDetailActors));
}
