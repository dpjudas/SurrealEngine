
#include "Precomp.h"
#include "CollisionHash.h"
#include "UObject/UActor.h"
#include "Math/floating.h"

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

double CollisionHash::RaySphereTrace(const dvec3& rayOrigin, double tmin, const dvec3& rayDirNormalized, double tmax, const dvec3& sphereCenter, double sphereRadius)
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

static int GetQuadraticRoots(double a, double b, double c, double& root_lower, double& root_upper)
{
	double discriminant = (b * b) - (4.0 * a * c);
	if (discriminant > DBL_EPSILON)
	{
		double b_term = b < DBL_EPSILON ? -b + sqrt(discriminant) : -b - sqrt(discriminant);

		root_lower = b_term / (2.0 * a); // quadratic formula
		root_upper = (2.0 * c) / b_term; // citardauq formula

		if (root_lower > root_upper)
			std::swap(root_lower, root_upper); // use of both formulae, plus this, avoids catastrophic cancellation due to floating-point limits

		return 2;
	}
	else if (discriminant > -DBL_EPSILON && discriminant <= DBL_EPSILON)
	{
		root_lower = -(b / 2.0 * a);
		root_upper = root_lower;
		return 1;
	}
	root_lower = NAN;
	root_upper = NAN;
	return 0;
}

// Compute the intersection of a ray and a plane with infinite bounds. The ray direction 
// must be normalized. Returns the hit distance, from the ray's origin; to get the hit 
// position, multiply that by the ray's direction and then add the ray's origin.
bool CollisionHash::RayPlaneTrace(const dvec3& rayOrigin, const dvec3& rayDirNormalized, const dvec3& planeOrigin, const dvec3& planeNormal, double& t)
{
	double denom = dot(planeNormal, rayDirNormalized);
	//
	// Non-zero check (accounting for floating-point imprecision):
	//
	if (denom > DBL_EPSILON || denom < -DBL_EPSILON)
	{
		double hd = dot(planeOrigin - rayOrigin, planeNormal) / denom;
		if (hd >= 0)
		{
			t = hd;
			return true;
		}
	}
	return false;
}

// Compute the intersection of a ray and a disc. The ray direction must be normalized. 
// Returns the hit distance, from the ray's origin.
bool CollisionHash::RayCircleTrace(const dvec3& rayOrigin, const dvec3& rayDirNormalized, const dvec3& circleCenter, const dvec3& circleNormal, double radius, double& t)
{
	double hd;
	bool plane = RayPlaneTrace(rayOrigin, rayDirNormalized, circleCenter, circleNormal, hd);
	if (!plane)
		return false;
	dvec3 hp = rayOrigin + rayDirNormalized * hd;
	dvec3 dd = hp - circleCenter;
	if (dot(dd, dd) > radius * radius)
		return false;
	//if (Hd < 0) // redundant with checks done in ray/plane
	//   return false;
	t = hd;
	return true;
}

