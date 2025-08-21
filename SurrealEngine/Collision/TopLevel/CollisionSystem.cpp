
#include "Precomp.h"
#include "CollisionSystem.h"
#include "UObject/UActor.h"
#include "Math/floating.h"
#include "Math/coords.h"
#include "Collision/BottomLevel/TraceRayModel.h"
#include "Collision/TopLevel/OverlapTest.h"
#include "Collision/TopLevel/TraceTest.h"

bool CollisionSystem::IsOverlapping(UActor* actor1, UActor* actor2)
{
	OverlapTester overlap(this);
	return overlap.IsOverlapping(actor1, actor2);
}

CollisionHitList CollisionSystem::OverlapTest(UActor* actor)
{
	return OverlapTest(actor->Location(), actor->CollisionHeight(), actor->CollisionRadius(), true, false, false);
}

CollisionHitList CollisionSystem::OverlapTest(const vec3& location, float height, float radius, bool testActors, bool testWorld, bool visibilityOnly)
{
	OverlapTester overlap(this);
	return overlap.TestOverlap(location, height, radius, false, true, false);
}

bool CollisionSystem::TraceAnyHit(vec3 from, vec3 to, UActor* tracingActor, bool traceActors, bool traceWorld, bool visibilityOnly)
{
	TraceTester trace(this);
	return trace.TraceAnyHit(from, to, tracingActor, traceActors, traceWorld, visibilityOnly);
}

CollisionHitList CollisionSystem::Trace(const vec3& from, const vec3& to, float height, float radius, bool traceActors, bool traceWorld, bool visibilityOnly)
{
	TraceTester trace(this);
	return trace.Trace(from, to, height, radius, traceActors, traceWorld, visibilityOnly);
}

CollisionHitList CollisionSystem::TraceDecal(const dvec3& origin, double tmin, const dvec3& dirNormalized, double tmax, bool visibilityOnly)
{
	TraceRayModel trace;
	return trace.Trace(Level->Model, origin, tmin, dirNormalized, tmax, visibilityOnly);
}

CollisionHit CollisionSystem::TraceFirstHit(const vec3& from, const vec3& to, UActor* tracingActor, const vec3& extents, const TraceFlags& flags)
{
	for (const CollisionHit& hit : Trace(from, to, extents.z, extents.x, flags.traceActors(), flags.traceWorld(), false))
	{
		if (hit.Actor && (!tracingActor || !tracingActor->IsOwnedBy(hit.Actor)))
		{
			if (hit.Actor->IsA("Pawn"))
			{
				if (flags.pawns)
					return hit;
			}
			else if (hit.Actor->IsA("Mover"))
			{
				if (flags.movers)
					return hit;
			}
			else if (hit.Actor->IsA("ZoneInfo"))
			{
				if (flags.zoneChanges)
					return hit;
			}
			else if (flags.others)
			{
				if (!flags.onlyProjectiles || hit.Actor->bProjTarget() || (hit.Actor->bBlockActors() && hit.Actor->bBlockPlayers()))
					return hit;
			}
		}
		else if (flags.world && !hit.Actor)
		{
			CollisionHit worldHit = hit;
			if (tracingActor)
				worldHit.Actor = tracingActor->Level();
			return worldHit;
		}
	}
	return {};
}

Array<UActor*> CollisionSystem::CollidingActors(const vec3& origin, float radius)
{
	OverlapTester overlap(this);
	return overlap.CollidingActors(origin, radius);
}

Array<UActor*> CollisionSystem::CollidingActors(const vec3& origin, float height, float radius)
{
	OverlapTester overlap(this);
	return overlap.CollidingActors(origin, height, radius);
}

Array<UActor*> CollisionSystem::EncroachingActors(UActor* actor)
{
	OverlapTester overlap(this);
	return overlap.EncroachingActors(actor);
}

void CollisionSystem::SetLevel(ULevel* level)
{
	Level = level;
}

void CollisionSystem::AddToCollision(UActor* actor)
{
	if (actor->bCollideActors())
	{
		vec3 location = actor->Location();
		float height = actor->CollisionHeight();
		float radius = actor->CollisionRadius();
		vec3 extents = { radius, radius, height };

		actor->Collision.Inserted = true;
		actor->Collision.Location = location;
		actor->Collision.Height = height;
		actor->Collision.Radius = radius;

		ivec3 start = GetStartExtents(location, extents);
		ivec3 end = GetEndExtents(location, extents);
		for (int z = start.z; z < end.z; z++)
		{
			for (int y = start.y; y < end.y; y++)
			{
				for (int x = start.x; x < end.x; x++)
				{
					CollisionActors[GetBucketId(x, y, z)].push_back(actor);
				}
			}
		}
	}
}

void CollisionSystem::RemoveFromCollision(UActor* actor)
{
	if (actor->Collision.Inserted)
	{
		vec3 location = actor->Collision.Location;
		float height = actor->Collision.Height;
		float radius = actor->Collision.Radius;
		vec3 extents = { radius, radius, height };

		ivec3 start = GetStartExtents(location, extents);
		ivec3 end = GetEndExtents(location, extents);
		for (int z = start.z; z < end.z; z++)
		{
			for (int y = start.y; y < end.y; y++)
			{
				for (int x = start.x; x < end.x; x++)
				{
					auto it = CollisionActors.find(GetBucketId(x, y, z));
					if (it != CollisionActors.end())
					{
						it->second.remove(actor);
						if (it->second.empty())
							CollisionActors.erase(it);
					}
				}
			}
		}

		actor->Collision.Inserted = false;
	}
}
