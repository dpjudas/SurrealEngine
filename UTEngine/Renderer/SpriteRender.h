#pragma once

#include "Math/vec.h"

struct FSceneNode;
class UTexture;
class Rotator;

class SpriteRender
{
public:
	void DrawSprite(FSceneNode* frame, UTexture* texture, const vec3& location, const Rotator& rotation, float drawscale);
};
