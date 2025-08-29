
#include "Precomp.h"
#include "OverlapTest.h"
#include "Collision/BottomLevel/OverlapAABBModel.h"
#include "UObject/UActor.h"

CollisionHitList OverlapTester::TestOverlap(const vec3& location, float height, float radius, bool testActors, bool testWorld, bool visibilityOnly)
{
	if (!testActors && !testWorld)
		return {};

	vec3 extents = { radius, radius, height };

	CollisionHitList hits;

	if (testActors)
	{
		double dradius = radius;
		double dheight = height;
		dvec3 dlocation = to_dvec3(location);

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
						for (UActor* actor : GetActors(x, y, z))
						{
							if (actor->Collision.CollisionCheckCounter != checkCounter)
							{
								actor->Collision.CollisionCheckCounter = checkCounter;
								if (CylinderActorOverlap(dlocation, dheight, dradius, actor))
								{
									vec3 normal(0.0f); // To do: do we need the normal for contact tests?
									hits.push_back({ 0.0f, normal, actor, nullptr, nullptr });
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
			CollisionHitList worldHits = overlapmodel.TestOverlap(GetLevel()->Model, location, extents, visibilityOnly);
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

Array<UActor*> OverlapTester::CollidingActors(const vec3& origin, float radius)
{
	dvec3 dorigin = to_dvec3(origin);
	double dradius = radius;
	vec3 extents = { radius, radius, radius };

	Array<UActor*> hits;

	int checkCounter = NextCheckCounter();
	ivec3 start = GetStartExtents(origin, extents);
	ivec3 end = GetEndExtents(origin, extents);
	if (end.x - start.x < 100 && end.y - start.y < 100 && end.z - start.z < 100)
	{
		for (int z = start.z; z < end.z; z++)
		{
			for (int y = start.y; y < end.y; y++)
			{
				for (int x = start.x; x < end.x; x++)
				{
					for (UActor* actor : GetActors(x, y, z))
					{
						if (actor->Collision.CollisionCheckCounter != checkCounter)
						{
							actor->Collision.CollisionCheckCounter = checkCounter;
							if (SphereActorOverlap(dorigin, dradius, actor))
								hits.push_back(actor);
						}
					}
				}
			}
		}
	}

	std::set<UActor*> seenActors;
	Array<UActor*> uniqueHits;
	uniqueHits.reserve(hits.size());
	for (auto& hit : hits)
	{
		if (hit)
		{
			if (seenActors.find(hit) == seenActors.end())
			{
				seenActors.insert(hit);
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

Array<UActor*> OverlapTester::CollidingActors(const vec3& origin, float height, float radius)
{
	dvec3 dorigin = to_dvec3(origin);
	double dheight = height;
	double dradius = radius;
	vec3 extents = { radius, radius, height };

	Array<UActor*> hits;

	int checkCounter = NextCheckCounter();
	ivec3 start = GetStartExtents(origin, extents);
	ivec3 end = GetEndExtents(origin, extents);
	if (end.x - start.x < 100 && end.y - start.y < 100 && end.z - start.z < 100)
	{
		for (int z = start.z; z < end.z; z++)
		{
			for (int y = start.y; y < end.y; y++)
			{
				for (int x = start.x; x < end.x; x++)
				{
					for (UActor* actor : GetActors(x, y, z))
					{
						if (actor->Collision.CollisionCheckCounter != checkCounter)
						{
							actor->Collision.CollisionCheckCounter = checkCounter;
							if (CylinderActorOverlap(dorigin, dheight, dradius, actor))
								hits.push_back(actor);
						}
					}
				}
			}
		}
	}

	std::set<UActor*> seenActors;
	Array<UActor*> uniqueHits;
	uniqueHits.reserve(hits.size());
	for (auto& hit : hits)
	{
		if (hit)
		{
			if (seenActors.find(hit) == seenActors.end())
			{
				seenActors.insert(hit);
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

Array<UActor*> OverlapTester::EncroachingActors(UActor* actor)
{
	UModel* brush = actor->Brush();
	if (!brush)
		return CollidingActors(actor->Location(), actor->CollisionHeight(), actor->CollisionRadius());

	// To do: is radius and height correct for a mover? Should it use the brush bounding box?

	vec3 origin = actor->Location();
	float height = actor->CollisionHeight();
	float radius = actor->CollisionRadius();
	vec3 extents = { radius, radius, height };

	mat4 rotateObjToWorld = Coords::Rotation(actor->Rotation()).ToMatrix();
	mat4 rotateWorldToObj = mat4::transpose(rotateObjToWorld);
	vec3 prePivot = actor->PrePivot();

	vec3 mainScale(1.0f);
	if (UMover* mover = UActor::TryCast<UMover>(actor))
	{
		mainScale = mover->MainScale().Scale;
	}

	Array<UActor*> hits;

	int checkCounter = NextCheckCounter();
	ivec3 start = GetStartExtents(origin, extents);
	ivec3 end = GetEndExtents(origin, extents);
	if (end.x - start.x < 100 && end.y - start.y < 100 && end.z - start.z < 100)
	{
		for (int z = start.z; z < end.z; z++)
		{
			for (int y = start.y; y < end.y; y++)
			{
				for (int x = start.x; x < end.x; x++)
				{
					for (UActor* testActor : GetActors(x, y, z))
					{
						if (actor->Collision.CollisionCheckCounter != checkCounter)
						{
							actor->Collision.CollisionCheckCounter = checkCounter;
							if (testActor == actor || testActor->Brush())
								continue;

							vec3 localOrigin = (rotateWorldToObj * vec4(testActor->Location() - origin, 1.0f)).xyz() / mainScale + prePivot;

							float testHeight = testActor->CollisionHeight();
							float testRadius = testActor->CollisionRadius();
							vec3 testExtents = { testRadius, testRadius, testHeight };
							testExtents /= mainScale;

							OverlapAABBModel test;
							auto modelHits = test.TestOverlap(brush, localOrigin, testExtents, false);
							if (!modelHits.empty())
								hits.push_back(testActor);
						}
					}
				}
			}
		}
	}

	std::set<UActor*> seenActors;
	Array<UActor*> uniqueHits;
	uniqueHits.reserve(hits.size());
	for (auto& hit : hits)
	{
		if (hit)
		{
			if (seenActors.find(hit) == seenActors.end())
			{
				seenActors.insert(hit);
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

bool OverlapTester::IsOverlapping(UActor* actor1, UActor* actor2)
{
	return CylinderActorOverlap(to_dvec3(actor1->Location()), actor1->CollisionHeight(), actor1->CollisionRadius(), actor2);
}

bool OverlapTester::CylinderCylinderOverlap(const dvec3& cylinderCenterA, double cylinderHeightA, double cylinderRadiusA, const dvec3& cylinderCenterB, double cylinderHeightB, double cylinderRadiusB)
{
	dvec3 dist = cylinderCenterA - cylinderCenterB;
	double h = cylinderHeightA + cylinderHeightB;
	double r = cylinderRadiusA + cylinderRadiusB;
	return std::abs(dist.z) < h && dot(dist.xy(), dist.xy()) < r * r;
}

// Returns the squared distance between point c and segment ab
static double SqDistPointSegment(const dvec3& a, const dvec3& b, const dvec3& c)
{
	dvec3 ab = b - a;
	dvec3 ac = c - a;
	double e = dot(ac, ab);

	// Handle cases where c projects outside ab
	if (e <= 0.0)
	{
		return dot(ac, ac);
	}
	double f = dot(ab, ab);
	if (e >= f)
	{
		dvec3 bc = c - b;
		return dot(bc, bc);
	}

	// Handle cases where c projects onto ab
	return dot(ac, ac) - e * e / f;
}

bool OverlapTester::SphereCapsuleOverlap(const dvec3& sphereCenter, double sphereRadius, const dvec3& capsuleCenter, double capsuleHeight, double capsuleRadius)
{
	dvec3 base = capsuleCenter - capsuleHeight;
	dvec3 tip = capsuleCenter + capsuleHeight;

	// Compute (squared) distance between sphere center and capsule line segment
	double dist2 = SqDistPointSegment(base, tip, sphereCenter);

	// If (squared) distance smaller than (squared) sum of radii, they collide
	double totalradius = sphereRadius + capsuleRadius;
	return dist2 <= totalradius * totalradius;
}

bool OverlapTester::SphereActorOverlap(const dvec3& sphereCenter, double sphereRadius, UActor* actor)
{
	if (actor->Brush()) // Ignore brushes for now
		return false;

	dvec3 center = to_dvec3(actor->Location());
	double height = actor->CollisionHeight();
	double radius = actor->CollisionRadius();

	// Note: this isn't entirely accurate - what does UE1 do?
	return SphereCapsuleOverlap(sphereCenter, sphereRadius, center, std::max(height - radius, 0.0), radius);
}

bool OverlapTester::CylinderActorOverlap(const dvec3& origin, double cylinderHeight, double cylinderRadius, UActor* actor)
{
	if (actor->Brush()) // Ignore brushes for now
		return false;

	dvec3 center = to_dvec3(actor->Location());
	double height = actor->CollisionHeight();
	double radius = actor->CollisionRadius();
	return CylinderCylinderOverlap(origin, cylinderHeight, cylinderRadius, center, height, radius);
}
