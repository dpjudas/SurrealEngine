#pragma once

#include <cmath>
#include "Math/vec.h"

class UActor;

class LightEffect
{
public:
	void Run(UActor* light, int width, int height, const vec3* locations, vec3 base, vec3 normal, const float* shadowmap, float* result);
};
