#pragma once

#include "Math/vec.h"
#include "LightEffect.h"
#include "Shadowmap.h"

class BspSurface;
class LightMapIndex;
class UModel;
class UZoneInfo;
class Coords;
struct Poly;

class LightmapBuilder
{
public:
	void Setup(UModel* model, const Coords& mapCoords, int lightMap, UZoneInfo* zoneActor);
	void AddStaticLights(UModel* model, int lightMap);

	int Width() const { return width; }
	int Height() const { return height; }
	const vec3* Pixels() const { return lightcolors.data(); }

private:
	const vec3* WorldLocations() const { return points.data(); }
	const vec3& WorldNormal() const { return normal; }

	void CalcWorldLocations(Coords MapCoords, const LightMapIndex& lmindex);

	int width = 0;
	int height = 0;
	Array<vec3> lightcolors;

	Array<vec3> points;
	vec3 normal;
	vec3 base;

	Shadowmap Shadow;
	LightEffect Effect;
	Array<float> illuminationmap;
};
