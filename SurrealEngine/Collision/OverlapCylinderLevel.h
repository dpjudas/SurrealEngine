#pragma once

#include "UObject/ULevel.h"

class OverlapCylinderLevel
{
public:
	CollisionHitList TestOverlap(ULevel* level, const vec3& location, float height, float radius, bool traceActors, bool traceWorld, bool visibilityOnly);

private:
	ULevel* Level = nullptr;
};
