
#include "Precomp.h"
#include "TraceTest.h"
#include "Collision/BottomLevel/TraceAABBModel.h"
#include "Collision/BottomLevel/TraceRayModel.h"
#include "UObject/UActor.h"

CollisionHitList TraceTester::Trace(const vec3& from, const vec3& to, float height, float radius, bool traceActors, bool traceWorld, bool visibilityOnly)
{
	if (from == to || (!traceActors && !traceWorld))
		return {};

	dvec3 origin = to_dvec3(from);
	dvec3 rayEnd = to_dvec3(to);
	dvec3 direction = to_dvec3(to) - origin;
	double tmin = 0.0f; // if this goes above 0.0, you will be able to walk through walls!
	double tmax = length(direction);
	if (tmax < tmin)
		return {};
	direction *= 1.0f / tmax;

	float margin = 1.0f;
	tmax += margin;

	CollisionHitList hits;

	if (traceActors)
	{
		double dradius = radius;
		double dheight = height;
		vec3 extents = { radius, radius, height };

		ivec3 start = GetSweepStartExtents(from, to, extents);
		ivec3 end = GetSweepEndExtents(from, to, extents);
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
							TraceActor(actor, origin, tmin, direction, tmax, dheight, dradius, hits);
						}
					}
				}
			}
		}
	}

	if (traceWorld)
	{
		if (radius == 0.0 && height == 0.0)
		{
			// Line/triangle intersect
			TraceRayModel tracemodel;
			CollisionHitList worldHits = tracemodel.Trace(GetLevel()->Model, origin, tmin, direction, tmax, visibilityOnly);
			hits.push_back(worldHits);
		}
		else
		{
			// AABB/Triangle intersect
			TraceAABBModel tracemodel;
			dvec3 extents = { (double)radius, (double)radius, (double)height };
			CollisionHitList worldHits = tracemodel.Trace(GetLevel()->Model, origin, tmin, direction, tmax, extents, visibilityOnly);
			hits.push_back(worldHits);
		}
	}

	// Sort by closest hit and only include the first hit for each actor

	std::stable_sort(hits.begin(), hits.end(), [](const auto& a, const auto& b) { return a.Fraction < b.Fraction; });
	std::set<UActor*> seenActors;
	CollisionHitList uniqueHits;
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

bool TraceTester::TraceAnyHit(vec3 from, vec3 to, UActor* tracingActor, bool traceActors, bool traceWorld, bool visibilityOnly)
{
	if (from == to || (!traceActors && !traceWorld))
		return false;

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
		CollisionHitList hits;

		ivec3 start = GetRayStartExtents(from, to);
		ivec3 end = GetRayEndExtents(from, to);
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
							if (actor != tracingActor && actor->bBlockActors())
							{
								TraceActor(actor, origin, tmin, direction, tmax, 0.0, 0.0, hits);
								if (!hits.empty())
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
		return tracemodel.TraceAnyHit(GetLevel()->Model, origin, tmin, direction, tmax, visibilityOnly);
	}

	return false;
}

void TraceTester::TraceActor(UActor* actor, const dvec3& origin, double tmin, const dvec3& dirNormalized, double tmax, double height, double radius, CollisionHitList& hits)
{
	if (UMover* mover = UObject::TryCast<UMover>(actor))
	{
		CollisionHitList brushHits;

		// Note: DrawScale does not affect mover

		//mat4 objectToWorld = mat4::translate(Location()) * Coords::Rotation(Rotation()).ToMatrix() * mat4::translate(-PrePivot()) * mat4::scale(DrawScale());

		mat4 rotateObjToWorld = Coords::Rotation(mover->Rotation()).ToMatrix();
		mat4 rotateWorldToObj = mat4::transpose(rotateObjToWorld);

		dvec3 localOrigin = dvec3(((rotateWorldToObj * vec4(vec3(origin) - mover->Location(), 1.0f)).xyz() + mover->PrePivot()));
		dvec3 localDirection = dvec3((rotateWorldToObj * vec4(vec3(dirNormalized), 1.0f)).xyz());

		double localTMin = tmin;
		double localTMax = tmax;

		if (radius == 0.0 && height == 0.0)
		{
			// Line/triangle intersect
			TraceRayModel tracemodel;
			brushHits = tracemodel.Trace(mover->Brush(), localOrigin, localTMin, localDirection, localTMax, false);
		}
		else
		{
			// AABB/Triangle intersect
			TraceAABBModel tracemodel;
			dvec3 extents = { (double)radius, (double)radius, (double)height };
			brushHits = tracemodel.Trace(mover->Brush(), localOrigin, localTMin, localDirection, localTMax, extents, false);
		}

		if (radius != 0.0 || height != 0.0)
		{
			for (auto& hit : brushHits)
			{
				hit.Actor = actor;
				hit.Normal = (rotateWorldToObj * vec4(hit.Normal, 1.0f)).xyz();
				hits.push_back(hit);
			}
		}
	}
	else
	{
		// Default cylinder
		double t = CylinderActorTrace(origin, tmin, dirNormalized, tmax, height, radius, actor);
		if (t < tmax)
		{
			dvec3 hitpos = origin + dirNormalized * t;
			hits.push_back({ (float)t, normalize(to_vec3(hitpos) - actor->Location()), actor, nullptr, nullptr });
		}
	}
}

