#pragma once

#include "vec.h"

inline vec3 hsbtorgb(uint8_t hue, uint8_t saturation, uint8_t brightness)
{
	double s = (saturation / 2.5);
	if (s > 32.0)
		s += 2.0;

	double v = (6.512735 * std::sqrt((double)brightness));
	double r = 0.0;
	double g = 0.0;
	double b = 0.0;

	if (saturation >= 250)
	{
		r = v;
		g = v;
		b = v;
	}
	else if (brightness > 0)
	{ 
		double sectorPos = hue / 85.0;
		int sectorNum = (int)(floor(sectorPos));
		double sectorFrac = sectorPos - sectorNum;

		double p = s * (v / 104.0);
		double q = ((1.0 - sectorFrac) * v) + (p * sectorFrac);
		double t = (sectorFrac * v) + (p * (1.0 - sectorFrac));

		switch (sectorNum)
		{
		case 0:
			r = q;
			g = t;
			b = p;
			break;
		case 1:
			r = p;
			g = q;
			b = t;
			break;
		case 2:
			r = t;
			g = p;
			b = q;
			break;
		}
	}

	return vec3((float)r/255.0f, (float)g/255.0f, (float)b/255.0f);
}
