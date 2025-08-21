#pragma once

#include "CollisionTest.h"

class OverlapTester : public CollisionTester
{
public:
	using CollisionTester::CollisionTester;

	CollisionHitList TestOverlap(const vec3& location, float height, float radius, bool testActors, bool testWorld, bool visibilityOnly);

	bool IsOverlapping(UActor* actor1, UActor* actor2);

	Array<UActor*> CollidingActors(const vec3& origin, float radius);
	Array<UActor*> CollidingActors(const vec3& origin, float height, float radius);
	Array<UActor*> EncroachingActors(UActor* actor);

private:
	// Sphere/actor overlap test
	static bool SphereActorOverlap(const dvec3& origin, double sphereRadius, UActor* actor);

	// Cylinder/actor overlap test
	static bool CylinderActorOverlap(const dvec3& origin, double cylinderHeight, double cylinderRadius, UActor* actor);

	// Sphere/capsule overlap test
	static bool SphereCapsuleOverlap(const dvec3& sphereCenter, double sphereRadius, const dvec3& capsuleCenter, double capsuleHeight, double capsuleRadius);

	// Cylinder/cylinder overlap test
	static bool CylinderCylinderOverlap(const dvec3& cylinderCenterA, double cylinderHeightA, double cylinderRadiusA, const dvec3& cylinderCenterB, double cylinderHeightB, double cylinderRadiusB);
};
