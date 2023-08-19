#pragma once

#include "vec.h"

vec3 hsbtorgb(double hue, double saturation, double brightness);

inline vec3 hsbtorgb(uint8_t hue, uint8_t saturation, uint8_t brightness)
{
	double unrealConversion = 1.0 / 255.0;
	double unrealHue = hue * 360 * unrealConversion;
	double unrealSaturation = (255 - saturation) * (unrealConversion);
	double unrealBrightness = (6.512735 * std::sqrt(brightness)) * (unrealConversion);

	return hsbtorgb(unrealHue, unrealSaturation, unrealBrightness);
}
