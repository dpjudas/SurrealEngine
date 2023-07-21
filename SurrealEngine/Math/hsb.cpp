
#include "Precomp.h"
#include "hsb.h"

vec3 hsbtorgb(double hue, double saturation, double brightness)
{
	double red, green, blue;

	if (saturation == 0)
	{
		red = green = blue = brightness;
	}
	else
	{
		// the color wheel consists of 6 sectors. Figure out which sector you're in.
		double sectorPos = hue / 60.0;
		int sectorNumber = (int)(floor(sectorPos));
		// get the fractional part of the sector
		double fractionalSector = sectorPos - sectorNumber;

		// calculate values for the three axes of the color. 
		double p = brightness * (1.0 - saturation);
		double q = brightness * (1.0 - (saturation * fractionalSector));
		double t = brightness * (1.0 - (saturation * (1 - fractionalSector)));

		// assign the fractional colors to r, g, and b based on the sector the angle is in.
		switch (sectorNumber)
		{
		case 0:
			red = brightness;
			green = t;
			blue = p;
			break;
		case 1:
			red = q;
			green = brightness;
			blue = p;
			break;
		case 2:
			red = p;
			green = brightness;
			blue = t;
			break;
		case 3:
			red = p;
			green = q;
			blue = brightness;
			break;
		case 4:
			red = t;
			green = p;
			blue = brightness;
			break;
		case 5:
			red = brightness;
			green = p;
			blue = q;
			break;
		}
	}

	return vec3((float)red, (float)green, (float)blue);
}
