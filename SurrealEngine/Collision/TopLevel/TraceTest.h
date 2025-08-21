#pragma once

#include "CollisionTest.h"

class TraceTester : public CollisionTester
{
public:
	using CollisionTester::CollisionTester;

	CollisionHitList Trace(const vec3& from, const vec3& to, float height, float radius, bool traceActors, bool traceWorld, bool visibilityOnly);
	bool TraceAnyHit(vec3 from, vec3 to, UActor* tracingActor, bool traceActors, bool traceWorld, bool visibilityOnly);

private:
	void TraceActor(UActor* actor, const dvec3& origin, double tmin, const dvec3& dirNormalized, double tmax, double height, double radius, CollisionHitList& hits);

	// Ray/actor hit trace
	static double RayActorTrace(const dvec3& origin, double tmin, const dvec3& dirNormalized, double tmax, UActor* actor);

	// Cylinder/actor hit trace
	static double CylinderActorTrace(const dvec3& origin, double tmin, const dvec3& dirNormalized, double tmax, double cylinderHeight, double cylinderRadius, UActor* actor);

	// Cylinder/cylinder hit trace
	static double CylinderCylinderTrace(const dvec3& origin, const dvec3& dirNormalized, double tmin, double tmax, const dvec3& cylinderCenterA, double cylinderHeightA, double cylinderRadiusA, double cylinderHeightB, double cylinderRadiusB);

	// Ray/sphere hit trace
	static double RaySphereTrace(const dvec3& rayOrigin, double tmin, const dvec3& rayDirNormalized, double tmax, const dvec3& sphereCenter, double sphereRadius);

	// Ray/cylinder hit trace
	static double RayCylinderTrace(const dvec3& rayOrigin, const dvec3& rayDirNormalized, double tmin, double tmax, const dvec3& cylinderCenter, double cylinderHeight, double cylinderRadius);

	// Ray/circle hit trace
	static bool RayCircleTrace(const dvec3& rayOrigin, const dvec3& rayDirNormalized, const dvec3& circleCenter, const dvec3& circleNormal, double radius, double& t);

	// Ray/plane hit trace
	static bool RayPlaneTrace(const dvec3& rayOrigin, const dvec3& rayDirNormalized, const dvec3& planeOrigin, const dvec3& planeNormal, double& t);

	friend class CollisionCommandlet;
};
