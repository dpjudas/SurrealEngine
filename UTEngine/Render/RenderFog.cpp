
#include "Precomp.h"
#include "RenderSubsystem.h"
#include "RenderDevice/RenderDevice.h"
#include "Engine.h"
#include "Math/hsb.h"

#ifndef NOSSE
#include <immintrin.h>
#endif

#define NOFOG // The current fog implementation is just too slow!

FTextureInfo RenderSubsystem::GetSurfaceFogmap(BspSurface& surface, const FSurfaceFacet& facet, UZoneInfo* zoneActor, UModel* model)
{
#ifdef NOFOG
	return {};
#else
	if (!zoneActor->bFogZone() || surface.LightMap < 0)
		return {};

	auto level = engine->Level;
	const LightMapIndex& lmindex = level->Model->LightMap[surface.LightMap];
	auto& fogtex = Light.fogtextures[surface.LightMap];
	std::unique_ptr<LightmapTexture>& fogtexture = fogtex.second;
	if (!fogtexture)
	{
		UnrealMipmap fogmip;
		fogmip.Width = lmindex.UClamp;
		fogmip.Height = lmindex.VClamp;
		fogmip.Data.resize((size_t)fogmip.Width * fogmip.Height * 4);

		fogtexture = std::make_unique<LightmapTexture>();
		fogtexture->Format = TextureFormat::BGRA8_LM;
		fogtexture->Mip = std::move(fogmip);
	}

	bool firstDrawThisScene = false;
	if (fogtex.first != sceneDrawNumber)
	{
		firstDrawThisScene = true;
		fogtex.first = sceneDrawNumber;
	}

	if (firstDrawThisScene)
		UpdateFogmapTexture(lmindex, (uint32_t*)fogtexture->Mip.Data.data(), surface, zoneActor, model);

	FTextureInfo texinfo;
	texinfo.CacheID = ((uint64_t)(ptrdiff_t)&surface) | 1;
	texinfo.bRealtimeChanged = firstDrawThisScene;
	texinfo.Format = fogtexture->Format;
	texinfo.Mips = &fogtexture->Mip;
	texinfo.NumMips = 1;
	texinfo.USize = texinfo.Mips[0].Width;
	texinfo.VSize = texinfo.Mips[0].Height;
	texinfo.Pan = { lmindex.PanX, lmindex.PanY };
	texinfo.UScale = lmindex.UScale;
	texinfo.VScale = lmindex.VScale;
	return texinfo;
#endif
}

void RenderSubsystem::UpdateFogmapTexture(const LightMapIndex& lmindex, uint32_t* texels, const BspSurface& surface, UZoneInfo* zoneActor, UModel* model)
{
	int width = lmindex.UClamp;
	int height = lmindex.VClamp;

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

	std::vector<vec4> fogcolors;
	fogcolors.resize((size_t)width * height, vec4(0.0f));

	for (UActor* light : Light.Lights)
	{
		if (light && light->VolumeRadius() != 0)
		{
			for (int y = 0; y < height; y++)
			{
				float x0 = uv[0].x + leftStep * (y + 0.5f - uv[0].y) + 0.5f;
				float x1 = uv[1].x + rightStep * (y + 0.5f - uv[1].y) + 0.5f;
				float t0 = (y + 0.5f - uv[0].y) / leftDY;
				float t1 = (y + 0.5f - uv[1].y) / rightDY;
				vec3 p0 = mix(p[0], p[2], t0);
				vec3 p1 = mix(p[1], p[2], t1);
				if (x1 < x0) { std::swap(x0, x1); std::swap(p0, p1); }
				DrawFogmapSpan(&fogcolors[(size_t)y * width], 0, width, x0, x1, p0, p1, light);
			}
		}
	}

	for (int y = 0; y < height; y++)
	{
		vec4* src = &fogcolors[(size_t)y * width];
		for (int x = 0; x < width; x++)
		{
			vec4& color = src[x];

			uint32_t red = (uint32_t)clamp(color.r * 255.0f + 0.5f, 0.0f, 255.0f);
			uint32_t green = (uint32_t)clamp(color.g * 255.0f + 0.5f, 0.0f, 255.0f);
			uint32_t blue = (uint32_t)clamp(color.b * 255.0f + 0.5f, 0.0f, 255.0f);
			uint32_t alpha = (uint32_t)clamp(color.a * 255.0f + 0.5f, 0.0f, 255.0f);

			alpha >>= 1; // format is RGBA8887 and not RGBA8888!

			texels[x] = (alpha << 24) | (red << 16) | (green << 8) | blue;
		}
		texels += width;
	}
}

void RenderSubsystem::DrawFogmapSpan(vec4* line, int start, int end, float x0, float x1, vec3 p0, vec3 p1, UActor* light)
{
	if (light->brightness < 0.0f)
	{
		light->fogcolor = hsbtorgb(light->LightHue(), light->LightSaturation(), light->LightBrightness());
		light->brightness = light->LightBrightness() * (1.0f / 255.0f) * light->VolumeBrightness() * (1.0f / 64.0f);
		light->fog = light->VolumeFog() * (1.0f / 255.0f);
		light->radius = light->VolumeRadius() * 32.0f;
		light->r2 = light->radius * light->radius;
	}

	vec3 fogcolor = light->fogcolor;
	float brightness = light->brightness;
	float fog = light->fog;
	float radius = light->radius;
	float r2 = light->r2;

	vec3 view = engine->CameraLocation;
	vec3 lightpos = light->Location();

	for (int i = start; i < end; i++)
	{
		float t = (i + 0.5f - x0) / (x1 - x0);
		vec3 point = mix(p0, p1, t);

		// Find the one or two points where the light intersects with our ray from the texel to the view
		vec3 ijk = point - view;
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
		line[i].r = fogcolor.r * fogamount + line[i].r * invalpha;
		line[i].g = fogcolor.g * fogamount + line[i].g * invalpha;
		line[i].b = fogcolor.b * fogamount + line[i].b * invalpha;
		line[i].a = std::min(line[i].a + alpha, 1.0f);
	}
}

vec4 RenderSubsystem::FindFogAt(const vec3& location)
{
	vec4 fogcontribution(0.0f);

	for (UActor* light : Light.Lights)
	{
		if (light && light->VolumeRadius() != 0)
		{
			if (light->brightness < 0.0f)
			{
				light->fogcolor = hsbtorgb(light->LightHue(), light->LightSaturation(), light->LightBrightness());
				light->brightness = light->LightBrightness() * (1.0f / 255.0f) * light->VolumeBrightness() * (1.0f / 64.0f);
				light->fog = light->VolumeFog() * (1.0f / 255.0f);
				light->radius = light->VolumeRadius() * 32.0f;
				light->r2 = light->radius * light->radius;
			}

			vec3 fogcolor = light->fogcolor;
			float brightness = light->brightness;
			float fog = light->fog;
			float radius = light->radius;
			float r2 = light->r2;

			vec3 view = engine->CameraLocation;
			vec3 lightpos = light->Location();

			// Find the one or two points where the light intersects with our ray from the texel to the view
			vec3 ijk = location - view;
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
			fogcontribution.r = fogcolor.r * fogamount + fogcontribution.r * invalpha;
			fogcontribution.g = fogcolor.g * fogamount + fogcontribution.g * invalpha;
			fogcontribution.b = fogcolor.b * fogamount + fogcontribution.b * invalpha;
			fogcontribution.a = std::min(fogcontribution.a + alpha, 1.0f);
		}
	}

	return fogcontribution;
}
