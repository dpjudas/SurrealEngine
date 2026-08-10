
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
		if (actor && actor->LightType() != LT_None && actor->LightBrightness() > 0)
		{
			if (actor->LightType() > LT_Steady && actor->LightType() != LT_BackdropLight)
			{
				// Always invalidate lights that pulse and blink
				actor->Light.LastUpdate = FrameCounter;

				// Flicker lights flickered randomly every frame in UE1 but this looks terrible at higher refresh rates.
				// Use an upper cap on how often it will flicker that roughly matches what was probably intended.
				if (actor->LightType() == LT_Flicker && actor->Light.NextFlickerTime < actor->Level()->TimeSeconds())
				{
					constexpr float flickerFrameRate = 20.0f;
					actor->Light.NextFlickerTime = actor->Level()->TimeSeconds() + 1.0f / flickerFrameRate;
					actor->Light.FlickerRandom = (RandInt(1) != 0);
				}
			}

			LightTree.Lights.push_back(actor);
		}
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
