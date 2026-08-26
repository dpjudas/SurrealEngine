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
	void Setup(UModel* model, const Coords& mapCoords, int lightMap);
	void SetAmbientLight(UZoneInfo* zoneActor);
	void AddStaticLights(UModel* model, int lightMap);
	void AddDynamicLights(UModel* model, int lightMap, const Array<UActor*>& lights);

	void LoadStaticLight(const Array<vec3>& staticLightColors);
	void SaveStaticLight(Array<vec3>& staticLightColors);

	int Width() const { return width; }
	int Height() const { return height; }
	const vec3* Pixels() const { return lightcolors.data(); }

	static vec3 GetLightColor(UActor* light);

private:
	const vec3* WorldLocations() const { return points.data(); }
	const vec3& WorldNormal() const { return normal; }

	void CalcWorldLocations(Coords MapCoords, const LightMapIndex& lmindex);

	void AddLightContribution(UActor* light);

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
