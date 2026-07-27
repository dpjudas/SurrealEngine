
#include "Precomp.h"
#include "UIceTexture.h"

void UIceTexture::UpdateFrame()
{
	if (!TextureModified)
	{
		UnrealMipmap& mipmap = UsedMipmaps.front();

		int width = mipmap.Width;
		int height = mipmap.Height;
		uint8_t* pixels = (uint8_t*)mipmap.Data.data();
		int count = width * height;

		UTexture* tex = SourceTexture();
		if (tex && !tex->UsedMipmaps.empty() && tex->UsedMipmaps.front().Width == mipmap.Width && tex->UsedMipmaps.front().Height == mipmap.Height)
		{
			const uint8_t* srcpixels = (const uint8_t*)tex->UsedMipmaps.front().Data.data();
			for (int i = 0; i < count; i++)
			{
				pixels[i] = srcpixels[i];
			}
		}
		else
		{
			for (int i = 0; i < count; i++)
			{
				pixels[i] = 200;
			}
		}

		TextureModified = true;
	}
}
