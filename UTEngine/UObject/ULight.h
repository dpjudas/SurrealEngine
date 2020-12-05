#pragma once

#include "UObject.h"

class UTexture;

class ULight : public UObject
{
public:
	ULight(ObjectStream* stream);

	vec3 Location = { 0.0f };
	int LightBrightness = 64;
	int LightHue = 0;
	int LightSaturation = 255;
	int LightRadius = 64;
	bool bCorona = false;
	UTexture* Skin = nullptr;
};
