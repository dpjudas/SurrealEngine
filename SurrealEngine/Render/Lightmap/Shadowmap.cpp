
#include "Precomp.h"
#include "Shadowmap.h"
#include "Math/vec.h"
#include "UObject/ULevel.h"

void Shadowmap::Load(UModel* model, int lightMap, int lightindex)
{
	const LightMapIndex& lmindex = model->LightMap[lightMap];
	int width = lmindex.UClamp;
	int height = lmindex.VClamp;
	int pitch = (width + 7) / 8;

	// Stop allocations over time by building up a reserve

	int size = width * height;
	if (pixels.size() < size)
		pixels.resize(size);
	if (tempbuf.size() < size)
		tempbuf.resize(size);
	this->width = width;
	this->height = height;

	// Convert bits to floats that are easier to work with

	const uint8_t* bits = model->LightBits.data() + lmindex.DataOffset + lightindex * pitch * height;
	for (int y = 0; y < height; y++)
	{
		float* line = &tempbuf[y * width];
		for (int x = 0; x < width; x++)
		{
			bool shadowtest = (bits[x >> 3] & (1 << (x & 7))) != 0;
			line[x] = (float)shadowtest;
		}
		bits += pitch;
	}

	// Apply 3x3 gaussian blur
	static const float weights[9] = { 0.125f, 0.25f, 0.125f, 0.25f, 0.50f, 0.25f, 0.125f, 0.25f, 0.125f };
	float* dest = pixels.data();
	const float* src = tempbuf.data();
	for (int y = 0; y < height; y++, dest += width, src += width)
	{
		for (int x = 0; x < width; x++)
		{
			float value = 0.0f;
			for (int yy = -1; yy <= 1; yy++)
			{
				int yyy = clamp(y + yy, 0, height - 1) - y;
				for (int xx = -1; xx <= 1; xx++)
				{
					int xxx = clamp(x + xx, 0, width - 1);
					value += src[yyy * width + xxx] * weights[4 + xx + yy * 3];
				}
			}

			dest[x] = value;
		}
	}
}
