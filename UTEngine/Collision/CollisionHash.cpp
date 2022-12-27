
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

double CollisionHash::RayCylinderIntersect(const dvec3& start, const dvec3& end, const dvec3& rayDirNormalized, double tmin, double tmax, const dvec3& cylinderCenter, double cylinderHeight, double cylinderRadius)
{
	dvec3 dir2d(rayDirNormalized.x, rayDirNormalized.y, 0);
	dir2d = normalize(dir2d);

	dvec3 c(0, 0, start.z);
	dvec3 cp = c - start;

	double cplen = length(cp);
	double tp = dot(cp, dir2d);

	double cp2 = cplen * cplen;
	double tp2 = tp * tp;

	double tc2 = cp2 - tp2;
	double tc = sqrt(tc2);

	if (tc > cylinderRadius)
		return tmax;

	double r2 = cylinderRadius * cylinderRadius;
	double ti = sqrt(std::max(r2 - tc2, 0.0));

	double ip = tp - ti;

	dvec3 cpnorm = normalize(cp);
	double cosb = dot(rayDirNormalized, cpnorm);
	double ppp = ip / cosb;

	dvec3 dirscaled = rayDirNormalized * ppp;
	dvec3 intersect = start + dirscaled;

	intersect.z = abs(intersect.z);
	double hHalf = cylinderHeight * 0.5;
	double t = tmax;
	if (intersect.z > hHalf)
	{
		dvec3 zdir(0.0, 0.0, -1.0);
		if (rayDirNormalized.z > 0.0)
			zdir.z = 1.0;

		double h = abs(start.z) - hHalf;
		double cosw = dot(rayDirNormalized, zdir);
		t = h / cosw;

		dirscaled = rayDirNormalized * t;
		dvec3 pcap = start + dirscaled;
		double rad = sqrt(pcap.x * pcap.x + pcap.y * pcap.y);

		if (rad > cylinderRadius)
			return tmax;
	}
	else
	{
		t = ppp;
	}

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
	if (actor->Brush()) // Ignore brushes for now
		return tmax;

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
	if (actor->Brush()) // Ignore brushes for now
		return tmax;

	float height = actor->CollisionHeight();
	float radius = actor->CollisionRadius();
	vec3 offset = vec3(0.0, 0.0, height - radius);
	dvec3 sphere0 = to_dvec3(actor->Location() - offset);
	dvec3 sphere1 = to_dvec3(actor->Location() + offset);
	double t0 = RaySphereIntersect(origin, tmin, dirNormalized, tmax, sphere0, radius + sphereRadius);
	double t1 = RaySphereIntersect(origin, tmin, dirNormalized, tmax, sphere1, radius + sphereRadius);
	return std::min(t0, t1);
}

double CollisionHash::ActorCylinderIntersect(const vec3& start, const vec3& end, const dvec3& dirNormalized, double tmin, double tmax, UActor* actor)
{
	if ( actor->Brush() ) // Ignore brushes for now
		return tmax;

	double height = actor->CollisionHeight();
	double radius = actor->CollisionRadius();
	dvec3 location = to_dvec3(actor->Location());
	const dvec3& dstart = to_dvec3(start);
	const dvec3& dend = to_dvec3(end);
	return RayCylinderIntersect(dstart, dend, dirNormalized, tmin, tmax, location, height, radius);
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
