#pragma once

#include "UObject/ULevel.h"

class OverlapAABBModel
{
public:
	CollisionHitList TestOverlap(UModel* model, const vec3& location, const vec3& extents, bool visibilityOnly);

private:
	UModel* Model = nullptr;
};
