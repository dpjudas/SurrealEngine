
#include "Precomp.h"
#include "LightRender.h"
#include "Package/PackageManager.h"
#include "RenderDevice/RenderDevice.h"
#include "UObject/UActor.h"
#include "UObject/UTexture.h"
#include "UObject/ULevel.h"
#include "Math/hsb.h"
#include "Engine.h"
#include "UTRenderer.h"
#include "Collision.h"

#ifndef NOSSE
#include <immintrin.h>
#endif

FTextureInfo LightRender::GetSurfaceLightmap(BspSurface& surface, const FSurfaceFacet& facet, UZoneInfo* zoneActor, UModel* model)
{
	if (surface.LightMap < 0)
		return {};

	auto level = engine->Level;
	const LightMapIndex& lmindex = level->Model->LightMap[surface.LightMap];
	UTexture*& lmtexture = level->Model->lmtextures[surface.LightMap];
	if (!lmtexture)
	{
		lmtexture = CreateLightmapTexture(lmindex, surface, zoneActor, model);
	}

	FTextureInfo lightmap;
	lightmap.CacheID = (uint64_t)(ptrdiff_t)&surface;
	lightmap.Texture = lmtexture;
	lightmap.Pan = { lmindex.PanX, lmindex.PanY };
	lightmap.UScale = lmindex.UScale;
	lightmap.VScale = lmindex.VScale;
	return lightmap;
}

UTexture* LightRender::CreateLightmapTexture(const LightMapIndex& lmindex, const BspSurface& surface, UZoneInfo* zoneActor, UModel* model)
{
	int width = lmindex.UClamp;
	int height = lmindex.VClamp;

	UnrealMipmap lmmip;
	lmmip.Width = width;
	lmmip.Height = height;
	lmmip.Data.resize((size_t)lmmip.Width * lmmip.Height * 4);

	if (lmindex.LightActors >= 0)
	{
		const vec3& N = model->Vectors[surface.vNormal];

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

		std::vector<vec3> lightcolors;
		lightcolors.resize((size_t)width * height, hsbtorgb(zoneActor->AmbientHue(), zoneActor->AmbientSaturation(), zoneActor->AmbientBrightness()));

		const uint8_t* bits = model->LightBits.data() + lmindex.DataOffset;
		int bitpos = 0;

		bool isSpecialLit = (surface.PolyFlags & PF_SpecialLit) == PF_SpecialLit;

		UActor** lightpos = &model->Lights[lmindex.LightActors];
		while (*lightpos)
		{
			UActor* light = *lightpos;

			if (light->bSpecialLit() != isSpecialLit)
				continue;

			for (int y = 0; y < height; y++)
			{
				float x0 = uv[0].x + leftStep * (y + 0.5f - uv[0].y) + 0.5f;
				float x1 = uv[1].x + rightStep * (y + 0.5f - uv[1].y) + 0.5f;
				float t0 = (y + 0.5f - uv[0].y) / leftDY;
				float t1 = (y + 0.5f - uv[1].y) / rightDY;
				vec3 p0 = mix(p[0], p[2], t0);
				vec3 p1 = mix(p[1], p[2], t1);
				if (x1 < x0) { std::swap(x0, x1); std::swap(p0, p1); }
				DrawLightmapSpan(&lightcolors[(size_t)y * width], 0, width, x0, x1, p0, p1, light, N, bits, bitpos);
			}

			lightpos++;
		}

		float weights[9] = { 0.125f, 0.25f, 0.125f, 0.25f, 0.50f, 0.25f, 0.125f, 0.25f, 0.125f };

		uint32_t* texels = (uint32_t*)lmmip.Data.data();
		bool isTranslucent = (surface.PolyFlags & PF_Translucent) == PF_Translucent;
		for (int y = 0; y < height; y++)
		{
			vec3* src = &lightcolors[(size_t)y * width];
			for (int x = 0; x < width; x++)
			{
				vec3 color = { 0.0f };
				for (int yy = -1; yy <= 1; yy++)
				{
					int yyy = clamp(y + yy, 0, height - 1) - y;
					for (int xx = -1; xx <= 1; xx++)
					{
						int xxx = clamp(x + xx, 0, width - 1);
						color += src[yyy * width + xxx] * weights[4 + xx + yy * 3];
					}
				}
				color *= 0.5f;

				uint32_t red = (uint32_t)clamp(color.r * 255.0f + 0.5f, 0.0f, 255.0f);
				uint32_t green = (uint32_t)clamp(color.g * 255.0f + 0.5f, 0.0f, 255.0f);
				uint32_t blue = (uint32_t)clamp(color.b * 255.0f + 0.5f, 0.0f, 255.0f);
				uint32_t alpha = 127;

				if (isTranslucent)
				{
					red = 64 + (red >> 2);
					green = 64 + (green >> 2);
					blue = 64 + (blue >> 2);
				}

				texels[x] = (alpha << 24) | (red << 16) | (green << 8) | blue;
			}
			texels += width;
		}
	}
	else
	{
		uint32_t* texels = (uint32_t*)lmmip.Data.data();
		int c = width * height;
		vec3 color = hsbtorgb(zoneActor->AmbientHue(), zoneActor->AmbientSaturation(), zoneActor->AmbientBrightness());
		uint32_t red = (uint32_t)clamp(color.r * 255.0f + 0.5f, 0.0f, 255.0f);
		uint32_t green = (uint32_t)clamp(color.g * 255.0f + 0.5f, 0.0f, 255.0f);
		uint32_t blue = (uint32_t)clamp(color.b * 255.0f + 0.5f, 0.0f, 255.0f);
		uint32_t alpha = 127;
		uint32_t ambientcolor = (alpha << 24) | (red << 16) | (green << 8) | blue;
		for (int i = 0; i < c; i++)
		{
			texels[i] = ambientcolor;
		}
	}

	auto lmtexture = UObject::Cast<UTexture>(engine->packages->NewObject("Lightmap", "Engine", "Texture"));
	lmtexture->ActualFormat = TextureFormat::RGBA7;
	lmtexture->Mipmaps.push_back(std::move(lmmip));
	lmtexture->Format() = (uint8_t)lmtexture->ActualFormat;
	return lmtexture;
}

