
#include "Precomp.h"
#include "RenderSubsystem.h"
#include "RenderDevice/RenderDevice.h"
#include "Lightmap/FogmapBuilder.h"
#include "Engine.h"
#include "Math/hsb.h"

// Don't render the fog in debug builds as we'd rather have a higher frame rate
#if defined(DEBUG) || defined(_DEBUG)
#define NOFOG
#endif

FTextureInfo RenderSubsystem::GetSurfaceFogmap(BspSurface& surface, const FSurfaceFacet& facet, UZoneInfo* zoneActor, UModel* model)
{
#ifdef NOFOG
	return {};
#else
	if (!zoneActor || !zoneActor->bFogZone() || surface.LightMap < 0)
		return {};

	uint32_t ambientID = (((uint32_t)zoneActor->AmbientHue()) << 16) | (((uint32_t)zoneActor->AmbientSaturation()) << 8) | (uint32_t)zoneActor->AmbientBrightness();

	uint64_t cacheID = (((uint64_t)surface.LightMap) << 32) | (((uint64_t)ambientID) << 8) | 2;

	auto level = engine->Level;
	const LightMapIndex& lmindex = level->Model->LightMap[surface.LightMap];
	auto& fogtex = Light.fogtextures[cacheID];
	std::unique_ptr<LightmapTexture>& fogtexture = fogtex.second;
	if (!fogtexture)
	{
#if 1 // Float high quality lightmaps
		UnrealMipmap fogmip;
		fogmip.Width = lmindex.UClamp;
		fogmip.Height = lmindex.VClamp;
		fogmip.Data.resize((size_t)fogmip.Width * fogmip.Height * sizeof(vec4));

		fogtexture = std::make_unique<LightmapTexture>();
		fogtexture->Format = TextureFormat::RGBA32_F;
		fogtexture->Mip = std::move(fogmip);
#else // Low quality lightmaps like UE1 got them
		UnrealMipmap fogmip;
		fogmip.Width = lmindex.UClamp;
		fogmip.Height = lmindex.VClamp;
		fogmip.Data.resize((size_t)fogmip.Width * fogmip.Height * 4);

		fogtexture = std::make_unique<LightmapTexture>();
		fogtexture->Format = TextureFormat::BGRA8_LM;
		fogtexture->Mip = std::move(fogmip);
#endif
	}

	bool firstDrawThisScene = false;
	if (fogtex.first != Light.FogFrameCounter)
	{
		firstDrawThisScene = true;
		fogtex.first = Light.FogFrameCounter;
	}

	if (firstDrawThisScene)
		UpdateFogmapTexture(lmindex, (uint32_t*)fogtexture->Mip.Data.data(), surface, zoneActor, model);

	FTextureInfo texinfo;
	texinfo.CacheID = cacheID;
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

void RenderSubsystem::UpdateFogmapTexture(const LightMapIndex& lmindex, uint32_t* dest, const BspSurface& surface, UZoneInfo* zoneActor, UModel* model)
{
	FogmapBuilder builder;
	builder.Setup(model, surface, zoneActor);

	for (UActor* light : Light.Lights)
	{
		if (light && light->VolumeRadius() != 0)
		{
			builder.AddLight(light, engine->CameraLocation);
		}
	}

#if 1 // Float high quality lightmaps
	size_t size = (size_t)builder.Width() * builder.Height();
	const vec4* src = builder.Pixels();
	memcpy(dest, src, size * sizeof(vec4));
#else // Low quality lightmaps like UE1 got them
	size_t size = (size_t)builder.Width() * builder.Height();
	const vec4* src = builder.Pixels();
	for (size_t i = 0; i < size; i++)
	{
		const vec4& color = src[i];

		uint32_t red = (uint32_t)clamp(src[i].r * 127.0f + 0.5f, 0.0f, 127.0f);
		uint32_t green = (uint32_t)clamp(src[i].g * 127.0f + 0.5f, 0.0f, 127.0f);
		uint32_t blue = (uint32_t)clamp(src[i].b * 127.0f + 0.5f, 0.0f, 127.0f);
		uint32_t alpha = (uint32_t)clamp(src[i].a * 127.0f + 0.5f, 0.0f, 127.0f);

		dest[i] = (alpha << 24) | (red << 16) | (green << 8) | blue;
	}
#endif
}

vec4 RenderSubsystem::GetVertexFog(UActor* actor, const vec3& location)
{
	vec4 color(0.0f);
#ifndef NOFOG
	vec3 view = engine->CameraLocation;
	vec3 rayDirection = location - view;
	float depth = std::sqrt(dot(rayDirection, rayDirection));
	rayDirection *= (1.0f / depth);

	for (UActor* light : Light.Lights)
	{
		if (light && light->VolumeRadius() != 0)
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

			float fogamount = FogmapBuilder::SphereDensity(view, rayDirection, lightpos, radius, depth) * brightness;

			float alpha = std::min(fogamount * fog, 1.0f);
			float invalpha = 1.0f - alpha;
			color.r = fogcolor.r * fogamount + color.r * invalpha;
			color.g = fogcolor.g * fogamount + color.g * invalpha;
			color.b = fogcolor.b * fogamount + color.b * invalpha;
			color.a = std::min(color.a + alpha, 1.0f);
		}
	}
#endif
	return color;
}
