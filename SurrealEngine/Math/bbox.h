#pragma once

#include "vec.h"
#include "mat.h"

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

	BBox transform(const mat4& objectToWorld) const
	{
		vec4 verts[8] =
		{
			objectToWorld * vec4(min.x, min.y, min.z, 1.0f),
			objectToWorld * vec4(max.x, min.y, min.z, 1.0f),
			objectToWorld * vec4(min.x, max.y, min.z, 1.0f),
			objectToWorld * vec4(max.x, max.y, min.z, 1.0f),
			objectToWorld * vec4(min.x, min.y, max.z, 1.0f),
			objectToWorld * vec4(max.x, min.y, max.z, 1.0f),
			objectToWorld * vec4(min.x, max.y, max.z, 1.0f),
			objectToWorld * vec4(max.x, max.y, max.z, 1.0f)
		};

		BBox result;
		result.min = verts[0].xyz();
		result.max = verts[0].xyz();
		for (int i = 1; i < 8; i++)
		{
			result.min.x = std::min(result.min.x, verts[i].x);
			result.min.y = std::min(result.min.y, verts[i].y);
			result.min.z = std::min(result.min.z, verts[i].z);
			result.max.x = std::max(result.max.x, verts[i].x);
			result.max.y = std::max(result.max.y, verts[i].y);
			result.max.z = std::max(result.max.z, verts[i].z);
		}
		return result;
	}

	vec3 min;
	vec3 max;

	bool IsValid = true;
};

inline bool operator==(const BBox& a, const BBox& b) { return a.min == b.min && a.max == b.max && a.IsValid == b.IsValid; }
inline bool operator!=(const BBox& a, const BBox& b) { return a.min != b.min || a.max != b.max || a.IsValid != b.IsValid; }
