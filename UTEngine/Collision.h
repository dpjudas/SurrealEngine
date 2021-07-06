#pragma once

#include "Math/vec.h"

class BspNode;

class Collision
{
public:
	bool TraceAnyHit(vec3 from, vec3 to);
	bool TraceAnyHit(const vec4& from, const vec4& to, BspNode* node, BspNode* nodes);
	bool HitTestNodePolygon(const vec4& from, const vec4& to, BspNode* node);
};
