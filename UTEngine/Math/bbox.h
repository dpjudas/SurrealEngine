#pragma once

#include "vec.h"

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

	bool intersects(const BBox& box) const
	{
		if (max.x < box.min.x || min.x > box.max.x) return false;
		if (max.y < box.min.y || min.y > box.max.y) return false;
		if (max.z < box.min.z || min.z > box.max.z) return false;

		return true;
	}

	vec3 min;
	vec3 max;

	bool IsValid = true;
};