double TraceTester::CylinderActorTrace(const dvec3& origin, double tmin, const dvec3& dirNormalized, double tmax, double cylinderHeight, double cylinderRadius, UActor* actor)
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

double TraceTester::CylinderCylinderTrace(const dvec3& origin, const dvec3& dirNormalized, double tmin, double tmax, const dvec3& cylinderCenterA, double cylinderHeightA, double cylinderRadiusA, double cylinderHeightB, double cylinderRadiusB)
{
	// A cylinder/cylinder trace is the same as a ray/cylinder trace, except the cylinder's radius and height has been extended by the other cylinder

	return RayCylinderTrace(origin, dirNormalized, tmin, tmax, cylinderCenterA, cylinderHeightA + cylinderHeightB, cylinderRadiusA + cylinderRadiusB);
}

#if 0 // Shock rifle combo blasts does not work with this version. Why? Does this mean that CylinderCylinderOverlap is broken as it is built upon the same idea?

double CollisionSystem::RayCylinderTrace(const dvec3& origin, const dvec3& dirNormalized, double tmin, double tmax, const dvec3& cylinderCenter, double cylinderHeight, double cylinderRadius)
{
	// Find the trace range where the ray can hit the cylinders (ray/planes test):
	double t0, t1;
	if (dirNormalized.z > FLT_EPSILON || dirNormalized.z < -FLT_EPSILON)
	{
		t0 = (cylinderCenter.z - origin.z - cylinderHeight) / dirNormalized.z;
		t1 = (cylinderCenter.z - origin.z + cylinderHeight) / dirNormalized.z;
		if (t1 < t0) std::swap(t0, t1);

		// If the trace range is outside the planes then there is no hit
		if (t1 < tmin || t0 >= tmax)
			return tmax;
	}
	else // trace is parallel to the plane - either we are always inside or we are always outside
	{
		if (std::abs(cylinderCenter.z - origin.z) >= cylinderHeight)
			return tmax;
		t0 = 0.0;
		t1 = tmax;
	}

	// Test if the first possible hit point is already overlapping. If it is, we hit the top or bottom of the cylinder.
	if (t0 >= tmin)
	{
		dvec2 dist2d = cylinderCenter.xy() + origin.xy() + dirNormalized.xy() * t0;
		if (dot(dist2d, dist2d) < cylinderRadius * cylinderRadius)
			return t0;
	}

	// Find the when the ray might hit the side in the XY plane (ray/circle test):
	dvec2 l = cylinderCenter.xy() - origin.xy();
	double s = dot(l, dirNormalized.xy());
	double l2 = dot(l, l);
	double r2 = cylinderRadius * cylinderRadius;
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

#else

static int GetQuadraticRoots(double a, double b, double c, double& root_lower, double& root_upper)
{
	double discriminant = (b * b) - (4.0 * a * c);
	if (discriminant > FLT_EPSILON)
	{
		double b_term = b < FLT_EPSILON ? -b + sqrt(discriminant) : -b - sqrt(discriminant);

		root_lower = b_term / (2.0 * a); // quadratic formula
		root_upper = (2.0 * c) / b_term; // citardauq formula

		if (root_lower > root_upper)
			std::swap(root_lower, root_upper); // use of both formulae, plus this, avoids catastrophic cancellation due to floating-point limits

		return 2;
	}
	else if (discriminant > -FLT_EPSILON && discriminant <= FLT_EPSILON)
	{
		root_lower = -(b / 2.0 * a);
		root_upper = root_lower;
		return 1;
	}
	root_lower = NAN;
	root_upper = NAN;
	return 0;
}

double TraceTester::RayCylinderTrace(const dvec3& rayOrigin, const dvec3& rayDirNormalized, double tmin, double tmax, const dvec3& cylinderCenter, double cylinderHeight, double cylinderRadius)
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
			valid2 = RayCircleTrace(rayOrigin, rayDirNormalized, cb, -ca, cylinderRadius, t1);
		}

		if (valid1)
			return t0;
		else if (valid2)
			return t1;
		else
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
		bool disc2 = RayCircleTrace(rayOrigin, rayDirNormalized, cb, -ca, cylinderRadius, d1);
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

#endif

double TraceTester::RayActorTrace(const dvec3& origin, double tmin, const dvec3& dirNormalized, double tmax, UActor* actor)
{
	if (actor->Brush()) // Ignore brushes for now
		return tmax;

	dvec3 center = to_dvec3(actor->Location());
	double height = actor->CollisionHeight();
	double radius = actor->CollisionRadius();
	return RayCylinderTrace(origin, dirNormalized, tmin, tmax, center, height, radius);
}

double TraceTester::RaySphereTrace(const dvec3& rayOrigin, double tmin, const dvec3& rayDirNormalized, double tmax, const dvec3& sphereCenter, double sphereRadius)
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

// Compute the intersection of a ray and a plane with infinite bounds. The ray direction 
// must be normalized. Returns the hit distance, from the ray's origin; to get the hit 
// position, multiply that by the ray's direction and then add the ray's origin.
bool TraceTester::RayPlaneTrace(const dvec3& rayOrigin, const dvec3& rayDirNormalized, const dvec3& planeOrigin, const dvec3& planeNormal, double& t)
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
bool TraceTester::RayCircleTrace(const dvec3& rayOrigin, const dvec3& rayDirNormalized, const dvec3& circleCenter, const dvec3& circleNormal, double radius, double& t)
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
