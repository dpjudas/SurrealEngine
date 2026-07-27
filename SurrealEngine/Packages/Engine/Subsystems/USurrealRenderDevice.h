#pragma once

#include "URenderDevice.h"

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
