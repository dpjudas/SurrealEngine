
#include "Precomp.h"
#include "RenderSubsystem.h"
#include "RenderDevice/RenderDevice.h"
#include "Engine.h"
#include "Math/hsb.h"

FTextureInfo RenderSubsystem::GetBrushLightmap(const Poly& poly, UZoneInfo* zoneActor, UModel* model)
{
	int lightmapIndex = poly.BrushPolyIndex;
	if (lightmapIndex < 0)
		return {};

	uint32_t ambientID = (((uint32_t)zoneActor->AmbientHue()) << 16) | (((uint32_t)zoneActor->AmbientSaturation()) << 8) | (uint32_t)zoneActor->AmbientBrightness();

	uint64_t cacheID = (((uint64_t)model->LightMap[lightmapIndex].LMCacheID) << 32) | (((uint64_t)ambientID) << 8) | 1;

	auto level = engine->Level;
	auto& lmtexture = Light.lmtextures[cacheID];
	if (!lmtexture)
	{
		Coords mapCoords;
		mapCoords.Origin = poly.Base;
		mapCoords.XAxis = poly.TextureU;
		mapCoords.YAxis = poly.TextureV;
		mapCoords.ZAxis = poly.Normal;

		Light.Builder.Setup(model, mapCoords, lightmapIndex, zoneActor);
		Light.Builder.AddStaticLights(model, lightmapIndex);

		lmtexture = CreateLightmapTexture();
	}

	const LightMapIndex& lmindex = model->LightMap[lightmapIndex];

	FTextureInfo texinfo;
	texinfo.CacheID = cacheID;
	texinfo.Format = lmtexture->Format;
	texinfo.Mips = &lmtexture->Mip;
	texinfo.NumMips = 1;
	texinfo.USize = texinfo.Mips[0].Width;
	texinfo.VSize = texinfo.Mips[0].Height;
	texinfo.Pan = { lmindex.PanX, lmindex.PanY };
	texinfo.UScale = lmindex.UScale;
	texinfo.VScale = lmindex.VScale;
	return texinfo;
}

FTextureInfo RenderSubsystem::GetSurfaceLightmap(BspSurface& surface, const FSurfaceFacet& facet, UZoneInfo* zoneActor, UModel* model)
{
	if (surface.LightMap < 0)
		return {};

	uint32_t ambientID = (((uint32_t)zoneActor->AmbientHue()) << 16) | (((uint32_t)zoneActor->AmbientSaturation()) << 8) | (uint32_t)zoneActor->AmbientBrightness();

	uint64_t cacheID = (((uint64_t)model->LightMap[surface.LightMap].LMCacheID) << 32) | (((uint64_t)ambientID) << 8) | 1;

	auto level = engine->Level;
	auto& lmtexture = Light.lmtextures[cacheID];
	if (!lmtexture)
	{
		Coords mapCoords;
		mapCoords.Origin = model->Points[surface.pBase];
		mapCoords.XAxis = model->Vectors[surface.vTextureU];
		mapCoords.YAxis = model->Vectors[surface.vTextureV];
		mapCoords.ZAxis = model->Vectors[surface.vNormal];

		Light.Builder.Setup(model, mapCoords, surface.LightMap, zoneActor);
		Light.Builder.AddStaticLights(model, surface.LightMap);

		lmtexture = CreateLightmapTexture();
	}

	const LightMapIndex& lmindex = model->LightMap[surface.LightMap];

	FTextureInfo texinfo;
	texinfo.CacheID = cacheID;
	texinfo.Format = lmtexture->Format;
	texinfo.Mips = &lmtexture->Mip;
	texinfo.NumMips = 1;
	texinfo.USize = texinfo.Mips[0].Width;
	texinfo.VSize = texinfo.Mips[0].Height;
	texinfo.Pan = { lmindex.PanX, lmindex.PanY };
	texinfo.UScale = lmindex.UScale;
	texinfo.VScale = lmindex.VScale;
	return texinfo;
}

