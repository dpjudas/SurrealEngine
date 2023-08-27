
#include "Precomp.h"
#include "LightmapBuilder.h"
#include "UObject/ULevel.h"
#include "UObject/UActor.h"
#include "RenderDevice/RenderDevice.h"
#include "Math/hsb.h"

void LightmapBuilder::Setup(UModel* model, const BspSurface& surface, UZoneInfo* zoneActor)
{
	const LightMapIndex& lmindex = model->LightMap[surface.LightMap];

	width = lmindex.UClamp;
	height = lmindex.VClamp;
	normal = model->Vectors[surface.vNormal];
	base = model->Points[surface.pBase];

	// Stop allocations over time by building up a reserve

	size_t size = (size_t)width * height;
	if (points.size() < size)
		points.resize(size);
	if (lightcolors.size() < size)
		lightcolors.resize(size);
	if (illuminationmap.size() < size)
		illuminationmap.resize(size);

	CalcWorldLocations(model, surface, lmindex);

	// Initialize lightmap with the ambient color

	vec3 ambientColor = hsbtorgb(zoneActor->AmbientHue(), zoneActor->AmbientSaturation(), zoneActor->AmbientBrightness()); // To do: is this the correct scale?
	// To do: is there more ambient light than just from the zone?

	for (vec3& c : lightcolors)
		c = ambientColor;

	// To do: how does polyflags affect the lightmap (if at all)?

	//bool isSpecialLit = (surface.PolyFlags & PF_SpecialLit) == PF_SpecialLit;
	//bool isTranslucent = (surface.PolyFlags & PF_Translucent) == PF_Translucent;
}

void LightmapBuilder::AddStaticLights(UModel* model, const BspSurface& surface)
{
	size_t count = (size_t)width * height;

	const LightMapIndex& lmindex = model->LightMap[surface.LightMap];
	if (lmindex.LightActors >= 0)
	{
		UActor** lightlist = &model->Lights[lmindex.LightActors];
		for (int lightindex = 0; lightlist[lightindex] != nullptr; lightindex++)
		{
			UActor* light = lightlist[lightindex];
			if (light->LightType() != LT_None && light->LightBrightness() > 0)
			{
				Shadow.Load(model, surface, lightindex);
				Effect.Run(light, width, height, WorldLocations(), base, WorldNormal(), Shadow.Pixels(), illuminationmap.data());

				vec3 lightcolor = hsbtorgb(light->LightHue(), light->LightSaturation(), light->LightBrightness());

				const float* src = illuminationmap.data();
				vec3* dest = lightcolors.data();
				for (size_t i = 0; i < count; i++)
				{
					vec3 color = src[i] * lightcolor;
					color.r = std::min(color.r, 1.0f);
					color.g = std::min(color.g, 1.0f);
					color.b = std::min(color.b, 1.0f);
					dest[i] += color;
				}
			}
		}
	}
}

void LightmapBuilder::CalcWorldLocations(UModel* model, const BspSurface& surface, const LightMapIndex& lmindex)
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
