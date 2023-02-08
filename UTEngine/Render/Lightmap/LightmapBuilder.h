#pragma once

#include "Math/vec.h"
#include "LightEffect.h"
#include "Shadowmap.h"

class BspSurface;
class LightMapIndex;
class UModel;
class UZoneInfo;

class LightmapBuilder
{
public:
	void Setup(UModel* model, const BspSurface& surface, UZoneInfo* zoneActor);
	void AddStaticLights(UModel* model, const BspSurface& surface);

	int Width() const { return width; }
	int Height() const { return height; }
	const vec3* Pixels() const { return lightcolors.data(); }

private:
	const vec3* WorldLocations() const { return points.data(); }
	const vec3& WorldNormal() const { return normal; }

	void CalcWorldLocations(UModel* model, const BspSurface& surface, const LightMapIndex& lmindex);

	int width = 0;
	int height = 0;
	std::vector<vec3> lightcolors;

	std::vector<vec3> points;
	vec3 normal;

	Shadowmap Shadowmap;
	LightEffect Effect;
	std::vector<float> illuminationmap;
};
