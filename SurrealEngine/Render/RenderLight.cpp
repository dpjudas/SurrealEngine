
#include "Precomp.h"
#include "RenderSubsystem.h"
#include "RenderDevice/RenderDevice.h"
#include "Engine.h"
#include "Math/hsb.h"

FTextureInfo RenderSubsystem::GetBrushLightmap(UMover* mover, const Poly& poly, UZoneInfo* zoneActor, UModel* model)
{
	// To do: implement mover->bDynamicLightMover()

	Coords localCoords;
	localCoords.Origin = -poly.Base;
	localCoords.XAxis = poly.TextureU;
	localCoords.YAxis = poly.TextureV;
	localCoords.ZAxis = poly.Normal;

	vec3 moverLocation = mover->BasePos() + mover->KeyPos()[mover->BrushRaytraceKey()];
	Rotator moverRotation = mover->BaseRot() + mover->KeyRot()[mover->BrushRaytraceKey()];
	mat4 objectToWorld = mat4::translate(moverLocation) * Coords::Rotation(moverRotation).ToMatrix() * mat4::scale(mover->MainScale().Scale) * mat4::translate(-mover->PrePivot()) * localCoords.ToMatrix();
	Coords worldCoords = Coords::FromMatrix(objectToWorld);

	return GetLightmap(model, poly.BrushPolyIndex, worldCoords, zoneActor);
}

FTextureInfo RenderSubsystem::GetSurfaceLightmap(BspSurface& surface, UZoneInfo* zoneActor, UModel* model)
{
	Coords mapCoords;
	mapCoords.Origin = model->Points[surface.pBase];
	mapCoords.XAxis = model->Vectors[surface.vTextureU];
	mapCoords.YAxis = model->Vectors[surface.vTextureV];
	mapCoords.ZAxis = model->Vectors[surface.vNormal];
	return GetLightmap(model, surface.LightMap, mapCoords, zoneActor);
}

FTextureInfo RenderSubsystem::GetLightmap(UModel* model, int lightmapIndex, const Coords& coords, UZoneInfo* zoneActor)
{
	if (lightmapIndex < 0)
		return {};

	uint32_t ambientID = (((uint32_t)zoneActor->AmbientHue()) << 16) | (((uint32_t)zoneActor->AmbientSaturation()) << 8) | (uint32_t)zoneActor->AmbientBrightness();
	uint64_t cacheID = (((uint64_t)model->LightMap[lightmapIndex].LMCacheID) << 32) | (((uint64_t)ambientID) << 8) | 1;

	auto& lmtexture = Light.lmtextures[cacheID];
	if (!lmtexture)
	{
		Light.Builder.Setup(model, coords, lightmapIndex, zoneActor);
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
			if (dot(L, L) < radius * radius && !engine->Level->Collision.TraceAnyHit(light->Location(), location, nullptr, false, true, true))
			{
				actor->LightInfo.LightList.push_back(light);
			}
		}
	}
}

vec3 RenderSubsystem::GetVertexLight(UActor* actor, const vec3& location, const vec3& normal, bool unlit)
{
	UZoneInfo* zoneActor = UObject::TryCast<UZoneInfo>(engine->Level->Model->Zones[actor->Region().ZoneNumber].ZoneActor);
	if (!zoneActor)
		zoneActor = engine->LevelInfo;

	// AmbientGlow value 255 is a special pulsating effect used for powerups
	float ambientGlow = actor->AmbientGlow() == 255 ? AmbientGlowAmount : actor->AmbientGlow() * (1.0f / 255.0f);
	vec3 ambientColor = ambientGlow + hsbtorgb(zoneActor->AmbientHue(), zoneActor->AmbientSaturation(), zoneActor->AmbientBrightness());

	if (unlit)
	{
		return ambientColor + actor->ScaleGlow() * 0.5f;
	}
	else
	{
		vec3 color(0.0f);

		for (UActor* light : actor->LightInfo.LightList)
		{
			float attenuation = LightEffect::VertexLight(light, location, normal);
			vec3 lightcolor = hsbtorgb(light->LightHue(), light->LightSaturation(), light->LightBrightness());
			color += lightcolor * attenuation;
		}

		return ambientColor + color * (actor->ScaleGlow() * 1.5f);
	}
}
