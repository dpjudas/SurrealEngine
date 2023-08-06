
#include "Precomp.h"
#include "RenderSubsystem.h"
#include "RenderDevice/RenderDevice.h"
#include "Engine.h"
#include "Math/hsb.h"

FTextureInfo RenderSubsystem::GetSurfaceLightmap(BspSurface& surface, const FSurfaceFacet& facet, UZoneInfo* zoneActor, UModel* model)
{
	if (surface.LightMap < 0)
		return {};

	uint32_t ambientID = (((uint32_t)zoneActor->AmbientHue()) << 16) | (((uint32_t)zoneActor->AmbientSaturation()) << 8) | (uint32_t)zoneActor->AmbientBrightness();

	uint64_t cacheID = (((uint64_t)surface.LightMap) << 32) | (((uint64_t)ambientID) << 8) | 1;

	auto level = engine->Level;
	auto& lmtexture = Light.lmtextures[cacheID];
	if (!lmtexture)
	{
		lmtexture = CreateLightmapTexture(surface, zoneActor, model);
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

std::unique_ptr<LightmapTexture> RenderSubsystem::CreateLightmapTexture(const BspSurface& surface, UZoneInfo* zoneActor, UModel* model)
{
	Light.Builder.Setup(model, surface, zoneActor);
	Light.Builder.AddStaticLights(model, surface);

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
}

vec3 RenderSubsystem::FindLightAt(const vec3& location, int zoneIndex)
{
	UZoneInfo* zoneActor = dynamic_cast<UZoneInfo*>(engine->Level->Model->Zones[zoneIndex].ZoneActor);
	if (!zoneActor)
		zoneActor = engine->LevelInfo;

	vec3 color = hsbtorgb(zoneActor->AmbientHue(), zoneActor->AmbientSaturation(), zoneActor->AmbientBrightness()) * 0.5f;

	for (UActor* light : Light.Lights)
	{
		if (light && !light->bCorona() && !light->bSpecialLit())
		{
			vec3 L = light->Location() - location;
			float radius = light->WorldLightRadius();
			float dist = dot(L, L) / (radius * radius);
			if (dist < 1.0f && !engine->Level->TraceRayAnyHit(light->Location(), location, nullptr, false, true, true))
			{
				vec3 lightcolor = hsbtorgb(light->LightHue(), light->LightSaturation(), 255) * clamp(light->LightBrightness() * (1.0f / 255.0f), 0.0f, 1.0f) * light->Level()->Brightness();

				float distanceAttenuation = LightEffect::LightDistanceFalloff(dist);
				float angleAttenuation = 0.75f; // std::max(dot(normalize(L), N), 0.0f);
				float attenuation = distanceAttenuation * angleAttenuation;
				color += lightcolor * attenuation;
			}
		}
	}

	return color;
}
