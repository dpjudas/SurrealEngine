
#include "Precomp.h"
#include "FogmapBuilder.h"
#include "UObject/ULevel.h"
#include "UObject/UActor.h"
#include "RenderDevice/RenderDevice.h"
#include "Math/hsb.h"

#ifndef NOSSE
#include <immintrin.h>
#endif

void FogmapBuilder::Setup(UModel* model, const BspSurface& surface, UZoneInfo* zoneActor)
{
	const LightMapIndex& lmindex = model->LightMap[surface.LightMap];

	width = lmindex.UClamp;
	height = lmindex.VClamp;

	// Stop allocations over time by building up a reserve

	size_t size = (size_t)width * height;
	if (points.size() < size)
		points.resize(size);
	if (fogcolors.size() < size)
		fogcolors.resize(size);

	CalcWorldLocations(model, surface, lmindex);

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
		light->FogInfo.r2 = light->FogInfo.radius * light->FogInfo.radius;
	}

	vec3 fogcolor = light->FogInfo.fogcolor;
	float brightness = light->FogInfo.brightness;
	float fog = light->FogInfo.fog;
	float radius = light->FogInfo.radius;
	float r2 = light->FogInfo.r2;

	vec3 lightpos = light->Location();

	size_t size = (size_t)width * height;
	const vec3* locations = WorldLocations();
	vec4* dest = fogcolors.data();
	for (size_t i = 0; i < size; i++)
	{
		// Find the one or two points where the light intersects with our ray from the texel to the view
		vec3 ijk = locations[i] - view;
		vec3 tmp = view - lightpos;
		float a = dot(ijk, ijk);
		float b = 2.0f * dot(ijk, tmp);
		float c = dot(tmp, tmp) - r2;
		float det = b * b - 4.0f * a * c;
		if (det < 0.0f)
			continue; // no hit, we are outside the fog
#ifndef NOSSE
		float sqrdet = _mm_cvtss_f32(_mm_sqrt_ss(_mm_set_ss(det)));
#else
		float sqrdet = std::sqrt(det);
#endif
		float t0 = (-b - sqrdet) / (2.0f * a);
		float t1 = (-b + sqrdet) / (2.0f * a);
		t0 = clamp(t0, 0.0f, 1.0f);
		t1 = clamp(t1, 0.0f, 1.0f);
		vec3 fogp0 = view + ijk * t0;
		vec3 fogp1 = view + ijk * t1;
		tmp = fogp1 - fogp0;
		float fogdistance2 = dot(tmp, tmp);

#if 0
#ifndef NOSSE
		float fogdistance = _mm_cvtss_f32(_mm_sqrt_ss(_mm_set_ss(fogdistance2)));
#else
		float fogdistance = std::sqrt(fogdistance2);
#endif
		float fogamount = std::min(fogdistance / radius, 1.0f) * brightness;
#else
		float fogamount = std::min(fogdistance2 / r2, 1.0f) * brightness;
#endif
		float alpha = std::min(fog * fogamount, 1.0f);
		float invalpha = 1.0f - alpha;
		dest[i].r = fogcolor.r * fogamount + dest[i].r * invalpha;
		dest[i].g = fogcolor.g * fogamount + dest[i].g * invalpha;
		dest[i].b = fogcolor.b * fogamount + dest[i].b * invalpha;
		dest[i].a = std::min(dest[i].a + alpha, 1.0f);
	}
}

void FogmapBuilder::CalcWorldLocations(UModel* model, const BspSurface& surface, const LightMapIndex& lmindex)
{
	// Note: this could be simplified a lot for better performance

	// Allow optimizer to move them into registers
	int width = this->width;
	int height = this->height;

	FSurfaceFacet facet;
	facet.MapCoords.Origin = model->Points[surface.pBase];
	facet.MapCoords.XAxis = model->Vectors[surface.vTextureU];
	facet.MapCoords.YAxis = model->Vectors[surface.vTextureV];

	float UDot = dot(facet.MapCoords.XAxis, facet.MapCoords.Origin);
	float VDot = dot(facet.MapCoords.YAxis, facet.MapCoords.Origin);
	float LMUPan = UDot + lmindex.PanX - 0.5f * lmindex.UScale;
	float LMVPan = VDot + lmindex.PanY - 0.5f * lmindex.VScale;
	float LMUMult = 1.0f / lmindex.UScale;
	float LMVMult = 1.0f / lmindex.VScale;

	vec3 p[3] =
	{
		facet.MapCoords.Origin,
		facet.MapCoords.Origin + facet.MapCoords.XAxis,
		facet.MapCoords.Origin + facet.MapCoords.YAxis
	};

	vec2 uv[3];
	for (int j = 0; j < 3; j++)
	{
		uv[j] =
		{
			(dot(facet.MapCoords.XAxis, p[j]) - LMUPan) * LMUMult,
			(dot(facet.MapCoords.YAxis, p[j]) - LMVPan) * LMVMult
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
