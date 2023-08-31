
#include "Precomp.h"
#include "FrustumPlanes.h"
#include "bbox.h"

FrustumPlanes::FrustumPlanes(const mat4& world_to_projection)
{
	planes[0] = nearFrustumPlane(world_to_projection);
	planes[1] = farFrustumPlane(world_to_projection);
	planes[2] = leftFrustumPlane(world_to_projection);
	planes[3] = rightFrustumPlane(world_to_projection);
	planes[4] = topFrustumPlane(world_to_projection);
	planes[5] = bottomFrustumPlane(world_to_projection);
}

IntersectionTestResult FrustumPlanes::test(const BBox& box) const
{
	bool is_intersecting = false;
	for (int i = 0; i < 6; i++)
	{
		IntersectionTestResult result = planeAABB(planes[i], box);
		if (result == IntersectionTestResult::outside)
		{
			return IntersectionTestResult::outside;
		}
		else if (result == IntersectionTestResult::intersecting)
		{
			is_intersecting = true;
		}
	}
	if (is_intersecting)
		return IntersectionTestResult::intersecting;
	else
		return IntersectionTestResult::inside;
}

int FrustumPlanes::testIntersecting(const BBox& box) const
{
	int bits = 0;
	for (int i = 0; i < 6; i++)
	{
		IntersectionTestResult result = planeAABB(planes[i], box);
		if (result == IntersectionTestResult::outside)
		{
			return -1;
		}
		else if (result == IntersectionTestResult::intersecting)
		{
			bits |= (1 << i);
		}
	}
	return bits;
}

IntersectionTestResult FrustumPlanes::planeAABB(const vec4& plane, const BBox& aabb)
{
	vec3 center = aabb.center();
	vec3 extents = aabb.extents();
	float e = extents.x * std::abs(plane.x) + extents.y * std::abs(plane.y) + extents.z * std::abs(plane.z);
	float s = center.x * plane.x + center.y * plane.y + center.z * plane.z + plane.w;
	if (s - e > 0)
		return IntersectionTestResult::inside;
	else if (s + e < 0)
		return IntersectionTestResult::outside;
	else
		return IntersectionTestResult::intersecting;
}

vec4 FrustumPlanes::leftFrustumPlane(const mat4& matrix)
{
	vec4 plane(
		matrix[3 + 0 * 4] + matrix[0 + 0 * 4],
		matrix[3 + 1 * 4] + matrix[0 + 1 * 4],
		matrix[3 + 2 * 4] + matrix[0 + 2 * 4],
		matrix[3 + 3 * 4] + matrix[0 + 3 * 4]);
	plane /= length(plane.xyz());
	return plane;
}

vec4 FrustumPlanes::rightFrustumPlane(const mat4& matrix)
{
	vec4 plane(
		matrix[3 + 0 * 4] - matrix[0 + 0 * 4],
		matrix[3 + 1 * 4] - matrix[0 + 1 * 4],
		matrix[3 + 2 * 4] - matrix[0 + 2 * 4],
		matrix[3 + 3 * 4] - matrix[0 + 3 * 4]);
	plane /= length(plane.xyz());
	return plane;
}

vec4 FrustumPlanes::topFrustumPlane(const mat4& matrix)
{
	vec4 plane(
		matrix[3 + 0 * 4] - matrix[1 + 0 * 4],
		matrix[3 + 1 * 4] - matrix[1 + 1 * 4],
		matrix[3 + 2 * 4] - matrix[1 + 2 * 4],
		matrix[3 + 3 * 4] - matrix[1 + 3 * 4]);
	plane /= length(plane.xyz());
	return plane;
}

vec4 FrustumPlanes::bottomFrustumPlane(const mat4& matrix)
{
	vec4 plane(
		matrix[3 + 0 * 4] + matrix[1 + 0 * 4],
		matrix[3 + 1 * 4] + matrix[1 + 1 * 4],
		matrix[3 + 2 * 4] + matrix[1 + 2 * 4],
		matrix[3 + 3 * 4] + matrix[1 + 3 * 4]);
	plane /= length(plane.xyz());
	return plane;
}

vec4 FrustumPlanes::nearFrustumPlane(const mat4& matrix)
{
	vec4 plane(
		matrix[3 + 0 * 4] + matrix[2 + 0 * 4],
		matrix[3 + 1 * 4] + matrix[2 + 1 * 4],
		matrix[3 + 2 * 4] + matrix[2 + 2 * 4],
		matrix[3 + 3 * 4] + matrix[2 + 3 * 4]);
	plane /= length(plane.xyz());
	return plane;
}

vec4 FrustumPlanes::farFrustumPlane(const mat4& matrix)
{
	vec4 plane(
		matrix[3 + 0 * 4] - matrix[2 + 0 * 4],
		matrix[3 + 1 * 4] - matrix[2 + 1 * 4],
		matrix[3 + 2 * 4] - matrix[2 + 2 * 4],
		matrix[3 + 3 * 4] - matrix[2 + 3 * 4]);
	plane /= length(plane.xyz());
	return plane;
}
