
#include "Precomp.h"
#include "UWetTexture.h"

void UWetTexture::UpdateFrame()
{
	if (!TextureModified)
	{
		UpdateWater();

		UnrealMipmap& mipmap = UsedMipmaps.front();

		UTexture* tex = SourceTexture();
		if (tex && !tex->UsedMipmaps.empty() && tex->UsedMipmaps.front().Width == mipmap.Width && tex->UsedMipmaps.front().Height == mipmap.Height)
		{
			int width = mipmap.Width;
			int height = mipmap.Height;
			uint8_t* pixels = (uint8_t*)mipmap.Data.data();
			const uint8_t* srcpixels = (const uint8_t*)tex->UsedMipmaps.front().Data.data();
			for (int y = 0; y < height; y++)
			{
				const WaterPixel* waterline = &WaterDepth[CurrentWaterDepth][y * width];
				const uint8_t* srcline = srcpixels + y * width;
				uint8_t* destline = pixels + y * width;
				for (int x = 0; x < width; x++)
				{
					// Use water as displacement

					int water = (int)(0.5f * waterline[x].XGradient * width);
					int srcx = clamp(x + water, 0, width - 1);
					destline[x] = srcline[srcx];
				}
			}
		}

		TextureModified = true;
	}
}
