
#include "Precomp.h"
#include "CollisionHash.h"
#include "UObject/UActor.h"

void CollisionHash::AddToCollision(UActor* actor)
{
	if (actor->bCollideActors())
	{
		vec3 location = actor->Location();
		float height = actor->CollisionHeight();
		float radius = actor->CollisionRadius();
		vec3 extents = { radius, radius, height };

		actor->CollisionHashInfo.Inserted = true;
		actor->CollisionHashInfo.Location = location;
		actor->CollisionHashInfo.Height = height;
		actor->CollisionHashInfo.Radius = radius;

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

void CollisionHash::RemoveFromCollision(UActor* actor)
{
	if (actor->CollisionHashInfo.Inserted)
	{
		vec3 location = actor->CollisionHashInfo.Location;
		float height = actor->CollisionHashInfo.Height;
		float radius = actor->CollisionHashInfo.Radius;
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

		actor->CollisionHashInfo.Inserted = false;
	}
}

double CollisionHash::RaySphereIntersect(const dvec3& rayOrigin, double tmin, const dvec3& rayDirNormalized, double tmax, const dvec3& sphereCenter, double sphereRadius)
{
	dvec3 l = sphereCenter - rayOrigin;
	double s = dot(l, rayDirNormalized);
	double l2 = dot(l, l);
	double r2 = sphereRadius * sphereRadius;
	if (s < 0 && l2 > r2)
		return tmax;
	double s2 = s * s;
	double m2 = l2 - s2;
	if (m2 > r2)
		return tmax;
	double q = std::sqrt(r2 - m2);
	double t = (l2 > r2) ? s - q : s + q;
	return (t >= tmin) ? t : tmax;
}

bool CollisionHash::ActorSphereCollision(const dvec3& origin, double sphereRadius, UActor* actor)
{
	double sphereRadius2 = sphereRadius * sphereRadius;

	float height = actor->CollisionHeight();
	float radius = actor->CollisionRadius();
	vec3 offset = vec3(0.0, 0.0, height - radius);
	dvec3 sphere0 = to_dvec3(actor->Location() - offset);
	dvec3 sphere1 = to_dvec3(actor->Location() + offset);

	dvec3 delta0 = sphere0 - origin;
	if (dot(delta0, delta0) < sphereRadius2)
		return true;
	dvec3 delta1 = sphere1 - origin;
	return dot(delta1, delta1) < sphereRadius2;
}

double CollisionHash::ActorRayIntersect(const dvec3& origin, double tmin, const dvec3& dirNormalized, double tmax, UActor* actor)
{
	float height = actor->CollisionHeight();
	float radius = actor->CollisionRadius();
	vec3 offset = vec3(0.0, 0.0, height - radius);
	dvec3 sphere0 = to_dvec3(actor->Location() - offset);
	dvec3 sphere1 = to_dvec3(actor->Location() + offset);
	double t0 = RaySphereIntersect(origin, tmin, dirNormalized, tmax, sphere0, radius);
	double t1 = RaySphereIntersect(origin, tmin, dirNormalized, tmax, sphere1, radius);
	return std::min(t0, t1);
}

double CollisionHash::ActorSphereIntersect(const dvec3& origin, double tmin, const dvec3& dirNormalized, double tmax, double sphereRadius, UActor* actor)
{
	float height = actor->CollisionHeight();
	float radius = actor->CollisionRadius();
	vec3 offset = vec3(0.0, 0.0, height - radius);
	dvec3 sphere0 = to_dvec3(actor->Location() - offset);
	dvec3 sphere1 = to_dvec3(actor->Location() + offset);
	double t0 = RaySphereIntersect(origin, tmin, dirNormalized, tmax, sphere0, radius + sphereRadius);
	double t1 = RaySphereIntersect(origin, tmin, dirNormalized, tmax, sphere1, radius + sphereRadius);
	return std::min(t0, t1);
}

std::vector<UActor*> CollisionHash::CollidingActors(const vec3& origin, float radius)
{
	dvec3 dorigin = to_dvec3(origin);
	double dradius = radius;
	vec3 extents = { radius, radius, radius };

	std::vector<UActor*> hits;

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
					auto it = CollisionActors.find(GetBucketId(x, y, z));
					if (it != CollisionActors.end())
					{
						for (UActor* actor : it->second)
						{
							if (ActorSphereCollision(dorigin, dradius, actor))
								hits.push_back(actor);
						}
					}
				}
			}
		}
	}

	std::set<UActor*> seenActors;
	std::vector<UActor*> uniqueHits;
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