void LightRender::DrawLightmapSpan(vec3* line, int start, int end, float x0, float x1, vec3 p0, vec3 p1, UActor* light, const vec3& N, const uint8_t* bits, int& bitpos)
{
	vec3 lightcolor = hsbtorgb(light->LightHue(), light->LightSaturation(), light->LightBrightness());

	for (int i = start; i < end; i++)
	{
		bool shadowtest = (bits[bitpos >> 3] & (1 << (bitpos & 7))) != 0;
		if (shadowtest)
		{
			float t = (i + 0.5f - x0) / (x1 - x0);
			vec3 point = mix(p0, p1, t);

			vec3 L = light->Location() - point;
			float distanceAttenuation = std::max(1.0f - length(L) / (light->LightRadius() * 32.0f), 0.0f);
			float angleAttenuation = std::max(dot(normalize(L), N), 0.0f);
			float attenuation = distanceAttenuation * angleAttenuation;
			line[i] += lightcolor * attenuation;
		}

		bitpos++;
	}

	bitpos = (bitpos + 7) / 8 * 8;
}

vec3 LightRender::FindLightAt(const vec3& location, int zoneIndex)
{
	UZoneInfo* zoneActor = dynamic_cast<UZoneInfo*>(engine->Level->Model->Zones[zoneIndex].ZoneActor);
	if (!zoneActor)
		zoneActor = engine->LevelInfo;

	vec3 color = hsbtorgb(zoneActor->AmbientHue(), zoneActor->AmbientSaturation(), zoneActor->AmbientBrightness());

	for (UActor* light : engine->renderer->Lights)
	{
		if (light && !light->bCorona() && !light->bSpecialLit())
		{
			vec3 L = light->Location() - location;
			float dist2 = dot(L, L);
			float lightRadius = light->LightRadius() * 32.0f;
			float lightRadius2 = lightRadius * lightRadius;
			if (dist2 < lightRadius2 && !engine->collision->TraceAnyHit(light->Location(), location))
			{
				vec3 lightcolor = hsbtorgb(light->LightHue(), light->LightSaturation(), light->LightBrightness());

				float distanceAttenuation = std::max(1.0f - std::sqrt(dist2) / lightRadius, 0.0f);
				float angleAttenuation = 0.75f; // std::max(dot(normalize(L), N), 0.0f);
				float attenuation = distanceAttenuation * angleAttenuation;
				color += lightcolor * attenuation;
			}
		}
	}

	return color;
}

FTextureInfo LightRender::GetSurfaceFogmap(BspSurface& surface, const FSurfaceFacet& facet, UZoneInfo* zoneActor, UModel* model)
{
	if (!zoneActor->bFogZone() || surface.LightMap < 0)
		return {};

	auto level = engine->Level;
	const LightMapIndex& lmindex = level->Model->LightMap[surface.LightMap];
	auto& fogtex = level->Model->fogtextures[surface.LightMap];
	UTexture*& fogtexture = fogtex.second;
	if (!fogtexture)
	{
		UnrealMipmap fogmip;
		fogmip.Width = lmindex.UClamp;
		fogmip.Height = lmindex.VClamp;
		fogmip.Data.resize((size_t)fogmip.Width * fogmip.Height * 4);

		fogtexture = UObject::Cast<UTexture>(engine->packages->NewObject("Fogmap", "Engine", "Texture"));
		fogtexture->ActualFormat = TextureFormat::RGBA7;
		fogtexture->Mipmaps.push_back(std::move(fogmip));
		fogtexture->Format() = (uint8_t)fogtexture->ActualFormat;
	}

	bool firstDrawThisScene = false;
	if (fogtex.first != engine->renderer->sceneDrawNumber)
	{
		firstDrawThisScene = true;
		fogtex.first = engine->renderer->sceneDrawNumber;
	}

	if (firstDrawThisScene)
		UpdateFogmapTexture(lmindex, (uint32_t*)fogtexture->Mipmaps.front().Data.data(), surface, zoneActor, model);

	FTextureInfo fogmap;
	fogmap.CacheID = ((uint64_t)(ptrdiff_t)&surface) | 1;
	fogmap.Texture = fogtexture;
	fogmap.Pan = { lmindex.PanX, lmindex.PanY };
	fogmap.UScale = lmindex.UScale;
	fogmap.VScale = lmindex.VScale;
	fogmap.bRealtimeChanged = firstDrawThisScene;
	return fogmap;
}

void LightRender::UpdateFogmapTexture(const LightMapIndex& lmindex, uint32_t* texels, const BspSurface& surface, UZoneInfo* zoneActor, UModel* model)
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

	for (UActor* light : engine->renderer->Lights)
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

void LightRender::DrawFogmapSpan(vec4* line, int start, int end, float x0, float x1, vec3 p0, vec3 p1, UActor* light)
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

	vec3 view = engine->Camera.Location;
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

vec4 LightRender::FindFogAt(const vec3& location)
{
	vec4 fogcontribution(0.0f);

	for (UActor* light : engine->renderer->Lights)
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

			vec3 view = engine->Camera.Location;
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
