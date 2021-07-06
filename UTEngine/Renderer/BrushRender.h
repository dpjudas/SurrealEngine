#pragma once

#include "Math/vec.h"

struct FSceneNode;
class UModel;
class BspNode;
class Rotator;

class BrushRender
{
public:
	void DrawBrush(FSceneNode* frame, UModel* brush, const vec3& location, const Rotator& rotation, float drawscale, int zoneIndex);

private:
	void DrawNodeSurfaceGouraud(FSceneNode* frame, UModel* model, const BspNode& node, int pass, const vec3& color = { 0.0f });
};
