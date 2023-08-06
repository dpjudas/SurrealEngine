
#include "Precomp.h"
#include "TraceRayLevel.h"
#include "TraceRayModel.h"
#include "UObject/UActor.h"

bool TraceRayLevel::TraceAnyHit(ULevel* level, vec3 from, vec3 to, UActor* tracingActor, bool traceActors, bool traceWorld, bool visibilityOnly)
{
	if (from == to || (!traceActors && !traceWorld))
		return false;

	Level = level;

	dvec3 origin = to_dvec3(from);
	dvec3 direction = to_dvec3(to) - origin;
	double tmin = 0.01f;
	double tmax = length(direction);
	if (tmax < tmin)
		return false;
	direction *= 1.0f / tmax;

	float margin = 1.0f;
	tmax += margin;

	if (traceActors)
	{
		ivec3 start = Level->Hash.GetRayStartExtents(from, to);
		ivec3 end = Level->Hash.GetRayEndExtents(from, to);
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
								if (actor != tracingActor && actor->bBlockActors() && Level->Hash.RayActorTrace(origin, tmin, direction, tmax, actor) < tmax)
									return true;
							}
						}
					}
				}
			}
		}
	}

	if (traceWorld)
	{
		TraceRayModel tracemodel;
		return tracemodel.TraceAnyHit(Level->Model, origin, tmin, direction, tmax, visibilityOnly);
	}

	return false;
}
