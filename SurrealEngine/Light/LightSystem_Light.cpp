
#include "Precomp.h"
#include "LightSystem.h"
#include "RenderDevice/RenderDevice.h"
#include "Engine.h"
#include "Render/RenderSubsystem.h"
#include "Math/hsb.h"
#include "Packages/Engine/Actors/Brush/UMover.h"
#include "Packages/Engine/Actors/Info/UZoneInfo.h"
#include "Packages/Engine/Resources/Level/UPolys.h"
#include "Packages/Engine/Resources/Level/UModel.h"

TextureInfo LightSystem::GetMoverLightmap(UMover* mover, const Poly& poly, UZoneInfo* zoneActor, UModel* model)
{
	Coords localCoords;
	localCoords.Origin = -poly.Base;
	localCoords.XAxis = poly.TextureU;
	localCoords.YAxis = poly.TextureV;
	localCoords.ZAxis = poly.Normal;

	vec3 scale = mover->MainScale().Scale;

	mat4 objectToWorld;
	if (mover->bDynamicLightMover())
	{
		vec3 moverLocation = mover->Location();
		Rotator moverRotation = mover->Rotation();
		objectToWorld = mat4::translate(moverLocation) * Coords::Rotation(moverRotation).ToMatrix() * mat4::scale(scale) * mat4::translate(-mover->PrePivot()) * localCoords.ToMatrix();
	}
	else
	{
		vec3 moverLocation = mover->BasePos() + mover->KeyPos()[mover->BrushRaytraceKey()];
		Rotator moverRotation = mover->BaseRot() + mover->KeyRot()[mover->BrushRaytraceKey()];
		objectToWorld = mat4::translate(moverLocation) * Coords::Rotation(moverRotation).ToMatrix() * mat4::scale(scale) * mat4::translate(-mover->PrePivot()) * localCoords.ToMatrix();
	}
	Coords worldCoords = Coords::FromMatrix(objectToWorld);

	if (!mover->Light.Calculated)
	{
		vec3 location = { 0.0f };
		float radius = 0.0f;
		if (!poly.Vertices.empty())
		{
			vec3 aabbMin = poly.Vertices.front();
			vec3 aabbMax = aabbMin;
			for (const vec3& v : poly.Vertices)
			{
				aabbMin.x = std::min(aabbMin.x, v.x);
				aabbMin.y = std::min(aabbMin.y, v.y);
				aabbMin.z = std::min(aabbMin.z, v.z);
				aabbMax.x = std::max(aabbMax.x, v.x);
				aabbMax.y = std::max(aabbMax.y, v.y);
				aabbMax.z = std::max(aabbMax.z, v.z);
			}
			auto halfmin = aabbMin * 0.5f;
			auto halfmax = aabbMax * 0.5f;
			location = halfmax + halfmin;
			radius = length(halfmax - halfmin);
		}
		mover->Light.Calculated = true;
		mover->Light.Center = location;
		mover->Light.Radius = radius;
	}

	return GetLightmap(
		model, poly.BrushPolyIndex, worldCoords, zoneActor,
		(objectToWorld * vec4(mover->Light.Center, 1.0f)).xyz(),
		mover->Light.Radius * std::max(scale.x, std::max(scale.y, scale.z)),
		mover->bDynamicLightMover() ? mover : nullptr, mover->bSpecialLit());
}

TextureInfo LightSystem::GetLevelLightmap(BspSurface& surface, UZoneInfo* zoneActor, UModel* model)
{
	Coords mapCoords;
	mapCoords.Origin = model->Points[surface.pBase];
	mapCoords.XAxis = model->Vectors[surface.vTextureU];
	mapCoords.YAxis = model->Vectors[surface.vTextureV];
	mapCoords.ZAxis = model->Vectors[surface.vNormal];
	return GetLightmap(model, surface.LightMap, mapCoords, zoneActor, surface.Center, surface.Radius, nullptr, surface.PolyFlags & PF_SpecialLit);
}

