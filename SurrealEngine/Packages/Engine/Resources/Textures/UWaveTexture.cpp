
#include "Precomp.h"
#include "UWaveTexture.h"
#include "Engine.h"
#include "Packages/Engine/Actors/Info/ULevelInfo.h"
#include "Light/LightEffect.h"
#include <algorithm>

void UWaveTexture::UpdateFrame()
{
	if (!TextureModified)
	{
		UnrealMipmap& mipmap = UsedMipmaps.front();

		int width = mipmap.Width;
		int height = mipmap.Height;
		uint8_t* pixels = (uint8_t*)mipmap.Data.data();

		float timeOffset = engine->LevelInfo->TimeSeconds() * 3.0f;
		const float pi2 = 3.14159265358979323846f * 2.0f;

		// Note: this is completely wrong and is made for nyleve's waterfall.
		// The correct method is to read the WaterPaint properties, then
		// use it as input to a wave effect where each texel changes based
		// on the neighbouring texels, just like in shaders that create wave
		// effects.

		for (int y = 0; y < height; y++)
		{
			float v2 = y * (2.0f / height);
			uint8_t* destline = pixels + y * width;
			for (int x = 0; x < width; x++)
			{
				int dx = (width >> 1) - x;
				int dy = (height >> 1) - y;
				float len = std::sqrt((float)(dx * dx + dy * dy));
				float wave = 0.5f * LightEffect::Sin(len * 0.6f - timeOffset);
				wave = wave * std::max(1.0f - len * (3.0f / width), 0.0f); // fade it out before we reach the edge
				destline[x] = (uint8_t)std::max(std::min((int)(85.0f + wave * 120.0f + 0.5f), 255), 0);
			}
		}

		TextureModified = true;
	}
}
