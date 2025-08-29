
#include "Precomp.h"
#include "FogmapBuilder.h"
#include "UObject/ULevel.h"
#include "UObject/UActor.h"
#include "RenderDevice/RenderDevice.h"
#include "Math/hsb.h"

#ifndef NOSSE
#include <immintrin.h>
#endif

void FogmapBuilder::Setup(UModel* model, const Coords& mapCoords, int lightMap, UZoneInfo* zoneActor)
{
	const LightMapIndex& lmindex = model->LightMap[lightMap];

	width = lmindex.UClamp;
	height = lmindex.VClamp;

	// Stop allocations over time by building up a reserve

	size_t size = (size_t)width * height;
	if (points.size() < size)
		points.resize(size);
	if (fogcolors.size() < size)
		fogcolors.resize(size);

	CalcWorldLocations(mapCoords, lmindex);

	// Initialize fogmap with full transparency

	vec4 zero(0.0f);
	for (vec4& c : fogcolors)
		c = zero;
}

void FogmapBuilder::AddLight(UActor* light, vec3 view)
{
	if (light->FogInfo.brightness < 0.0f)
	{
		light->FogInfo.fogcolor = hsbtorgb(light->LightHue(), light->LightSaturation(), light->LightBrightness());
		light->FogInfo.brightness = light->LightBrightness() * (1.0f / 255.0f) * light->VolumeBrightness() * (1.0f / 64.0f);
		light->FogInfo.fog = light->VolumeFog() * (1.0f / 255.0f);
		light->FogInfo.radius = light->WorldVolumetricRadius();
	}

	vec3 fogcolor = light->FogInfo.fogcolor;
	float brightness = light->FogInfo.brightness * 5.0f;
	float fog = light->FogInfo.fog;
	float radius = light->FogInfo.radius;

	vec3 lightpos = light->Location();

	size_t size = (size_t)width * height;
	const vec3* locations = WorldLocations();
	vec4* dest = fogcolors.data();
	for (size_t i = 0; i < size; i++)
	{
		vec3 rayDirection = locations[i] - view;
		float depth = std::sqrt(dot(rayDirection, rayDirection));
		rayDirection *= (1.0f / depth);
		float fogamount = SphereDensity(view, rayDirection, lightpos, radius, depth) * brightness;

		float alpha = std::min(fogamount * fog, 1.0f);
		float invalpha = 1.0f - alpha;
		dest[i].r = fogcolor.r * fogamount + dest[i].r * invalpha;
		dest[i].g = fogcolor.g * fogamount + dest[i].g * invalpha;
		dest[i].b = fogcolor.b * fogamount + dest[i].b * invalpha;
		dest[i].a = std::min(dest[i].a + alpha, 1.0f);
	}
}

// The MIT License
// https://www.youtube.com/c/InigoQuilez
// https://iquilezles.org/
// Copyright (c) 2015 Inigo Quilez
//
// Analytically integrating quadratically decaying participating media within a sphere.
// Related info: https://iquilezles.org/articles/spherefunctions
//
float FogmapBuilder::SphereDensity(const vec3& rayOrigin, const vec3& rayDirection, const vec3& sphereCenter, float sphereRadius, float dbuffer)
{
	// normalize the problem to the canonical sphere
	float ndbuffer = dbuffer / sphereRadius;
	vec3 rc = (rayOrigin - sphereCenter) / sphereRadius;

	// find intersection with sphere
	float b = dot(rayDirection, rc);
	float c = dot(rc, rc) - 1.0f;
	float h = b * b - c;

	// not intersecting
	if (h < 0.0f) return 0.0f;

	h = std::sqrt(h);

	//return h*h*h;

	float t1 = -b - h;
	float t2 = -b + h;

	// not visible (behind camera or behind ndbuffer)
	if (t2 < 0.0f || t1 > ndbuffer) return 0.0f;

	// clip integration segment from camera to ndbuffer
	t1 = std::max(t1, 0.0f);
	t2 = std::min(t2, ndbuffer);

	// analytical integration of an inverse squared density
	float i1 = -(c * t1 + b * t1 * t1 + t1 * t1 * t1 * (1.0f / 3.0f));
	float i2 = -(c * t2 + b * t2 * t2 + t2 * t2 * t2 * (1.0f / 3.0f));
	return (i2 - i1) * (3.0f / 4.0f);
}

void FogmapBuilder::CalcWorldLocations(Coords MapCoords, const LightMapIndex& lmindex)
{
	// Note: this could be simplified a lot for better performance

	// Allow optimizer to move them into registers
	int width = this->width;
	int height = this->height;

	float UDot = dot(MapCoords.XAxis, MapCoords.Origin);
	float VDot = dot(MapCoords.YAxis, MapCoords.Origin);
	float LMUPan = UDot + lmindex.PanX - 0.5f * lmindex.UScale;
	float LMVPan = VDot + lmindex.PanY - 0.5f * lmindex.VScale;
	float LMUMult = 1.0f / lmindex.UScale;
	float LMVMult = 1.0f / lmindex.VScale;

	vec3 p[3] =
	{
		MapCoords.Origin,
		MapCoords.Origin + MapCoords.XAxis,
		MapCoords.Origin + MapCoords.YAxis
	};

	vec2 uv[3];
	for (int j = 0; j < 3; j++)
	{
		uv[j] =
		{
			(dot(MapCoords.XAxis, p[j]) - LMUPan) * LMUMult,
			(dot(MapCoords.YAxis, p[j]) - LMVPan) * LMVMult
		};
	}

	float leftDX = uv[2].x - uv[0].x;
	float leftDY = uv[2].y - uv[0].y;
	float leftStep = leftDX / leftDY;
	float rightDX = uv[2].x - uv[1].x;
	float rightDY = uv[2].y - uv[1].y;
	float rightStep = rightDX / rightDY;

	for (int y = 0; y < height; y++)
	{
		float x0 = uv[0].x + leftStep * (y + 0.5f - uv[0].y) + 0.5f;
		float x1 = uv[1].x + rightStep * (y + 0.5f - uv[1].y) + 0.5f;
		float t0 = (y + 0.5f - uv[0].y) / leftDY;
		float t1 = (y + 0.5f - uv[1].y) / rightDY;
		vec3 p0 = mix(p[0], p[2], t0);
		vec3 p1 = mix(p[1], p[2], t1);
		if (x1 < x0)
		{
			std::swap(x0, x1);
			std::swap(p0, p1);
		}

		vec3* dest = &points[y * width];
		for (int i = 0; i < width; i++)
		{
			float t = (i + 0.5f - x0) / (x1 - x0);
			dest[i] = mix(p0, p1, t);
		}
	}
}