TextureInfo LightSystem::GetLightmap(UModel* model, int lightmapIndex, const Coords& coords, UZoneInfo* zoneActor, const vec3& worldLocation, float radius, UMover* dynamicMover, bool specialLit)
{
	if (lightmapIndex < 0)
		return {};

	LightMapIndex& lmindex = model->LightMap[lightmapIndex];

	uint32_t ambientID = (((uint32_t)zoneActor->AmbientHue()) << 16) | (((uint32_t)zoneActor->AmbientSaturation()) << 8) | (uint32_t)zoneActor->AmbientBrightness();
	uint64_t cacheID = (((uint64_t)lmindex.LMCacheID) << 32) | (((uint64_t)ambientID) << 8) | 1;

	if (dynamicMover)
	{
		// Place mover ID in upper 12 bits (4096 movers). This leaves 20 bits (1 million) for the lightmaps.
		if (dynamicMover->Light.MoverID == 0)
			dynamicMover->Light.MoverID = NextMoverID++;
		cacheID |= ((uint64_t)dynamicMover->Light.MoverID) << 52;
	}

	int checkCounter = LightmapCheckCounter++;

	// Collect lights for the lightmap and check if they changed

	int lastStaticUpdate = -100;
	int lastDynamicUpdate = -100;
	TempDynLightList.clear();

	if (!dynamicMover)
	{
		// Examine all the lights that are statically baked into the map:

		if (lmindex.LightActors >= 0)
		{
			UActor** lightlist = &model->Lights[lmindex.LightActors];
			for (int lightindex = 0; lightlist[lightindex] != nullptr; lightindex++)
			{
				UActor* light = lightlist[lightindex];
				CheckLight(light);
				lastStaticUpdate = std::max(lastStaticUpdate, light->Light.LastUpdate);

				// Mark the light as visited so the second pass doesn't pick it up
				light->Light.LightmapCheckCounter = checkCounter;
			}
		}

		// Look at all lights potentially touching the surface. They go into our dynamic light list:

		LightTree.CollectLights(worldLocation, radius);
		for (UActor* light : LightTree.CollectedLights)
		{
			if (light->Light.LightmapCheckCounter != checkCounter)
			{
				light->Light.LightmapCheckCounter = checkCounter;
				if (!light->bStatic() && !light->bNoDelete() && light->bSpecialLit() == specialLit)
				{
					CheckLight(light);
					lastDynamicUpdate = std::max(lastDynamicUpdate, light->Light.LastUpdate);
					TempDynLightList.push_back(light);
				}
			}
		}
	}
	else
	{
		// To do: ideally we only want to do this if the mover moved
		lastDynamicUpdate = FrameCounter;

		for (UActor* light : dynamicMover->TouchingLights.List)
		{
			CheckLight(light);
			lastDynamicUpdate = std::max(lastDynamicUpdate, light->Light.LastUpdate);
			TempDynLightList.push_back(light);
		}
	}

	// If anything changed update the lightmap:

	bool bRealtimeChanged = false;
	auto& lmtexture = lmtextures[cacheID];
	if (!lmtexture || lmtexture->LastStaticUpdate != lastStaticUpdate || lmtexture->LastDynamicUpdate != lastDynamicUpdate)
	{
		engine->render->Stats.LightmapsUpdated++;

		Builder.Setup(model, coords, lightmapIndex);

		if (!lmtexture || lmtexture->Mip.Width != Builder.Width() || lmtexture->Mip.Height != Builder.Height())
		{
			lmtexture = std::make_unique<LightmapTexture>();
			lmtexture->Format = TextureFormat::RGBA32_F;
			lmtexture->Mip.Width = Builder.Width();
			lmtexture->Mip.Height = Builder.Height();
			lmtexture->Mip.Data.resize((size_t)lmtexture->Mip.Width * lmtexture->Mip.Height * sizeof(vec4));
		}

		if (dynamicMover)
		{
			Builder.SetAmbientLight(zoneActor);
		}
		else if (lmtexture->LastStaticUpdate != lastStaticUpdate)
		{
			Builder.SetAmbientLight(zoneActor);
			Builder.AddStaticLights(model, lightmapIndex);
			Builder.SaveStaticLight(lmtexture->StaticLightColors);
		}
		else if (lmtexture->StaticLightColors.size() == Builder.Width() * Builder.Height())
		{
			Builder.LoadStaticLight(lmtexture->StaticLightColors);
		}

		Builder.AddDynamicLights(model, lightmapIndex, TempDynLightList);

		UnrealMipmap& lmmip = lmtexture->Mip;
		vec4* dest = (vec4*)lmmip.Data.data();
		const vec3* src = Builder.Pixels();
		int count = lmmip.Width * lmmip.Height;
		for (int i = 0; i < count; i++)
		{
			dest[i].r = std::min(src[i].r, 1.0f);
			dest[i].g = std::min(src[i].g, 1.0f);
			dest[i].b = std::min(src[i].b, 1.0f);
			dest[i].a = 1.0f;
		}

		lmtexture->LastStaticUpdate = lastStaticUpdate;
		lmtexture->LastDynamicUpdate = lastDynamicUpdate;
		bRealtimeChanged = true;
	}

	TextureInfo texinfo;
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

	vec3 location = light->Location();
	float radius = light->WorldLightRadius();
	uint8_t type = light->LightType();
	uint8_t effect = light->LightEffect();
	uint8_t hue = light->LightHue();
	uint8_t saturation = light->LightSaturation();
	uint8_t brightness = light->LightBrightness();
	if (
		location != light->Light.Location ||
		radius != light->Light.Radius ||
		type != light->Light.Type ||
		effect != light->Light.Effect ||
		hue != light->Light.Hue ||
		saturation != light->Light.Saturation ||
		brightness != light->Light.Brightness)
	{
		light->Light.Location = location;
		light->Light.Radius = radius;
		light->Light.Type = type;
		light->Light.Effect = effect;
		light->Light.Hue = hue;
		light->Light.Saturation = saturation;
		light->Light.Brightness = brightness;
		light->Light.LastUpdate = FrameCounter;
	}
}

