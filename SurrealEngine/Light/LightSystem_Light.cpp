
#include "Precomp.h"
#include "LightSystem.h"
#include "RenderDevice/RenderDevice.h"
#include "Engine.h"
#include "Math/hsb.h"
#include "Packages/Engine/Actors/Brush/UMover.h"
#include "Packages/Engine/Actors/Info/UZoneInfo.h"
#include "Packages/Engine/Resources/Level/UPolys.h"
#include "Packages/Engine/Resources/Level/UModel.h"

FTextureInfo LightSystem::GetMoverLightmap(UMover* mover, const Poly& poly, UZoneInfo* zoneActor, UModel* model)
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

FTextureInfo LightSystem::GetLevelLightmap(BspSurface& surface, UZoneInfo* zoneActor, UModel* model)
{
	Coords mapCoords;
	mapCoords.Origin = model->Points[surface.pBase];
	mapCoords.XAxis = model->Vectors[surface.vTextureU];
	mapCoords.YAxis = model->Vectors[surface.vTextureV];
	mapCoords.ZAxis = model->Vectors[surface.vNormal];
	return GetLightmap(model, surface.LightMap, mapCoords, zoneActor);
}

FTextureInfo LightSystem::GetLightmap(UModel* model, int lightmapIndex, const Coords& coords, UZoneInfo* zoneActor)
{
	if (lightmapIndex < 0)
		return {};

	LightMapIndex& lmindex = model->LightMap[lightmapIndex];

	uint32_t ambientID = (((uint32_t)zoneActor->AmbientHue()) << 16) | (((uint32_t)zoneActor->AmbientSaturation()) << 8) | (uint32_t)zoneActor->AmbientBrightness();
	uint64_t cacheID = (((uint64_t)lmindex.LMCacheID) << 32) | (((uint64_t)ambientID) << 8) | 1;

	int lastUpdate = -1;
	if (lmindex.LightActors >= 0)
	{
		UActor** lightlist = &model->Lights[lmindex.LightActors];
		for (int lightindex = 0; lightlist[lightindex] != nullptr; lightindex++)
		{
			UActor* light = lightlist[lightindex];
			CheckLight(light);
			lastUpdate = std::max(lastUpdate, light->Light.LastUpdate);
		}
	}

	bool bRealtimeChanged = false;
	auto& lmtexture = lmtextures[cacheID];
	if (!lmtexture || lmtexture->LastUpdate != lastUpdate)
	{
		Builder.Setup(model, coords, lightmapIndex, zoneActor);
		Builder.AddStaticLights(model, lightmapIndex);

		if (!lmtexture)
		{
			lmtexture = std::make_unique<LightmapTexture>();
			lmtexture->Format = TextureFormat::RGBA32_F;
			lmtexture->Mip.Width = Builder.Width();
			lmtexture->Mip.Height = Builder.Height();
			lmtexture->Mip.Data.resize((size_t)lmtexture->Mip.Width * lmtexture->Mip.Height * sizeof(vec4));
		}

		UnrealMipmap& lmmip = lmtexture->Mip;
		vec4* dest = (vec4*)lmmip.Data.data();
		const vec3* src = Builder.Pixels();
		int count = lmmip.Width * lmmip.Height;
		for (int i = 0; i < count; i++)
		{
			dest[i] = vec4(src[i], 1.0f);
		}

		lmtexture->LastUpdate = lastUpdate;
		bRealtimeChanged = true;
	}

	FTextureInfo texinfo;
	texinfo.bRealtimeChanged = bRealtimeChanged;
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

void LightSystem::CheckLight(UActor* light)
{
	if (light->Light.LastCheck == FrameCounter)
		return;

	light->Light.LastCheck = FrameCounter;

	uint8_t type = light->LightType();
	uint8_t effect = light->LightEffect();
	uint8_t hue = light->LightHue();
	uint8_t saturation = light->LightSaturation();
	uint8_t brightness = light->LightBrightness();
	if (type != light->Light.Type ||
		effect != light->Light.Effect ||
		hue != light->Light.Hue ||
		saturation != light->Light.Saturation ||
		brightness != light->Light.Brightness)
	{
		light->Light.Type = type;
		light->Light.Effect = effect;
		light->Light.Hue = hue;
		light->Light.Saturation = saturation;
		light->Light.Brightness = brightness;
		light->Light.LastUpdate = FrameCounter;
	}
}

vec3 LightSystem::GetVertexLight(UActor* actor, const vec3& location, const vec3& normal, bool unlit, UZoneInfo* zoneActor)
{
	// AmbientGlow value 255 is a special pulsating effect used for powerups
	float ambientGlow = actor->AmbientGlow() == 255 ? AmbientGlowAmount : actor->AmbientGlow() * (1.0f / 255.0f);
	vec3 ambientColor = ambientGlow + hsbtorgb(zoneActor->AmbientHue(), zoneActor->AmbientSaturation(), zoneActor->AmbientBrightness());

	if (unlit)
	{
		return (ambientColor + actor->ScaleGlow() * 0.5f) * 2.0f;
	}
	else
	{
		vec3 color(0.0f);

		for (UActor* light : actor->TouchingLights.List)
		{
			vec3 L = light->Location() - location;
			float attenuation = std::max(1.0f - length(L) / light->WorldLightRadius(), 0.0f);
			if (attenuation > 0.0f)
			{
				float angleAttenuation = std::abs(dot(normalize(L), normal));
				vec3 lightcolor = hsbtorgb(light->LightHue(), light->LightSaturation(), light->LightBrightness());
				color += lightcolor * (attenuation * angleAttenuation);
			}
		}

		return (ambientColor + color * (actor->ScaleGlow() * 1.5f)) * 2.0f;
	}
}
