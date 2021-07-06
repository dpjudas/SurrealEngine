
#include "Precomp.h"
#include "Collision.h"
#include "UObject/ULevel.h"
#include "Engine.h"

bool Collision::TraceAnyHit(vec3 from, vec3 to)
{
	return TraceAnyHit(vec4(from, 1.0f), vec4(to, 1.0f), &engine->level->Model->Nodes.front(), engine->level->Model->Nodes.data());
}

bool Collision::TraceAnyHit(const vec4& from, const vec4& to, BspNode* node, BspNode* nodes)
{
	BspNode* polynode = node;
	while (true)
	{
		if (HitTestNodePolygon(from, to, polynode))
			return true;

		if (polynode->Plane < 0) break;
		polynode = nodes + polynode->Plane;
	}

	vec4 plane = { node->PlaneX, node->PlaneY, node->PlaneZ, -node->PlaneW };
	float fromSide = dot(from, plane);
	float toSide = dot(to, plane);

	if (node->Front >= 0 && (fromSide >= 0.0f || toSide >= 0.0f) && TraceAnyHit(from, to, nodes + node->Front, nodes))
		return true;
	else if (node->Back >= 0 && (fromSide <= 0.0f || toSide <= 0.0f) && TraceAnyHit(from, to, nodes + node->Back, nodes))
		return true;
	else
		return false;
}

bool Collision::HitTestNodePolygon(const vec4& from, const vec4& to, BspNode* node)
{
	if (node->NumVertices == 0 || (node->NodeFlags & NF_NotVisBlocking) || (node->Surf >= 0 && engine->level->Model->Surfaces[node->Surf].PolyFlags & PF_NotSolid))
		return false;

	vec4 plane = { node->PlaneX, node->PlaneY, node->PlaneZ, -node->PlaneW };

	// Test if plane is actually crossed
	float fromSide = dot(from, plane);
	float toSide = dot(to, plane);
	if ((fromSide > 0.0f && toSide > 0.0f) || (fromSide < 0.0f && toSide < 0.0f))
		return false;

	// Find hit on plane
	vec3 rayvec = to.xyz() - from.xyz();
	float t = -(dot(from, plane) / dot(rayvec, plane.xyz()));
	vec3 planehit = from.xyz() + t * rayvec;

	// Project hit onto the plane:
	vec3 xaxis, yaxis;
	vec3 zaxis = plane.xyz();
	vec3 absN = { std::abs(plane.x), std::abs(plane.y), std::abs(plane.z) };
	if (absN.x > absN.z && absN.x > absN.y)
		xaxis = cross(zaxis, { 0.0f, 0.0f, 1.0f });
	else if (absN.y > absN.z && absN.y > absN.x)
		xaxis = cross(zaxis, { 1.0f, 0.0f, 0.0f });
	else
		xaxis = cross(zaxis, { 0.0f, 1.0f, 0.0f });
	yaxis = cross(zaxis, xaxis);

	vec2 planehit2d = { dot(xaxis, planehit), dot(yaxis, planehit) };

	// Edge test all lines in the convex polygon to determine if the point is inside
	BspVert* v = &engine->level->Model->Vertices[node->VertPool];
	vec3* points = engine->level->Model->Points.data();
	int count = node->NumVertices;
	int insidecount = 0;
	for (int i = 0; i < count; i++)
	{
		int j = (i + 1) % count;
		vec3 p0 = points[v[i].Vertex];
		vec3 p1 = points[v[j].Vertex];
		vec2 edgepoint0 = { dot(xaxis, p0), dot(yaxis, p0) };
		vec2 edgepoint1 = { dot(xaxis, p1), dot(yaxis, p1) };
		vec2 edgevec = edgepoint1 - edgepoint0;
		vec2 edgeN = { -edgevec.y, edgevec.x };
		vec3 edgeplane = { edgeN, -dot(edgeN, edgepoint0) };
		float side = dot(edgeplane, { planehit2d, 1.0f });
		if (side <= 0.0f)
			insidecount++;
	}

	return insidecount == count || insidecount == 0;
}
