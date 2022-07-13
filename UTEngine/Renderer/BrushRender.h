#pragma once

#include "Math/vec.h"

struct FSceneNode;
class UModel;
class BspNode;
class Rotator;
class UActor;

class BrushRender
{
public:
	void DrawBrush(FSceneNode* frame, UActor* actor);

private:
	void DrawNodeSurfaceGouraud(UModel* model, const BspNode& node, int pass, const vec3& color = { 0.0f });
};
