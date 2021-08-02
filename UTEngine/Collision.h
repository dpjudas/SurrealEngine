#pragma once

#include "Math/vec.h"

class BspNode;

class CylinderShape
{
public:
	CylinderShape() = default;
	CylinderShape(const vec3& center, float height, float radius) : Center(center), Height(height), Radius(radius) { }

	vec3 Center = vec3(0.0f);
	float Height = 0.0f;
	float Radius = 0.0f;
};

class SweepHit
{
public:
	float Fraction = 1.0f;
	vec3 Normal = vec3(0.0f);
};

class Collision
{
public:
	bool TraceAnyHit(vec3 from, vec3 to);
	SweepHit Sweep(CylinderShape* shape, const vec3& to);
	
private:
	bool TraceAnyHit(const vec4& from, const vec4& to, BspNode* node, BspNode* nodes);
	SweepHit Sweep(const vec4& from, const vec4& to, float radius, BspNode* node, BspNode* nodes);

	float NodeRayIntersect(const vec4& from, const vec4& to, BspNode* node);
	float NodeSphereIntersect(const vec4& from, const vec4& to, float radius, BspNode* node);

	float TriangleRayIntersect(const vec4& from, const vec4& to, const vec3* points);
	float TriangleSphereIntersect(const vec4& from, const vec4& to, float radius, const vec3* points);
};