double CollisionHash::RayCylinderTrace(const dvec3& rayOrigin, const dvec3& rayDirNormalized, double tmin, double tmax, const dvec3& cylinderCenter, double cylinderHeight, double cylinderRadius)
{
	//
	// First, identify intersections between a line and an infinite cylinder. An infinite 
	// cylinder has no base and extends in both directions.
	//
	dvec3 ct = cylinderCenter;
	dvec3 cb = cylinderCenter;

	ct.z += cylinderHeight;
	cb.z -= cylinderHeight;

	dvec3 rl = rayOrigin - cb;  // Ray origin local to centerpoint
	dvec3 cs = ct - cb;         // Cylinder spine
	double ch = length(cs);     // Cylinder height
	dvec3 ca = cs / ch;         // Cylinder axis

	auto caDotRd = dot(ca, rayDirNormalized);
	auto caDotRl = dot(ca, rl);
	auto rlDotRl = dot(rl, rl);

	double a = 1 - (caDotRd * caDotRd);
	double b = 2 * (dot(rayDirNormalized, rl) - caDotRd * caDotRl);
	double c = rlDotRl - caDotRl * caDotRl - (cylinderRadius * cylinderRadius);

	double t0;
	double t1;
	int numRoots = GetQuadraticRoots(a, b, c, t0, t1);
	if (numRoots == 0)
	{
		//
		// There is no intersection between a line (i.e. a "double-sided" ray) and the 
		// infinite cylinder that matches our finite cylinder. This means that we cannot 
		// be hitting any part of the cylinder: if we were hitting the base from the 
		// inside, for example, then the "back of our ray" would be hitting the upper 
		// part of the infinite cylinder.
		//
		return tmax;
	}

	bool valid1 = true;
	bool valid2 = true;
	//
	dvec3 hp1 = rayOrigin + rayDirNormalized * t0;
	dvec3 hp2 = rayOrigin + rayDirNormalized * t1;
	double ho1 = dot(ct - hp1, ca); // height offset
	double ho2 = dot(ct - hp2, ca);
	//
	int validRoots = numRoots;
	if (t0 < 0.0 || ho1 < DBL_EPSILON || ho1 > ch)
	{
		valid1 = false;
		--validRoots;
	}
	if (t1 < 0.0 || ho2 < DBL_EPSILON || ho2 > ch)
	{
		valid2 = false;
		if (numRoots > 1)
		--validRoots;
	}
	double t = tmax;
	if (validRoots == 0)
	{
		//
		// The ray never hits the bounded cylinder's curved surface. If we're looking 
		// along the cylinder's axis -- whether from inside or outside -- then the ray 
		// could still hit an endcap.
		// 
		// Let's project the ray origin onto the cylinder's axis, and figure out which 
		// endcap we're nearer to. (Well, actually, we already have that value: it's 
		// Ca_dot_Rl.)
		//
		if (caDotRl <= 0.0)
		{
			// above
			valid1 = RayCircleTrace(rayOrigin, rayDirNormalized, ct, ca, cylinderRadius, t0);
		}
		else if (caDotRl >= ch)
		{
			// below
			valid1 = RayCircleTrace(rayOrigin, rayDirNormalized, cb, ca, cylinderRadius, t1);
		}
		else
		{
			return tmax;
		}
		if (valid1)
		{
			t = t0;
			return t;
		}
		return tmax;
	}
	if (validRoots == 1)
	{
		//
		// The ray hits the cylinder's curved surface only once. This can only happen under 
		// two cases: the ray originates from inside the cylinder, and points outward; or 
		// the ray passes through the bounded cylinder once and then through an endcap.
		//
		if (valid2)
		{
			hp1 = hp2;
			ho1 = ho2;
			valid1 = true;
			t0 = t1;
		}

		double d0;
		double d1;
		bool disc1 = RayCircleTrace(rayOrigin, rayDirNormalized, ct, ca, cylinderRadius, d0);
		bool disc2 = RayCircleTrace(rayOrigin, rayDirNormalized, cb, ca, cylinderRadius, d1);
		if (disc1)
		{
			if (disc2)
			{
				if (d1 < d0)
				d0 = d1;
			}
		}
		else if (disc2)
		{
			d0 = t1;
			disc1 = disc2;
		}

		if (disc1)
		{
			if (d0 < t0)
			{
				t = d0;
				return t;
			}
		}
		else
		{
			return tmax;
		}
	}
	t = std::min(t0, t1);
	return t;
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

bool CollisionHash::SphereCapsuleOverlap(const dvec3& sphereCenter, double sphereRadius, const dvec3& capsuleCenter, double capsuleHeight, double capsuleRadius)
{
	dvec3 base = capsuleCenter - capsuleHeight;
	dvec3 tip = capsuleCenter + capsuleHeight;

	// Compute (squared) distance between sphere center and capsule line segment
	double dist2 = SqDistPointSegment(base, tip, sphereCenter);

	// If (squared) distance smaller than (squared) sum of radii, they collide
	double totalradius = sphereRadius + capsuleRadius;
	return dist2 <= totalradius * totalradius;
}

bool CollisionHash::CylinderCylinderOverlap(const dvec3& cylinderCenterA, double cylinderHeightA, double cylinderRadiusA, const dvec3& cylinderCenterB, double cylinderHeightB, double cylinderRadiusB)
{
	dvec3 dist = cylinderCenterA - cylinderCenterB;
	double h = cylinderHeightA + cylinderHeightB;
	double r = cylinderRadiusA + cylinderRadiusB;
	return std::abs(dist.z) < h && dot(dist.xy(), dist.xy()) < r * r;
}

double CollisionHash::CylinderCylinderTrace(const dvec3& origin, const dvec3& dirNormalized, double tmin, double tmax, const dvec3& cylinderCenterA, double cylinderHeightA, double cylinderRadiusA, double cylinderHeightB, double cylinderRadiusB)
{
	double h = cylinderHeightA + cylinderHeightB;
	double r = cylinderRadiusA + cylinderRadiusB;

	// Find the trace range where the cylinders can hit each other (ray/planes test):
	double t0, t1;
	if (dirNormalized.z > FLT_EPSILON || dirNormalized.z < -FLT_EPSILON)
	{
		t0 = (cylinderCenterA.z - origin.z - h) / dirNormalized.z;
		t1 = (cylinderCenterA.z - origin.z + h) / dirNormalized.z;
		if (t1 < t0) std::swap(t0, t1);

		// If the trace range is outside the planes then there is no hit
		if (t1 < tmin || t0 >= tmax)
			return tmax;
	}
	else // trace is parallel to the plane - either we are always inside or we are always outside
	{
		if (std::abs(cylinderCenterA.z - origin.z) >= h)
			return tmax;
		t0 = 0.0;
		t1 = 1.0;
	}

	// Test if the first possible hit point is already overlapping. If it is, we hit the top or bottom of the cylinder.
	if (t0 >= tmin)
	{
		dvec2 dist2d = cylinderCenterA.xy() + origin.xy() + dirNormalized.xy() * t0;
		if (dot(dist2d, dist2d) < r * r)
			return t0;
	}

	// Find the when sides might hit each other in the XY plane (ray/circle test):
	dvec2 l = cylinderCenterA.xy() - origin.xy();
	double s = dot(l, dirNormalized.xy());
	double l2 = dot(l, l);
	double r2 = r * r;
	if (s < 0 && l2 > r2)
		return tmax;
	double s2 = s * s;
	double m2 = l2 - s2;
	if (m2 > r2)
		return tmax;
	double q = std::sqrt(r2 - m2);
	double t = (l2 > r2) ? s - q : s + q;
	return (t >= tmin && t >= t0 && t <= t1) ? t : tmax;
}

bool CollisionHash::SphereActorOverlap(const dvec3& sphereCenter, double sphereRadius, UActor* actor)
{
	if (actor->Brush()) // Ignore brushes for now
		return false;
	
	dvec3 center = to_dvec3(actor->Location());
	double height = actor->CollisionHeight();
	double radius = actor->CollisionRadius();

	// Note: this isn't entirely accurate - what does UE1 do?
	return SphereCapsuleOverlap(sphereCenter, sphereRadius, center, std::max(height - radius, 0.0), radius);
}

bool CollisionHash::CylinderActorOverlap(const dvec3& origin, double cylinderHeight, double cylinderRadius, UActor* actor)
{
	if (actor->Brush()) // Ignore brushes for now
		return false;

	dvec3 center = to_dvec3(actor->Location());
	double height = actor->CollisionHeight();
	double radius = actor->CollisionRadius();
	return CylinderCylinderOverlap(origin, cylinderHeight, cylinderRadius, center, height, radius);
}

double CollisionHash::RayActorTrace(const dvec3& origin, double tmin, const dvec3& dirNormalized, double tmax, UActor* actor)
{
	if (actor->Brush()) // Ignore brushes for now
		return tmax;

	dvec3 center = to_dvec3(actor->Location());
	double height = actor->CollisionHeight();
	double radius = actor->CollisionRadius();
	return RayCylinderTrace(origin, dirNormalized, tmin, tmax, center, height, radius);
}

double CollisionHash::CylinderActorTrace(const dvec3& origin, double tmin, const dvec3& dirNormalized, double tmax, double cylinderHeight, double cylinderRadius, UActor* actor)
{
	if (actor->Brush()) // Ignore brushes for now
		return tmax;

	dvec3 center = to_dvec3(actor->Location());
	double height = actor->CollisionHeight();
	double radius = actor->CollisionRadius();

	if (cylinderHeight > 0.0f && cylinderRadius > 0.0f)
	{
		return CylinderCylinderTrace(origin, dirNormalized, tmin, tmax, center, height, radius, cylinderHeight, cylinderRadius);
	}
	else
	{
		// Downgrade to a ray test if the cylinder is a point
		return RayCylinderTrace(origin, dirNormalized, tmin, tmax, center, height, radius);
	}
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
							if (SphereActorOverlap(dorigin, dradius, actor))
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

std::vector<UActor*> CollisionHash::CollidingActors(const vec3& origin, float height, float radius)
{
	dvec3 dorigin = to_dvec3(origin);
	double dheight = height;
	double dradius = radius;
	vec3 extents = { radius, radius, height };

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
							if (CylinderActorOverlap(dorigin, dheight, dradius, actor))
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
