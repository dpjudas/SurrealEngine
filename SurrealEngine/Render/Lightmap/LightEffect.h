#pragma once

#include <cmath>
#include "Math/vec.h"

class UActor;

class LightEffect
{
public:
	void Run(UActor* light, int width, int height, const vec3* locations, vec3 normal, const float* shadowmap, float* result);

	static float VertexLight(UActor* light, const vec3& location, const vec3& normal);

	static float LightDistanceFalloff(float dist)
	{
		float v = std::sqrt(dist + (1.0f / 4096.0f));
		float v2 = v * v;
		float v3 = v2 * v;
		return (1.0f + 2.0f * v3 - 3.0f * v2) / v; // To do: is this the correct falloff?
	}
};
