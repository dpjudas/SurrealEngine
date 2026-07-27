#pragma once

#include "UClient.h"

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
