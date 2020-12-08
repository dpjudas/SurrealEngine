#pragma once

#include "UObject.h"

class UTexture;
class UMesh;

enum class ActorDrawType
{
	None,
	Sprite,
	Mesh,
	Brush,
	RopeSprite,
	VerticalSprite,
	Terraform,
	SpriteAnimOnce
};

class UActor : public UObject
{
public:
	UActor(ObjectStream* stream);

	vec3 Location = { 0.0f };
	int LightBrightness = 64;
	int LightHue = 0;
	int LightSaturation = 255;
	int LightRadius = 64;
	bool bCorona = false;

	ActorDrawType DrawType = ActorDrawType::None;
	UTexture* Skin = nullptr;
	UTexture* Texture = nullptr;
	UMesh* Mesh = nullptr;
};
