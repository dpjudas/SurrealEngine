
#include "Precomp.h"
#include "LightSystem.h"
#include "Packages/Engine/Actors/UActor.h"
#include "Packages/Engine/Actors/Info/ULevelInfo.h"
#include "Packages/Engine/Resources/Level/ULevel.h"
#include "Packages/Engine/Resources/Level/UModel.h"
#include "Math/floating.h"
#include "Math/coords.h"
#include "Utils/Random.h"
#include "Engine.h"

LightSystem::LightSystem()
{
}

LightSystem::~LightSystem()
{
}

void LightSystem::Tick(float levelTimeElapsed)
{
	AmbientGlowTime = std::fmod(AmbientGlowTime + 0.8f * levelTimeElapsed, 1.0f);
	AmbientGlowAmount = 0.20f + 0.20f * std::sin(radians(AmbientGlowTime * 360.0f));
}

void LightSystem::OnMapLoaded()
{
	engine->Level->Light.FogBalls.clear();
	engine->Level->Light.lmtextures.clear();
	engine->Level->Light.fogtextures.clear();

	std::set<UActor*> lightset;
	for (UActor* light : engine->Level->Model->Lights)
	{
		if (light)
			lightset.insert(light);
	}

	for (UActor* light : lightset)
	{
		if (light->VolumeRadius() != 0)
			engine->Level->Light.FogBalls.push_back(light);
	}
}

void LightSystem::BeginFrame()
{
	FrameCounter++;

	LightTree.Lights.clear();
	for (UActor* actor : engine->Level->Actors)
	{
		if (!actor)
			continue;

		uint8_t lightType = actor->LightType();
		if (lightType == LT_None || actor->LightBrightness() == 0)
			continue;

		bool needsUpdate = false;

		if (lightType > LT_Steady && lightType != LT_BackdropLight)
		{
			// Always invalidate lights that pulse and blink
			needsUpdate = true;
		}

		// Flicker lights flickered randomly every frame in UE1 but this looks terrible at higher refresh rates.
		// Use an upper cap on how often it will flicker that roughly matches what was probably intended.
		if (lightType == LT_Flicker && actor->Light.NextFlickerTime < actor->Level()->TimeSeconds())
		{
			constexpr float flickerFrameRate = 25.0f;
			actor->Light.NextFlickerTime = actor->Level()->TimeSeconds() + 1.0f / flickerFrameRate;
			actor->Light.FlickerRandom = (RandInt(1) != 0);
		}

		// Some light effects animate
		uint8_t lightEffect = actor->LightEffect();
		switch (lightEffect)
		{
		default:
		case LE_None:
		case LE_Warp:
		case LE_CloudCast:
		case LE_Unused:
		case LE_OmniBumpMap:
		case LE_Cylinder:
		case LE_NonIncidence:
		case LE_StaticSpot:
		case LE_Spotlight:
		case LE_Shell:
			// These don't animate
			break;

		case LE_Searchlight:
			// This one only updates if it has a light period
			if (actor->LightPeriod() != 0)
				needsUpdate = true;
			break;

		case LE_TorchWaver:
		case LE_FireWaver:
		case LE_WateryShimmer:
		case LE_SlowWave:
		case LE_FastWave:
		case LE_Shock:
		case LE_Disco:
		case LE_Interference:
		case LE_Rotor:
			needsUpdate = true;
			break;
		}

		if (needsUpdate)
			actor->Light.LastUpdate = FrameCounter;

		LightTree.Lights.push_back(actor);
	}
	LightTree.CreateTLAS();
}

void LightSystem::UpdateLightList(UActor* actor)
{
	vec3 location = actor->Location();

	if (!actor->TouchingLights.NeedsUpdate && actor->TouchingLights.Location == location)
		return;

	actor->TouchingLights.NeedsUpdate = false;
	actor->TouchingLights.Location = location;
	actor->TouchingLights.List.clear();

	if (actor->bUnlit())
		return;

	vec3 extents = actor->BspInfo.BoundingBox.extents();

	LightTree.CollectLights(location, std::max(extents.x, std::max(extents.y, extents.z)));
	for (UActor* light : LightTree.CollectedLights)
	{
		if (!light->bCorona() && !light->bSpecialLit())
		{
			float radius = light->WorldLightRadius();
			vec3 L = light->Location() - location;
			if (light->LightEffect() == LE_Cylinder) // Cylinder lights have infinite Z axis range
			{
				L.z = 0.0f;
			}
			if (dot(L, L) < radius * radius && !engine->Level->Collision.TraceAnyHit(light->Location(), location, actor, false, true, true))
			{
				actor->TouchingLights.List.push_back(light);
			}
		}
	}
}

void LightSystem::SetLevel(ULevel* level)
{
	Level = level;
}
