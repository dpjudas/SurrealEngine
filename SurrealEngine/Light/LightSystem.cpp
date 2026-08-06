
#include "Precomp.h"
#include "LightSystem.h"
#include "Packages/Engine/Actors/UActor.h"
#include "Packages/Engine/Resources/Level/ULevel.h"
#include "Packages/Engine/Resources/Level/UModel.h"
#include "Math/floating.h"
#include "Math/coords.h"
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
}

void LightSystem::UpdateLightList(UActor* actor)
{
	vec3 location = actor->BspInfo.BoundingBox.center();

	if (!actor->TouchingLights.NeedsUpdate && actor->TouchingLights.Location == location)
		return;

	actor->TouchingLights.NeedsUpdate = false;
	actor->TouchingLights.Location = location;
	actor->TouchingLights.List.clear();

	if (actor->bUnlit())
		return;

	vec3 extents = actor->BspInfo.BoundingBox.extents();

	int checkCounter = NextCheckCounter();
	ivec3 start = GetStartExtents(location, extents);
	ivec3 end = GetEndExtents(location, extents);
	if (end.x - start.x < 100 && end.y - start.y < 100 && end.z - start.z < 100)
	{
		for (int z = start.z; z < end.z; z++)
		{
			for (int y = start.y; y < end.y; y++)
			{
				for (int x = start.x; x < end.x; x++)
				{
					for (UActor* light : GetActors(x, y, z))
					{
						if (light->TouchingLights.CheckCounter != checkCounter)
						{
							light->TouchingLights.CheckCounter = checkCounter;
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
				}
			}
		}
	}
}

void LightSystem::SetLevel(ULevel* level)
{
	Level = level;
}

void LightSystem::AddLight(UActor* light)
{
	if (light->LightType() != LT_None && light->LightBrightness() > 0)
	{
		vec3 location = light->Location();
		float radius = light->WorldLightRadius();

		light->Light.Inserted = true;
		light->Light.Location = location;
		light->Light.Radius = radius;
		light->Light.LastUpdate = FrameCounter;

		ivec3 start = GetStartExtents(location, radius);
		ivec3 end = GetEndExtents(location, radius);
		for (int z = start.z; z < end.z; z++)
		{
			for (int y = start.y; y < end.y; y++)
			{
				for (int x = start.x; x < end.x; x++)
				{
					LightActors[GetBucketId(x, y, z)].push_back(light);
				}
			}
		}
	}
}

void LightSystem::RemoveLight(UActor* light)
{
	if (light->Light.Inserted)
	{
		vec3 location = light->Light.Location;
		float radius = light->Light.Radius;

		ivec3 start = GetStartExtents(location, radius);
		ivec3 end = GetEndExtents(location, radius);
		for (int z = start.z; z < end.z; z++)
		{
			for (int y = start.y; y < end.y; y++)
			{
				for (int x = start.x; x < end.x; x++)
				{
					auto it = LightActors.find(GetBucketId(x, y, z));
					if (it != LightActors.end())
					{
						it->second.remove(light);
						if (it->second.empty())
							LightActors.erase(it);
					}
				}
			}
		}

		light->Light.Inserted = false;
		light->Light.LastUpdate = FrameCounter;
	}
}
