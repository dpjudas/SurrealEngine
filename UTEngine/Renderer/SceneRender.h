#pragma once

#include "Math/vec.h"
#include "Math/mat.h"

struct FSceneNode;
class BspSurface;
class BspNode;
class FrustumPlanes;
class UModel;
class UZoneInfo;

class SceneRender
{
public:
	void DrawScene();
	void DrawNode(FSceneNode* frame, const BspNode& node, const FrustumPlanes& clip, uint64_t zonemask, int pass);
	void DrawNodeSurface(FSceneNode* frame, UModel* model, const BspNode& node, int pass);

	int FindZoneAt(vec3 location);
	int FindZoneAt(const vec4& location, BspNode* node, BspNode* nodes);

	uint64_t FindRenderZoneMask(FSceneNode* frame, const BspNode& node, const FrustumPlanes& clip, int zone);
	uint64_t FindRenderZoneMaskForPortal(FSceneNode* frame, const BspNode& node, const FrustumPlanes& clip, int portalzone);

private:
	mat4 CoordsMatrix();

	FSceneNode CreateSceneFrame();
	FSceneNode CreateSkyFrame();

	std::vector<int> portalsvisited;
	UZoneInfo* cameraZoneActor = nullptr;
};
