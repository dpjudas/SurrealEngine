#pragma once

#include "Math/vec.h"

class UZoneInfo;
class UModel;
class UActor;
class UTexture;
class BspSurface;
class BspNode;
class LightMapIndex;
struct FTextureInfo;
struct FSceneNode;
struct FSurfaceFacet;

class LightRender
{
public:
	vec3 FindLightAt(const vec3& location, int zoneIndex);
	vec4 FindFogAt(const vec3& location);
	FTextureInfo GetSurfaceLightmap(BspSurface& surface, const FSurfaceFacet& facet, UZoneInfo* zoneActor, UModel* model);
	FTextureInfo GetSurfaceFogmap(BspSurface& surface, const FSurfaceFacet& facet, UZoneInfo* zoneActor, UModel* model);

private:
	UTexture* CreateLightmapTexture(const LightMapIndex& lmindex, const BspSurface& surface, UZoneInfo* zoneActor, UModel* model);
	void DrawLightmapSpan(vec3* line, int start, int end, float x0, float x1, vec3 p0, vec3 p1, UActor* light, const vec3& N, const uint8_t* bits, int& bitpos);

	void UpdateFogmapTexture(const LightMapIndex& lmindex, uint32_t* texels, const BspSurface& surface, UZoneInfo* zoneActor, UModel* model);
	void DrawFogmapSpan(vec4* line, int start, int end, float x0, float x1, vec3 p0, vec3 p1, UActor* light);
};