std::unique_ptr<LightmapTexture> RenderSubsystem::CreateLightmapTexture()
{
#if 1 // Float high quality lightmaps

	UnrealMipmap lmmip;
	lmmip.Width = Light.Builder.Width();
	lmmip.Height = Light.Builder.Height();
	lmmip.Data.resize((size_t)lmmip.Width * lmmip.Height * sizeof(vec4));

	vec4* dest = (vec4*)lmmip.Data.data();
	const vec3* src = Light.Builder.Pixels();
	int count = lmmip.Width * lmmip.Height;
	for (int i = 0; i < count; i++)
	{
		dest[i] = vec4(src[i], 1.0f);
	}

	auto lmtexture = std::make_unique<LightmapTexture>();
	lmtexture->Format = TextureFormat::RGBA32_F;
	lmtexture->Mip = std::move(lmmip);
	return lmtexture;

#else // Low quality lightmaps like UE1 got them

	UnrealMipmap lmmip;
	lmmip.Width = Light.Builder.Width();
	lmmip.Height = Light.Builder.Height();
	lmmip.Data.resize((size_t)lmmip.Width * lmmip.Height * 4);

	uint32_t* dest = (uint32_t*)lmmip.Data.data();
	const vec3* src = Light.Builder.Pixels();
	int count = lmmip.Width * lmmip.Height;
	for (int i = 0; i < count; i++)
	{
		uint32_t red = (uint32_t)clamp(src[i].r * 127.0f + 0.5f, 0.0f, 127.0f);
		uint32_t green = (uint32_t)clamp(src[i].g * 127.0f + 0.5f, 0.0f, 127.0f);
		uint32_t blue = (uint32_t)clamp(src[i].b * 127.0f + 0.5f, 0.0f, 127.0f);
		uint32_t alpha = 127;

		dest[i] = (alpha << 24) | (red << 16) | (green << 8) | blue;
	}

	auto lmtexture = std::make_unique<LightmapTexture>();
	lmtexture->Format = TextureFormat::BGRA8_LM;
	lmtexture->Mip = std::move(lmmip);
	return lmtexture;

#endif
}

void RenderSubsystem::UpdateActorLightList(UActor* actor)
{
	vec3 location = actor->Location();

	if (!actor->LightInfo.NeedsUpdate && actor->LightInfo.Location == location)
		return;

	actor->LightInfo.NeedsUpdate = false;
	actor->LightInfo.Location = location;
	actor->LightInfo.LightList.clear();

	if (actor->bUnlit())
		return;

	for (UActor* light : Light.Lights)
	{
		if (light && !light->bCorona() && !light->bSpecialLit())
		{
			float radius = light->WorldLightRadius();
			vec3 L = light->Location() - location;
			if (light->LightEffect() == LE_Cylinder) // Cylinder lights have infinite Z axis range
			{
				L.z = 0.0f;
			}
			if (dot(L, L) < radius * radius && !engine->Level->TraceRayAnyHit(light->Location(), location, nullptr, false, true, true))
			{
				actor->LightInfo.LightList.push_back(light);
			}
		}
	}
}

vec3 RenderSubsystem::GetVertexLight(UActor* actor, const vec3& location, const vec3& normal, bool unlit)
{
	if (unlit)
	{
		return vec3(clamp(actor->ScaleGlow() * 0.5f + actor->AmbientGlow() * (1.0f / 256.0f), 0.0f, 1.0f));
	}
	else
	{
		UZoneInfo* zoneActor = static_cast<UZoneInfo*>(engine->Level->Model->Zones[actor->Region().ZoneNumber].ZoneActor);
		if (!zoneActor)
			zoneActor = engine->LevelInfo;

		vec3 color = hsbtorgb(zoneActor->AmbientHue(), zoneActor->AmbientSaturation(), zoneActor->AmbientBrightness());

		for (UActor* light : actor->LightInfo.LightList)
		{
			float attenuation = LightEffect::VertexLight(light, location, normal);
			vec3 lightcolor = hsbtorgb(light->LightHue(), light->LightSaturation(), light->LightBrightness());
			color += lightcolor * attenuation;
		}

		return color * 2.0f;
	}
}
