#pragma once

#include "Math/vec.h"

class BspSurface;
class LightMapIndex;
class UModel;
class UZoneInfo;
class UActor;

class FogmapBuilder
{
public:
	void Setup(UModel* model, const BspSurface& surface, UZoneInfo* zoneActor);
	void AddLight(UActor* light, vec3 view);

	int Width() const { return width; }
	int Height() const { return height; }
	const vec4* Pixels() const { return fogcolors.data(); }

private:
	const vec3* WorldLocations() const { return points.data(); }

	void CalcWorldLocations(UModel* model, const BspSurface& surface, const LightMapIndex& lmindex);

	int width = 0;
	int height = 0;
	std::vector<vec4> fogcolors;
	std::vector<vec3> points;
};
