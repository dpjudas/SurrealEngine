
#include "Precomp.h"
#include "Shadowmap.h"
#include "Math/vec.h"
#include "Packages/Engine/Resources/Level/UModel.h"

Shadowmap::Shadowmap()
{
	// Precompute 3x3 gaussian blur table
	blurTable.resize(512);
	const float weights[9] = { 0.125f, 0.25f, 0.125f, 0.25f, 0.50f, 0.25f, 0.125f, 0.25f, 0.125f };
	for (int i = 0; i < 512; i++)
	{
		float src[9];
		for (int j = 0; j < 9; j++)
			src[j] = (float)((i >> j) & 1);
		float value = 0.0f;
		for (int yy = -1; yy <= 1; yy++)
		{
			for (int xx = -1; xx <= 1; xx++)
			{
				value += src[4 + xx + yy * 3] * weights[4 + xx + yy * 3];
			}
		}
		blurTable[i] = value;
	}
}

void Shadowmap::Clear(UModel* model, int lightMap)
{
	const LightMapIndex& lmindex = model->LightMap[lightMap];
	int width = lmindex.UClamp;
	int height = lmindex.VClamp;
	int size = width * height;
	if (pixels.size() < size)
		pixels.resize(size);
	this->width = width;
	this->height = height;
	float* dest = pixels.data();
	for (int i = 0; i < size; i++)
		dest[i] = 1.0f;
}

void Shadowmap::Load(UModel* model, int lightMap, int lightindex)
{
	const LightMapIndex& lmindex = model->LightMap[lightMap];
	int width = lmindex.UClamp;
	int height = lmindex.VClamp;
	int pitch = (width + 7) / 8;
	int size = width * height;
	if (pixels.size() < size)
		pixels.resize(size);
	if (tempbuf.size() < size)
		tempbuf.resize(size);
	this->width = width;
	this->height = height;

#if 1
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

#else // There is a bug in this that creates artifacts. It also wasn't much faster anyway...

	// Convert bits to floats and apply 3x3 gaussian blur

	const uint8_t* bits = model->LightBits.data() + lmindex.DataOffset + lightindex * pitch * height;
	if (width > 2 && height > 2)
	{
		const float* blur = blurTable.data();
		int offmiddle = 0;
		for (int y = 0; y < height; y++)
		{
			int offtop = (y > 0) ? offmiddle - pitch : offmiddle;
			int offbottom = (y < height - 1) ? offmiddle + pitch : offmiddle;

			uint32_t top = bits[offtop] & 0b111;
			uint32_t middle = bits[offmiddle] & 0b111;
			uint32_t bottom = bits[offbottom] & 0b111;

			float* line = &pixels[y * width];
			line[0] = blur[top | (middle << 3) | (bottom << 6)];
			int x = 2;
			while (x < width)
			{
				int byteidx = x >> 3;
				int bitidx = x & 7;
				top = ((top << 1) | ((bits[offtop + byteidx] >> bitidx) & 1)) & 0b111;
				middle = ((middle << 1) | ((bits[offmiddle + byteidx] >> bitidx) & 1)) & 0b111;
				bottom = ((bottom << 1) | ((bits[offbottom + byteidx] >> bitidx) & 1)) & 0b111;
				line[x - 1] = blur[top | (middle << 3) | (bottom << 6)];
				x++;
			}
			line[x - 1] = blur[top | (middle << 3) | (bottom << 6)];

			offmiddle += pitch;
		}
	}
	else
	{
		for (int y = 0; y < height; y++)
		{
			int offset = y * pitch;
			float* line = &pixels[y * width];
			for (int x = 0; x < width; x++)
			{
				int byteidx = x >> 3;
				int bitidx = x & 7;
				line[x] = (float)((bits[offset + byteidx] >> bitidx) & 1);
			}
		}
	}
#endif
}
