
#include "Precomp.h"
#include "RenderSubsystem.h"
#include "RenderDevice/RenderDevice.h"
#include "Lightmap/FogmapBuilder.h"
#include "Engine.h"
#include "Math/hsb.h"

#define NOFOG // The current fog implementation is just too slow!

FTextureInfo RenderSubsystem::GetSurfaceFogmap(BspSurface& surface, const FSurfaceFacet& facet, UZoneInfo* zoneActor, UModel* model)
{
#ifdef NOFOG
	return {};
#else
	if (!zoneActor || !zoneActor->bFogZone() || surface.LightMap < 0)
		return {};

	uint64_t cacheID = (((uint64_t)surface.LightMap) << 16) | (((uint64_t)(zoneActor ? zoneActor->Region().ZoneNumber : 255)) << 8) | 2;

	auto level = engine->Level;
	const LightMapIndex& lmindex = level->Model->LightMap[surface.LightMap];
	auto& fogtex = Light.fogtextures[cacheID];
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
}
