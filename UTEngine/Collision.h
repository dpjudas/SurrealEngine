#pragma once

#include "Math/vec.h"

class BspNode;

class CylinderShape
{
public:
	CylinderShape() = default;
	CylinderShape(const vec3& center, float height, float radius) : Center(center.x, center.y, center.z), Height(height), Radius(radius) { }

	dvec3 Center = dvec3(0.0);
	double Height = 0.0;
	double Radius = 0.0;
};

class SweepHit
{
public:
	float Fraction = 1.0;
	vec3 Normal = vec3(0.0);
};

class Collision
{
public:
	bool TraceAnyHit(vec3 from, vec3 to);
	SweepHit Sweep(CylinderShape* shape, const vec3& to);
	
private:
	bool TraceAnyHit(const dvec4& from, const dvec4& to, BspNode* node, BspNode* nodes);
	SweepHit Sweep(const dvec4& from, const dvec4& to, double radius, BspNode* node, BspNode* nodes);

	double NodeRayIntersect(const dvec4& from, const dvec4& to, BspNode* node);
	double NodeSphereIntersect(const dvec4& from, const dvec4& to, double radius, BspNode* node);

	double TriangleRayIntersect(const dvec4& from, const dvec4& to, const dvec3* points);
	double TriangleSphereIntersect(const dvec4& from, const dvec4& to, double radius, const dvec3* points);
};
