#pragma once

#include "UObject/ULevel.h"

class TraceRayModel
{
public:
	std::vector<TraceHit> Trace(UModel* model, const dvec3& origin, double tmin, const dvec3& dirNormalized, double tmax, bool visibilityOnly);
	bool TraceAnyHit(UModel* model, const dvec3& origin, double tmin, const dvec3& dirNormalized, double tmax, bool visibilityOnly);

private:
	void Trace(const dvec3& origin, double tmin, const dvec3& dirNormalized, double tmax, bool visibilityOnly, BspNode* node, std::vector<TraceHit>& hits);
	bool TraceAnyHit(const dvec3& origin, double tmin, const dvec3& dirNormalized, double tmax, bool visibilityOnly, BspNode* node);

	double NodeRayIntersect(const dvec3& origin, double tmin, const dvec3& dirNormalized, double tmax, BspNode* node);
	double TriangleRayIntersect(const dvec3& origin, const dvec3& dirNormalized, double tmax, const dvec3* points);

	UModel* Model = nullptr;
};
