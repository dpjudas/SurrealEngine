#pragma once

#include "vec.h"

extern float hsbtorgb_v_table[256];

inline vec3 hsbtorgb(uint8_t hue, uint8_t saturation, uint8_t brightness)
{
	if (saturation >= 250)
	{
		return vec3(hsbtorgb_v_table[brightness] * (1.0f / 255.0f)); // vec3(6.512735f * std::sqrt((float)brightness / 255.0f));
	}
	else if (brightness > 0)
	{ 
		float v = hsbtorgb_v_table[brightness]; // 6.512735f * std::sqrt((float)brightness);

		float s = saturation * (1.0f / 2.5f);
		if (s > 32.0f)
			s += 2.0f;

		float sectorPos = hue * (1.0f / 85.0f);
		int sectorNum = (int)sectorPos;
		float sectorFrac = sectorPos - sectorNum;

		float p = s * v * (1.0f / 104.0f);
		float q = ((1.0f - sectorFrac) * v) + (p * sectorFrac);
		float t = (sectorFrac * v) + (p * (1.0f - sectorFrac));

		if (hue < 85)
		{
			return vec3(q * (1.0f / 255.0f), t * (1.0f / 255.0f), p * (1.0f / 255.0f));
		}
		else if (hue < 2 * 85)
		{
			return vec3(p * (1.0f / 255.0f), q * (1.0f / 255.0f), t * (1.0f / 255.0f));
		}
		else
		{
			return vec3(t * (1.0f / 255.0f), p * (1.0f / 255.0f), q * (1.0f / 255.0f));
		}
	}
	else
	{
		return vec3(0.0f);
	}
}