static float LightDistanceFalloff(float distsqr)
{
	float v = std::sqrt(distsqr + 0.0001f);
	float v2 = v * v;
	float v3 = v2 * v;
	return std::min((1.0f + 2.0f * v3 - 3.0f * v2) / v, 1.0f);
}

void LightSystem::InitVertexLight(VertexLight& out, UActor* actor, UZoneInfo* zoneActor)
{
	// AmbientGlow value 255 is a special pulsating effect used for powerups
	float ambientGlow = actor->AmbientGlow() == 255 ? AmbientGlowAmount : actor->AmbientGlow() * (1.0f / 255.0f);
	out.AmbientColor = ambientGlow + hsbtorgb(zoneActor->AmbientHue(), zoneActor->AmbientSaturation(), zoneActor->AmbientBrightness());
	out.ScaleGlow = actor->ScaleGlow() * 1.5f;

	int lightIndex = 0;
	for (UActor* light : actor->TouchingLights.List)
	{
		out.Lights[lightIndex].Location = light->Location();
		out.Lights[lightIndex].Color = LightmapBuilder::GetLightColor(light);
		float invRadius = 1.0f / light->WorldLightRadius();
		out.Lights[lightIndex].InvRadiusSquared = invRadius * invRadius;
		lightIndex++;
		if (lightIndex == VertexLight::MaxLights)
			break;
	}
	out.NumLights = lightIndex;

	out.CameraLocation = engine->CameraLocation;

	int fogIndex = 0;
	for (UActor* light : FogBalls)
	{
		if (light->FogInfo.brightness < 0.0f)
		{
			light->FogInfo.fogcolor = hsbtorgb(light->LightHue(), light->LightSaturation(), light->LightBrightness());
			light->FogInfo.brightness = light->LightBrightness() * (1.0f / 255.0f) * light->VolumeBrightness() * (1.0f / 64.0f);
			light->FogInfo.fog = light->VolumeFog() * (1.0f / 255.0f);
			light->FogInfo.radius = light->WorldVolumetricRadius();
			light->FogInfo.location = light->Location();
		}

		out.FogBalls[fogIndex].fogcolor = light->FogInfo.fogcolor;
		out.FogBalls[fogIndex].brightness = light->FogInfo.brightness * 5.0f;
		out.FogBalls[fogIndex].fog = light->FogInfo.fog;
		out.FogBalls[fogIndex].radius = light->FogInfo.radius;
		out.FogBalls[fogIndex].lightpos = light->FogInfo.location;

		fogIndex++;
		if (fogIndex == VertexLight::MaxFogBalls)
			break;
	}
	out.NumFogBalls = fogIndex;
}
