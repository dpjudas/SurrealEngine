#pragma once

#include "Math/vec.h"
#include <unordered_map>
#include <list>

class ULevel;
class UActor;
class CollisionHitList;
class CollisionHit;

struct TraceFlags
{
	bool pawns = false;
	bool movers = false;
	bool others = false;
	bool world = false;
	bool zoneChanges = false;
	bool onlyProjectiles = false;

	bool traceActors() const { return pawns || movers || others || zoneChanges || onlyProjectiles; }
	bool traceWorld() const { return world; }
};

class CollisionSystem
{
public:
	// To do: Clean this up. Too many functions doing kinda the same with subtle differences.

	bool IsOverlapping(UActor* actor1, UActor* actor2);
	CollisionHitList OverlapTest(UActor* actor);
	CollisionHitList OverlapTest(ULevel* level, const vec3& location, float height, float radius, bool testActors, bool testWorld, bool visibilityOnly);

	void TraceTest(UActor* actor, const dvec3& origin, double tmin, const dvec3& dirNormalized, double tmax, double height, double radius, CollisionHitList& hits);
	bool TraceAnyHit(vec3 from, vec3 to, UActor* tracingActor, bool traceActors, bool traceWorld, bool visibilityOnly);
	CollisionHitList Trace(const vec3& from, const vec3& to, float height, float radius, bool traceActors, bool traceWorld, bool visibilityOnly);
	CollisionHitList TraceDecal(const dvec3& origin, double tmin, const dvec3& dirNormalized, double tmax, bool visibilityOnly);
	CollisionHit TraceFirstHit(const vec3& from, const vec3& to, UActor* tracingActor, const vec3& extents, const TraceFlags& flags);

	Array<UActor*> CollidingActors(const vec3& origin, float radius);
	Array<UActor*> CollidingActors(const vec3& origin, float height, float radius);
	Array<UActor*> EncroachingActors(UActor* actor);

	void SetLevel(ULevel* level);
	void AddToCollision(UActor* actor);
	void RemoveFromCollision(UActor* actor);

private: // Important: this is private for a reason. Do not break the separation of concerns by making UObject related stuff peak directly into this!
	friend class OverlapCylinderLevel;
	friend class TraceRayLevel;
	friend class TraceCylinderLevel;
	friend class CollisionCommandlet;

	static ivec3 GetStartExtents(const vec3& location, const vec3& extents)
	{
		int xx = (int)std::floor((location.x - extents.x) * (1.0f / 256.0f));
		int yy = (int)std::floor((location.y - extents.y) * (1.0f / 256.0f));
		int zz = (int)std::floor((location.z - extents.z) * (1.0f / 256.0f));
		return { xx, yy, zz };
	}

	static ivec3 GetRayStartExtents(const vec3& from, const vec3& to)
	{
		float x = std::min(from.x, to.x);
		float y = std::min(from.y, to.y);
		float z = std::min(from.z, to.z);
		int xx = (int)std::floor(x * (1.0f / 256.0f));
		int yy = (int)std::floor(y * (1.0f / 256.0f));
		int zz = (int)std::floor(z * (1.0f / 256.0f));
		return { xx, yy, zz };
	}

	static ivec3 GetSweepStartExtents(const vec3& from, const vec3& to, const vec3& extents)
	{
		float x = std::min(from.x, to.x) - extents.x;
		float y = std::min(from.y, to.y) - extents.y;
		float z = std::min(from.z, to.z) - extents.z;
		int xx = (int)std::floor(x * (1.0f / 256.0f));
		int yy = (int)std::floor(y * (1.0f / 256.0f));
		int zz = (int)std::floor(z * (1.0f / 256.0f));
		return { xx, yy, zz };
	}

	static ivec3 GetEndExtents(const vec3& location, const vec3& extents)
	{
		int xx = (int)std::floor((location.x + extents.x) * (1.0f / 256.0f)) + 1;
		int yy = (int)std::floor((location.y + extents.y) * (1.0f / 256.0f)) + 1;
		int zz = (int)std::floor((location.z + extents.z) * (1.0f / 256.0f)) + 1;
		return { xx, yy, zz };
	}

