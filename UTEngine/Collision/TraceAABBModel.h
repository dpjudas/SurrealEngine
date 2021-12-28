#pragma once

#include "UObject/ULevel.h"

class TraceAABBModel
{
public:
	std::vector<SweepHit> Trace(UModel* model, const dvec3& origin, double tmin, const dvec3& dirNormalized, double tmax, const dvec3& extents, bool visibilityOnly);

private:
	void Trace(const dvec3& origin, double tmin, const dvec3& dirNormalized, double tmax, const dvec3& extents, bool visibilityOnly, BspNode* node, std::vector<SweepHit>& hits);

	static double HullNodeAABBIntersect(const dvec3& origin, double tmin, const dvec3& dirNormalized, double tmax, const dvec3& extents, BspNode* node, bool flipPlane);
	static int NodeAABBOverlap(const dvec3& center, const dvec3& extents, BspNode* node);

	UModel* Model = nullptr;
};
