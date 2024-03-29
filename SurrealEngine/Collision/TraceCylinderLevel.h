#pragma once

#include "UObject/ULevel.h"

class TraceCylinderLevel
{
public:
	CollisionHitList Trace(ULevel* level, const vec3& from, const vec3& to, float height, float radius, bool traceActors, bool traceWorld, bool visibilityOnly);

private:
	ULevel* Level = nullptr;
};