	static ivec3 GetRayEndExtents(const vec3& from, const vec3& to)
	{
		float x = std::max(from.x, to.x);
		float y = std::max(from.y, to.y);
		float z = std::max(from.z, to.z);
		int xx = (int)std::floor(x * (1.0f / 256.0f)) + 1;
		int yy = (int)std::floor(y * (1.0f / 256.0f)) + 1;
		int zz = (int)std::floor(z * (1.0f / 256.0f)) + 1;
		return { xx, yy, zz };
	}

	static ivec3 GetSweepEndExtents(const vec3& from, const vec3& to, const vec3& extents)
	{
		float x = std::max(from.x, to.x) + extents.x;
		float y = std::max(from.y, to.y) + extents.y;
		float z = std::max(from.z, to.z) + extents.z;
		int xx = (int)std::floor(x * (1.0f / 256.0f)) + 1;
		int yy = (int)std::floor(y * (1.0f / 256.0f)) + 1;
		int zz = (int)std::floor(z * (1.0f / 256.0f)) + 1;
		return { xx, yy, zz };
	}

	static uint32_t GetBucketId(int x, int y, int z)
	{
		return ((x & 0x3ff) << 20) | ((y & 0x3ff) << 10) | (z & 0x3ff);
	}

	// Ray/actor hit trace
	static double RayActorTrace(const dvec3& origin, double tmin, const dvec3& dirNormalized, double tmax, UActor* actor);

	// Cylinder/actor hit trace
	static double CylinderActorTrace(const dvec3& origin, double tmin, const dvec3& dirNormalized, double tmax, double cylinderHeight, double cylinderRadius, UActor* actor);

	// Sphere/actor overlap test
	static bool SphereActorOverlap(const dvec3& origin, double sphereRadius, UActor* actor);

	// Cylinder/actor overlap test
	static bool CylinderActorOverlap(const dvec3& origin, double cylinderHeight, double cylinderRadius, UActor* actor);

	// Ray/sphere hit trace
	static double RaySphereTrace(const dvec3& rayOrigin, double tmin, const dvec3& rayDirNormalized, double tmax, const dvec3& sphereCenter, double sphereRadius);

	// Ray/cylinder hit trace
	static double RayCylinderTrace(const dvec3& rayOrigin, const dvec3& rayDirNormalized, double tmin, double tmax, const dvec3& cylinderCenter, double cylinderHeight, double cylinderRadius);

	// Ray/circle hit trace
	static bool RayCircleTrace(const dvec3& rayOrigin, const dvec3& rayDirNormalized, const dvec3& circleCenter, const dvec3& circleNormal, double radius, double& t);

	// Ray/plane hit trace
	static bool RayPlaneTrace(const dvec3& rayOrigin, const dvec3& rayDirNormalized, const dvec3& planeOrigin, const dvec3& planeNormal, double& t);

	// Cylinder/cylinder hit trace
	static double CylinderCylinderTrace(const dvec3& origin, const dvec3& dirNormalized, double tmin, double tmax, const dvec3& cylinderCenterA, double cylinderHeightA, double cylinderRadiusA, double cylinderHeightB, double cylinderRadiusB);

	// Sphere/capsule overlap test
	static bool SphereCapsuleOverlap(const dvec3& sphereCenter, double sphereRadius, const dvec3& capsuleCenter, double capsuleHeight, double capsuleRadius);

	// Cylinder/cylinder overlap test
	static bool CylinderCylinderOverlap(const dvec3& cylinderCenterA, double cylinderHeightA, double cylinderRadiusA, const dvec3& cylinderCenterB, double cylinderHeightB, double cylinderRadiusB);

	ULevel* Level = nullptr;
	std::unordered_map<uint32_t, std::list<UActor*>> CollisionActors;
};
