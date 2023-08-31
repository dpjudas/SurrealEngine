#pragma once

#include "Math/vec.h"
#include "Math/FrustumPlanes.h"

class BBox;

class ClipSpan
{
public:
	int x0;
	int x1;
};

class BspClipper
{
public:
	BspClipper();
	~BspClipper();

	void Setup(const mat4& world_to_projection);

	bool CheckSurface(const vec3* vertices, uint32_t count, bool solid);
	bool IsAABBVisible(const BBox& bbox);

private:
	bool IsVisible(int y, int x0, int x1);

	bool DrawTriangle(const vec4* const* vert, bool solid, bool ccw);
	int ClipEdge(const vec4* const* verts);

	bool DrawClippedTriangle(const vec4* const* vertices, bool solid);
	bool DrawSpan(int y, int x0, int x1, bool solid);

	static bool IsDegenerate(const vec4* const* vert);
	static bool IsFrontfacing(const vec4* const* vert);
	static void SortVertices(const vec4* const* vertices, const vec4** sortedVertices);

	std::vector<std::vector<ClipSpan>> Viewport;
	FrustumPlanes FrustumClip;
	mat4 WorldToProjection;

	enum { max_additional_vertices = 16 };
	float weightsbuffer[max_additional_vertices * 3 * 2];
	float* weights = nullptr;

	const int ViewportWidth = 1024;
	const int ViewportHeight = 1024;
};
