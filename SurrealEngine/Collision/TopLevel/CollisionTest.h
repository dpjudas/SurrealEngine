#pragma once

#include "CollisionSystem.h"
#include "UObject/ULevel.h"

class CollisionTester
{
public:
	CollisionTester(CollisionSystem* collision) : Collision(collision) {}

	static ivec3 GetStartExtents(const vec3& location, const vec3& extents) { return CollisionSystem::GetStartExtents(location, extents); }
	static ivec3 GetRayStartExtents(const vec3& from, const vec3& to) { return CollisionSystem::GetRayStartExtents(from, to); }
	static ivec3 GetSweepStartExtents(const vec3& from, const vec3& to, const vec3& extents) { return CollisionSystem::GetSweepStartExtents(from, to, extents); }

	static ivec3 GetEndExtents(const vec3& location, const vec3& extents) { return CollisionSystem::GetEndExtents(location, extents); }
	static ivec3 GetRayEndExtents(const vec3& from, const vec3& to) { return CollisionSystem::GetRayEndExtents(from, to); }
	static ivec3 GetSweepEndExtents(const vec3& from, const vec3& to, const vec3& extents) { return CollisionSystem::GetSweepEndExtents(from, to, extents); }

	const std::list<UActor*>& GetActors(int x, int y, int z) const
	{
		auto it = Collision->CollisionActors.find(CollisionSystem::GetBucketId(x, y, z));
		if (it != Collision->CollisionActors.end())
			return it->second;
		else
			return emptyList;
	}

	ULevel* GetLevel() const { return Collision->Level; }

private:
	CollisionSystem* Collision = nullptr;
	inline static std::list<UActor*> emptyList;
};
