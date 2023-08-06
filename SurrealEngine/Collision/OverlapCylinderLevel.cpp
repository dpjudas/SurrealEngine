
#include "Precomp.h"
#include "OverlapCylinderLevel.h"
#include "OverlapAABBModel.h"

CollisionHitList OverlapCylinderLevel::TestOverlap(ULevel* level, const vec3& location, float height, float radius, bool testActors, bool testWorld, bool visibilityOnly)
{
	if (!testActors && !testWorld)
		return {};

	Level = level;

	vec3 extents = { radius, radius, height };

	CollisionHitList hits;

	if (testActors)
	{
		double dradius = radius;
		double dheight = height;
		dvec3 dlocation = to_dvec3(location);

		ivec3 start = Level->Hash.GetStartExtents(location, extents);
		ivec3 end = Level->Hash.GetEndExtents(location, extents);
		if (end.x - start.x < 100 && end.y - start.y < 100 && end.z - start.z < 100)
		{
			for (int z = start.z; z < end.z; z++)
			{
				for (int y = start.y; y < end.y; y++)
				{
					for (int x = start.x; x < end.x; x++)
					{
						auto it = Level->Hash.CollisionActors.find(Level->Hash.GetBucketId(x, y, z));
						if (it != Level->Hash.CollisionActors.end())
						{
							for (UActor* actor : it->second)
							{
								if (Level->Hash.CylinderActorOverlap(dlocation, dheight, dradius, actor))
								{
									vec3 normal(0.0f); // To do: do we need the normal for contact tests?
									hits.push_back({ 0.0f, normal, actor, nullptr });
								}
							}
						}
					}
				}
			}
		}
	}

	if (testWorld)
	{
		if (extents != vec3(0.0f))
		{
			OverlapAABBModel overlapmodel;
			CollisionHitList worldHits = overlapmodel.TestOverlap(Level->Model, location, extents, visibilityOnly);
			hits.push_back(worldHits);
		}
	}

	// Only include each actor once

	std::stable_sort(hits.begin(), hits.end(), [](const auto& a, const auto& b) { return a.Actor < b.Actor; });

	UActor* prevActor = nullptr;
	CollisionHitList uniqueHits;
	for (auto& hit : hits)
	{
		if (hit.Actor)
		{
			if (hit.Actor != prevActor)
			{
				prevActor = hit.Actor;
				uniqueHits.push_back(hit);
			}
		}
		else
		{
			uniqueHits.push_back(hit);
		}
	}

	return uniqueHits;
}
