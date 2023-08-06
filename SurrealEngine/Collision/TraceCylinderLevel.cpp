
#include "Precomp.h"
#include "TraceCylinderLevel.h"
#include "TraceAABBModel.h"
#include "TraceRayModel.h"
#include "UObject/UActor.h"

SweepHitList TraceCylinderLevel::Trace(ULevel* level, const vec3& from, const vec3& to, float height, float radius, bool traceActors, bool traceWorld, bool visibilityOnly)
{
	if (from == to || (!traceActors && !traceWorld))
		return {};

	Level = level;

	dvec3 origin = to_dvec3(from);
	dvec3 rayEnd = to_dvec3(to);
	dvec3 direction = to_dvec3(to) - origin;
	double tmin = 0.01f;
	double tmax = length(direction);
	if (tmax < tmin)
		return {};
	direction *= 1.0f / tmax;

	float margin = 1.0f;
	tmax += margin;

	std::vector<SweepHit> hits;

	if (traceActors)
	{
		double dradius = radius;
		double dheight = height;
		vec3 extents = { radius, radius, height };

		ivec3 start = Level->Hash.GetSweepStartExtents(from, to, extents);
		ivec3 end = Level->Hash.GetSweepEndExtents(from, to, extents);
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
								double t = Level->Hash.CylinderActorTrace(origin, tmin, direction, tmax, dheight, dradius, actor);
								if (t < tmax)
								{
									dvec3 hitpos = origin + direction * t;
									hits.push_back({ (float)t, normalize(to_vec3(hitpos) - actor->Location()), actor });
								}
							}
						}
					}
				}
			}
		}
	}

	if (traceWorld)
	{
		dvec3 extents = { (double)radius, (double)radius, (double)height };
		if (extents == dvec3(0.0, 0.0, 0.0))
		{
			// Line/triangle intersect
			TraceRayModel tracemodel;
			TraceHitList worldHits = tracemodel.Trace(Level->Model, origin, tmin, direction, tmax, visibilityOnly);
			hits.insert(hits.end(), worldHits.begin(), worldHits.end());
		}
		else
		{
			// AABB/Triangle intersect
			TraceAABBModel tracemodel;
			SweepHitList worldHits = tracemodel.Trace(Level->Model, origin, tmin, direction, tmax, extents, visibilityOnly);
			hits.insert(hits.end(), worldHits.begin(), worldHits.end());
		}
	}

	// Sort by closest hit and only include the first hit for each actor

	std::stable_sort(hits.begin(), hits.end(), [](const auto& a, const auto& b) { return a.Fraction < b.Fraction; });
	std::set<UActor*> seenActors;
	SweepHitList uniqueHits;
	for (auto& hit : hits)
	{
		if (hit.Actor)
		{
			if (seenActors.find(hit.Actor) == seenActors.end())
			{
				seenActors.insert(hit.Actor);
				uniqueHits.push_back(hit);
			}
		}
		else
		{
			uniqueHits.push_back(hit);
		}
	}

	tmax -= margin;
	for (auto& hit : uniqueHits)
	{
		hit.Fraction = (float)(std::max(hit.Fraction - margin, 0.0f) / tmax);
	}

	return uniqueHits;
}
