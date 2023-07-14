#pragma once

#include "UObject/ULevel.h"

class TraceSphereModel
{
public:
	std::vector<SweepHit> Trace(UModel* model, const dvec3& origin, double tmin, const dvec3& dirNormalized, double tmax, double radius, bool visibilityOnly);

private:
	void Trace(const dvec3& origin, double tmin, const dvec3& dirNormalized, double tmax, double radius, bool visibilityOnly, BspNode* node, std::vector<SweepHit>& hits);

	double NodeSphereIntersect(const dvec3& origin, double tmin, const dvec3& dirNormalized, double tmax, double radius, BspNode* node);
	double TriangleSphereIntersect(const dvec3& from, const dvec3& to, double radius, const dvec3* points);

	UModel* Model = nullptr;
};
