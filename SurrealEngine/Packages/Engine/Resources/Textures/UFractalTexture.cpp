
#include "Precomp.h"
#include "UFractalTexture.h"

void UFractalTexture::Load(ObjectStream* stream)
{
	UTexture::Load(stream);

	UsedFormat = TextureFormat::P8;
	UsedMipmaps.resize(1);

	int width = GetInt("UClamp");
	int height = GetInt("VClamp");

	UnrealMipmap& mipmap = UsedMipmaps.front();
	mipmap.Width = width;
	mipmap.Height = height;
	mipmap.Data.resize((size_t)mipmap.Width * mipmap.Height);
	uint8_t* pixels = (uint8_t*)mipmap.Data.data();
	memset(pixels, 0, (size_t)width * height);
}

void UFractalTexture::Save(PackageStreamWriter* stream)
{
	UTexture::Save(stream);
}
