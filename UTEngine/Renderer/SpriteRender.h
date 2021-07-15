#pragma once

#include "Math/vec.h"

struct FSceneNode;
class UActor;

class SpriteRender
{
public:
	void DrawSprite(FSceneNode* frame, UActor* actor);
};
