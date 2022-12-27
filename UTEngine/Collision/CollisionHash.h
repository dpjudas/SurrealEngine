#pragma once

#include "Math/vec.h"
#include <unordered_map>
#include <list>

class UActor;

class CollisionHash
{
public:
	std::unordered_map<uint32_t, std::list<UActor*>> CollisionActors;

	void AddToCollision(UActor* actor);
	void RemoveFromCollision(UActor* actor);

	std::vector<UActor*> CollidingActors(const vec3& origin, float radius);

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

	bool ActorSphereCollision(const dvec3& origin, double sphereRadius, UActor* actor);
	double ActorRayIntersect(const dvec3& origin, double tmin, const dvec3& dirNormalized, double tmax, UActor* actor);
	double ActorSphereIntersect(const dvec3& origin, double tmin, const dvec3& dirNormalized, double tmax, double sphereRadius, UActor* actor);
	double ActorCylinderIntersect(const vec3& from, const vec3& to, const dvec3& dirNormalized, double tmin, double tmax, UActor* actor);
	double RaySphereIntersect(const dvec3& rayOrigin, double tmin, const dvec3& rayDirNormalized, double tmax, const dvec3& sphereCenter, double sphereRadius);
	double RayCylinderIntersect(const dvec3& start, const dvec3& end, const dvec3& rayDirNormalized, double tmin, double tmax, const dvec3& cylinderCenter, double cylinderHeight, double cylinderRadius);
};
