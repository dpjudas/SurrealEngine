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

class BBox
{
public:
	BBox() = default;
	BBox(const vec3& aabb_min, const vec3& aabb_max) : min(aabb_min), max(aabb_max) { }

	vec3 center() const
	{
		auto halfmin = min * 0.5f;
		auto halfmax = max * 0.5f;
		return halfmax + halfmin;
	}

	vec3 extents() const
	{
		auto halfmin = min * 0.5f;
		auto halfmax = max * 0.5f;
		return halfmax - halfmin;
	}

	vec3 min;
	vec3 max;

	bool IsValid = true;
};
