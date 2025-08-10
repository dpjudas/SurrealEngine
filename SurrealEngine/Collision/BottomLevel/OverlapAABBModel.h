#pragma once

#include "UObject/ULevel.h"

class BBox;

class OverlapAABBModel
{
public:
	CollisionHitList TestOverlap(UModel* model, const vec3& center, const vec3& extents, bool visibilityOnly);

private:
	void TestOverlap(const dvec3& center, const dvec3& extents, bool visibilityOnly, BspNode* node, CollisionHitList& hits);
	int NodeAABBOverlap(const dvec3& center, const dvec3& extents, BspNode* node);
	static int PlaneAABBOverlap(const dvec3& center, const dvec3& extents, const dvec4& plane);
	static bool AABBOverlap(const BBox& bboxA, const BBox& bboxB);

	UModel* Model = nullptr;
};
