#pragma once

#include "Packages/Engine/Resources/Level/ULevel.h"

class TraceRayModel
{
public:
	CollisionHitList Trace(UActor* owner, UModel* model, const dvec3& origin, double tmin, const dvec3& dirNormalized, double tmax, bool visibilityOnly);
	bool TraceAnyHit(UActor* owner, UModel* model, const dvec3& origin, double tmin, const dvec3& dirNormalized, double tmax, bool visibilityOnly);

private:
	void Trace(const dvec3& origin, double tmin, const dvec3& dirNormalized, double tmax, bool visibilityOnly, BspNode* node, CollisionHitList& hits);
	bool TraceAnyHit(const dvec3& origin, double tmin, const dvec3& dirNormalized, double tmax, bool visibilityOnly, BspNode* node);

	double NodeRayIntersect(const dvec3& origin, double tmin, const dvec3& dirNormalized, double tmax, BspNode* node);
	double TriangleRayIntersect(const dvec3& origin, const dvec3& dirNormalized, double tmax, const dvec3* points);

	UActor* Owner = nullptr;
	UModel* Model = nullptr;
};
