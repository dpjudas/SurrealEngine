#pragma once

#include "Math/vec.h"
#include "Math/mat.h"

class BBox;

enum class IntersectionTestResult
{
	outside,
	inside,
	intersecting,
};

class FrustumPlanes
{
public:
	FrustumPlanes() = default;
	FrustumPlanes(const mat4& world_to_projection);

	IntersectionTestResult test(const BBox& box) const;

	vec4 planes[6];

private:
	static IntersectionTestResult planeAABB(const vec4& plane, const BBox& aabb);

	static vec4 leftFrustumPlane(const mat4& matrix);
	static vec4 rightFrustumPlane(const mat4& matrix);
	static vec4 topFrustumPlane(const mat4& matrix);
	static vec4 bottomFrustumPlane(const mat4& matrix);
	static vec4 nearFrustumPlane(const mat4& matrix);
	static vec4 farFrustumPlane(const mat4& matrix);
};
