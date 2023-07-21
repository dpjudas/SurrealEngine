#pragma once

#include "vec.h"

vec3 hsbtorgb(double hue, double saturation, double brightness);

inline vec3 hsbtorgb(uint8_t hue, uint8_t saturation, uint8_t brightness)
{
	return hsbtorgb(hue * (360.0 / 255.0), (255 - saturation) * (1.0 / 255.0), brightness * (1.0 / 255.0));
}
