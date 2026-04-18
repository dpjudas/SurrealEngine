
#include "Precomp.h"
#include "LightSystem.h"
#include "UObject/UActor.h"
#include "UObject/ULevel.h"
#include "Math/floating.h"
#include "Math/coords.h"
#include "Engine.h"

void LightSystem::UpdateLightList(UActor* actor)
{
	vec3 location = actor->BspInfo.BoundingBox.center();

	if (!actor->LightInfo.NeedsUpdate && actor->LightInfo.Location == location)
		return;

	actor->LightInfo.NeedsUpdate = false;
	actor->LightInfo.Location = location;
	actor->LightInfo.LightList.clear();

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
						if (light->Light.CheckCounter != checkCounter)
						{
							light->Light.CheckCounter = checkCounter;
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
									actor->LightInfo.LightList.push_back(light);
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
	}
}
