
#include "Precomp.h"
#include "TextureUploader.h"
#include "UObject/UTexture.h"

TextureUploader* TextureUploader::GetUploader(TextureFormat format)
{
	static std::map<TextureFormat, std::unique_ptr<TextureUploader>> Uploaders;
	if (Uploaders.empty())
	{
		// Original.
		Uploaders[TextureFormat::P8].reset(new TextureUploader_P8());
		Uploaders[TextureFormat::BGRA8_LM].reset(new TextureUploader_BGRA8_LM());
		Uploaders[TextureFormat::R5G6B5].reset(new TextureUploader_Simple(VK_FORMAT_R5G6B5_UNORM_PACK16, 2));
		// Note: according to formats this should have been VK_FORMAT_BC1_RGB_UNORM_BLOCK, but some textures does use the alpha bit!
		Uploaders[TextureFormat::BC1].reset(new TextureUploader_4x4Block(VK_FORMAT_BC1_RGBA_UNORM_BLOCK, 8));
		Uploaders[TextureFormat::RGB8].reset(new TextureUploader_Simple(VK_FORMAT_R8G8B8_UNORM, 3));
		Uploaders[TextureFormat::BGRA8].reset(new TextureUploader_Simple(VK_FORMAT_B8G8R8A8_UNORM, 4));

		// S3TC (continued).
		Uploaders[TextureFormat::BC2].reset(new TextureUploader_4x4Block(VK_FORMAT_BC2_UNORM_BLOCK, 16));
		Uploaders[TextureFormat::BC3].reset(new TextureUploader_4x4Block(VK_FORMAT_BC3_UNORM_BLOCK, 16));

		// RGTC.
		Uploaders[TextureFormat::BC4].reset(new TextureUploader_4x4Block(VK_FORMAT_BC4_UNORM_BLOCK, 8));
		Uploaders[TextureFormat::BC4_S].reset(new TextureUploader_4x4Block(VK_FORMAT_BC4_SNORM_BLOCK, 8));
		Uploaders[TextureFormat::BC5].reset(new TextureUploader_4x4Block(VK_FORMAT_BC5_UNORM_BLOCK, 16));
		Uploaders[TextureFormat::BC5_S].reset(new TextureUploader_4x4Block(VK_FORMAT_BC5_SNORM_BLOCK, 16));

		// BPTC.
		Uploaders[TextureFormat::BC7].reset(new TextureUploader_4x4Block(VK_FORMAT_BC7_UNORM_BLOCK, 16));
		Uploaders[TextureFormat::BC6H_S].reset(new TextureUploader_4x4Block(VK_FORMAT_BC6H_SFLOAT_BLOCK, 16));
		Uploaders[TextureFormat::BC6H].reset(new TextureUploader_4x4Block(VK_FORMAT_BC6H_UFLOAT_BLOCK, 16));

		// Normalized RGBA.
		Uploaders[TextureFormat::RGBA16].reset(new TextureUploader_Simple(VK_FORMAT_R16G16B16A16_UNORM, 8));
		Uploaders[TextureFormat::RGBA16_S].reset(new TextureUploader_Simple(VK_FORMAT_R16G16B16A16_SNORM, 8));
		//Uploaders[TextureFormat::RGBA32].reset(new TextureUploader_Simple(VK_FORMAT_R32G32B32A32_UNORM, 16));
		//Uploaders[TextureFormat::RGBA32_S].reset(new TextureUploader_Simple(VK_FORMAT_R32G32B32A32_SNORM, 16));

		// S3TC (continued).
		Uploaders[TextureFormat::BC1_PA].reset(new TextureUploader_4x4Block(VK_FORMAT_BC1_RGBA_UNORM_BLOCK, 8));

		// Normalized RGBA (continued).
		Uploaders[TextureFormat::R8].reset(new TextureUploader_Simple(VK_FORMAT_R8_UNORM, 1));
		Uploaders[TextureFormat::R8_S].reset(new TextureUploader_Simple(VK_FORMAT_R8_SNORM, 1));
		Uploaders[TextureFormat::R16].reset(new TextureUploader_Simple(VK_FORMAT_R16_UNORM, 2));
		Uploaders[TextureFormat::R16_S].reset(new TextureUploader_Simple(VK_FORMAT_R16_SNORM, 2));
		//Uploaders[TextureFormat::R32].reset(new TextureUploader_Simple(VK_FORMAT_R32_UNORM, 4));
		//Uploaders[TextureFormat::R32_S].reset(new TextureUploader_Simple(VK_FORMAT_R32_SNORM, 4));
		Uploaders[TextureFormat::RG8].reset(new TextureUploader_Simple(VK_FORMAT_R8G8_UNORM, 2));
		Uploaders[TextureFormat::RG8_S].reset(new TextureUploader_Simple(VK_FORMAT_R8G8_SNORM, 2));
		Uploaders[TextureFormat::RG16].reset(new TextureUploader_Simple(VK_FORMAT_R16G16_UNORM, 4));
		Uploaders[TextureFormat::RG16_S].reset(new TextureUploader_Simple(VK_FORMAT_R16G16_SNORM, 4));
		//Uploaders[TextureFormat::RG32].reset(new TextureUploader_Simple(VK_FORMAT_R32G32_UNORM, 8));
		//Uploaders[TextureFormat::RG32_S].reset(new TextureUploader_Simple(VK_FORMAT_R32G32_SNORM, 8));
		Uploaders[TextureFormat::RGB8_S].reset(new TextureUploader_Simple(VK_FORMAT_R8G8B8_SNORM, 3));
		Uploaders[TextureFormat::RGB16_].reset(new TextureUploader_Simple(VK_FORMAT_R16G16B16_UNORM, 6));
		Uploaders[TextureFormat::RGB16_S].reset(new TextureUploader_Simple(VK_FORMAT_R16G16B16_SNORM, 6));
		//Uploaders[TextureFormat::RGB32].reset(new TextureUploader_Simple(VK_FORMAT_R32G32B32_UNORM, 12));
		//Uploaders[TextureFormat::RGB32_S].reset(new TextureUploader_Simple(VK_FORMAT_R32G32B32_SNORM, 12));
		Uploaders[TextureFormat::RGBA8_].reset(new TextureUploader_Simple(VK_FORMAT_R8G8B8A8_UNORM, 4));
		Uploaders[TextureFormat::RGBA8_S].reset(new TextureUploader_Simple(VK_FORMAT_R8G8B8A8_SNORM, 4));

		// Floating point RGBA.
		Uploaders[TextureFormat::R16_F].reset(new TextureUploader_Simple(VK_FORMAT_R16_SFLOAT, 2));
		Uploaders[TextureFormat::R32_F].reset(new TextureUploader_Simple(VK_FORMAT_R32_SFLOAT, 4));
		Uploaders[TextureFormat::RG16_F].reset(new TextureUploader_Simple(VK_FORMAT_R16G16_SFLOAT, 4));
		Uploaders[TextureFormat::RG32_F].reset(new TextureUploader_Simple(VK_FORMAT_R32G32_SFLOAT, 8));
		Uploaders[TextureFormat::RGB16_F].reset(new TextureUploader_Simple(VK_FORMAT_R16G16B16_SFLOAT, 6));
		Uploaders[TextureFormat::RGB32_F].reset(new TextureUploader_Simple(VK_FORMAT_R32G32B32_SFLOAT, 12));
		Uploaders[TextureFormat::RGBA16_F].reset(new TextureUploader_Simple(VK_FORMAT_R16G16B16A16_SFLOAT, 8));
		Uploaders[TextureFormat::RGBA32_F].reset(new TextureUploader_Simple(VK_FORMAT_R32G32B32A32_SFLOAT, 16));

		// ETC1/ETC2/EAC.
		//Uploaders[TextureFormat::ETC1].reset(new TextureUploader_4x4Block(VK_FORMAT_ETC1_R8G8B8_UNORM_BLOCK, 8));
		Uploaders[TextureFormat::ETC2].reset(new TextureUploader_4x4Block(VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK, 8));
		Uploaders[TextureFormat::ETC2_PA].reset(new TextureUploader_4x4Block(VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK, 8));
		Uploaders[TextureFormat::ETC2_RGB_EAC_A].reset(new TextureUploader_4x4Block(VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK, 16));
		Uploaders[TextureFormat::EAC_R].reset(new TextureUploader_4x4Block(VK_FORMAT_EAC_R11_UNORM_BLOCK, 8));
		Uploaders[TextureFormat::EAC_R_S].reset(new TextureUploader_4x4Block(VK_FORMAT_EAC_R11_SNORM_BLOCK, 8));
		Uploaders[TextureFormat::EAC_RG].reset(new TextureUploader_4x4Block(VK_FORMAT_EAC_R11G11_UNORM_BLOCK, 16));
		Uploaders[TextureFormat::EAC_RG_S].reset(new TextureUploader_4x4Block(VK_FORMAT_EAC_R11G11_SNORM_BLOCK, 16));

		// ASTC.
		Uploaders[TextureFormat::ASTC_4x4].reset(new TextureUploader_2DBlock(VK_FORMAT_ASTC_4x4_UNORM_BLOCK, 4, 4, 16));
		Uploaders[TextureFormat::ASTC_5x4].reset(new TextureUploader_2DBlock(VK_FORMAT_ASTC_5x4_UNORM_BLOCK, 5, 4, 16));
		Uploaders[TextureFormat::ASTC_5x5].reset(new TextureUploader_2DBlock(VK_FORMAT_ASTC_5x5_UNORM_BLOCK, 5, 5, 16));
		Uploaders[TextureFormat::ASTC_6x5].reset(new TextureUploader_2DBlock(VK_FORMAT_ASTC_6x5_UNORM_BLOCK, 6, 5, 16));
		Uploaders[TextureFormat::ASTC_6x6].reset(new TextureUploader_2DBlock(VK_FORMAT_ASTC_6x6_UNORM_BLOCK, 6, 6, 16));
		Uploaders[TextureFormat::ASTC_8x5].reset(new TextureUploader_2DBlock(VK_FORMAT_ASTC_8x5_UNORM_BLOCK, 8, 5, 16));
		Uploaders[TextureFormat::ASTC_8x6].reset(new TextureUploader_2DBlock(VK_FORMAT_ASTC_8x6_UNORM_BLOCK, 8, 6, 16));
		Uploaders[TextureFormat::ASTC_8x8].reset(new TextureUploader_2DBlock(VK_FORMAT_ASTC_8x8_UNORM_BLOCK, 8, 8, 16));
		Uploaders[TextureFormat::ASTC_10x5].reset(new TextureUploader_2DBlock(VK_FORMAT_ASTC_10x5_UNORM_BLOCK, 10, 5, 16));
		Uploaders[TextureFormat::ASTC_10x6].reset(new TextureUploader_2DBlock(VK_FORMAT_ASTC_10x6_UNORM_BLOCK, 10, 6, 16));
		Uploaders[TextureFormat::ASTC_10x8].reset(new TextureUploader_2DBlock(VK_FORMAT_ASTC_10x8_UNORM_BLOCK, 10, 8, 16));
		Uploaders[TextureFormat::ASTC_10x10].reset(new TextureUploader_2DBlock(VK_FORMAT_ASTC_10x10_UNORM_BLOCK, 10, 10, 16));
		Uploaders[TextureFormat::ASTC_12x10].reset(new TextureUploader_2DBlock(VK_FORMAT_ASTC_12x10_UNORM_BLOCK, 12, 10, 16));
		Uploaders[TextureFormat::ASTC_12x12].reset(new TextureUploader_2DBlock(VK_FORMAT_ASTC_12x12_UNORM_BLOCK, 12, 12, 16));
		//Uploaders[TextureFormat::ASTC_3x3x3].reset(new TextureUploader_3DBlock(VK_FORMAT_ASTC_3x3x3_UNORM_BLOCK, 3, 3, 3, 16));
		//Uploaders[TextureFormat::ASTC_4x3x3].reset(new TextureUploader_3DBlock(VK_FORMAT_ASTC_4x3x3_UNORM_BLOCK, 4, 3, 3, 16));
		//Uploaders[TextureFormat::ASTC_4x4x3].reset(new TextureUploader_3DBlock(VK_FORMAT_ASTC_4x4x3_UNORM_BLOCK, 4, 4, 3, 16));
		//Uploaders[TextureFormat::ASTC_4x4x4].reset(new TextureUploader_3DBlock(VK_FORMAT_ASTC_4x4x4_UNORM_BLOCK, 4, 4, 4, 16));
		//Uploaders[TextureFormat::ASTC_5x4x4].reset(new TextureUploader_3DBlock(VK_FORMAT_ASTC_5x4x4_UNORM_BLOCK, 5, 4, 4, 16));
		//Uploaders[TextureFormat::ASTC_5x5x4].reset(new TextureUploader_3DBlock(VK_FORMAT_ASTC_5x5x4_UNORM_BLOCK, 5, 5, 4, 16));
		//Uploaders[TextureFormat::ASTC_5x5x5].reset(new TextureUploader_3DBlock(VK_FORMAT_ASTC_5x5x5_UNORM_BLOCK, 5, 5, 5, 16));
		//Uploaders[TextureFormat::ASTC_6x5x5].reset(new TextureUploader_3DBlock(VK_FORMAT_ASTC_6x5x5_UNORM_BLOCK, 6, 5, 5, 16));
		//Uploaders[TextureFormat::ASTC_6x6x5].reset(new TextureUploader_3DBlock(VK_FORMAT_ASTC_6x6x5_UNORM_BLOCK, 6, 6, 5, 16));
		//Uploaders[TextureFormat::ASTC_6x6x6].reset(new TextureUploader_3DBlock(VK_FORMAT_ASTC_6x6x6_UNORM_BLOCK, 6, 6, 6, 16));

		// PVRTC.
		// Requires VK_IMG_format_pvrtc
		//Uploaders[VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG].reset(new TextureUploader_2DBlock(VK_FORMAT_ASTC_12x12_UNORM_BLOCK, 8, 4, 8));
		//Uploaders[VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG].reset(new TextureUploader_4x4Block(VK_FORMAT_ASTC_12x12_UNORM_BLOCK, 8));
		//Uploaders[VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG].reset(new TextureUploader_2DBlock(VK_FORMAT_ASTC_12x12_UNORM_BLOCK, 8, 4, 8));
		//Uploaders[VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG].reset(new TextureUploader_4x4Block(VK_FORMAT_ASTC_12x12_UNORM_BLOCK, 8));

		// RGBA (Integral).
		Uploaders[TextureFormat::R8_UI].reset(new TextureUploader_Simple(VK_FORMAT_R8_UINT, 1));
		Uploaders[TextureFormat::R8_I].reset(new TextureUploader_Simple(VK_FORMAT_R8_SINT, 1));
		Uploaders[TextureFormat::R16_UI].reset(new TextureUploader_Simple(VK_FORMAT_R16_UINT, 2));
		Uploaders[TextureFormat::R16_I].reset(new TextureUploader_Simple(VK_FORMAT_R16_SINT, 2));
		Uploaders[TextureFormat::R32_UI].reset(new TextureUploader_Simple(VK_FORMAT_R32_UINT, 4));
		Uploaders[TextureFormat::R32_I].reset(new TextureUploader_Simple(VK_FORMAT_R32_SINT, 4));
		Uploaders[TextureFormat::RG8_UI].reset(new TextureUploader_Simple(VK_FORMAT_R8G8_UINT, 2));
		Uploaders[TextureFormat::RG8_I].reset(new TextureUploader_Simple(VK_FORMAT_R8G8_SINT, 2));
		Uploaders[TextureFormat::RG16_UI].reset(new TextureUploader_Simple(VK_FORMAT_R16G16_UINT, 4));
		Uploaders[TextureFormat::RG16_I].reset(new TextureUploader_Simple(VK_FORMAT_R16G16_SINT, 4));
		Uploaders[TextureFormat::RG32_UI].reset(new TextureUploader_Simple(VK_FORMAT_R32G32_UINT, 8));
		Uploaders[TextureFormat::RG32_I].reset(new TextureUploader_Simple(VK_FORMAT_R32G32_SINT, 8));
		Uploaders[TextureFormat::RGB8_UI].reset(new TextureUploader_Simple(VK_FORMAT_R8G8B8_UINT, 3));
		Uploaders[TextureFormat::RGB8_I].reset(new TextureUploader_Simple(VK_FORMAT_R8G8B8_SINT, 3));
		Uploaders[TextureFormat::RGB16_UI].reset(new TextureUploader_Simple(VK_FORMAT_R16G16B16_UINT, 6));
		Uploaders[TextureFormat::RGB16_I].reset(new TextureUploader_Simple(VK_FORMAT_R16G16B16_SINT, 6));
		Uploaders[TextureFormat::RGB32_UI].reset(new TextureUploader_Simple(VK_FORMAT_R32G32B32_UINT, 12));
		Uploaders[TextureFormat::RGB32_I].reset(new TextureUploader_Simple(VK_FORMAT_R32G32B32_SINT, 12));
		Uploaders[TextureFormat::RGBA8_UI].reset(new TextureUploader_Simple(VK_FORMAT_R8G8B8A8_UINT, 4));
		Uploaders[TextureFormat::RGBA8_I].reset(new TextureUploader_Simple(VK_FORMAT_R8G8B8A8_SINT, 4));
		Uploaders[TextureFormat::RGBA16_UI].reset(new TextureUploader_Simple(VK_FORMAT_R16G16B16A16_UINT, 8));
		Uploaders[TextureFormat::RGBA16_I].reset(new TextureUploader_Simple(VK_FORMAT_R16G16B16A16_SINT, 8));
		Uploaders[TextureFormat::RGBA32_UI].reset(new TextureUploader_Simple(VK_FORMAT_R32G32B32A32_UINT, 16));
		Uploaders[TextureFormat::RGBA32_I].reset(new TextureUploader_Simple(VK_FORMAT_R32G32B32A32_SINT, 16));

		// Special.
		//Uploaders[TextureFormat::ARGB8].reset(new TextureUploader_ARGB8());
		//Uploaders[TextureFormat::ABGR8].reset(new TextureUploader_ABGR8());
		Uploaders[TextureFormat::RGB10A2].reset(new TextureUploader_RGB10A2());
		Uploaders[TextureFormat::RGB10A2_UI].reset(new TextureUploader_RGB10A2_UI());
		Uploaders[TextureFormat::RGB10A2_LM].reset(new TextureUploader_RGB10A2_LM());
		//Uploaders[TextureFormat::RGB9E5].reset(new TextureUploader_RGB9E5());
		//Uploaders[TextureFormat::P8_RGB9E5].reset(new TextureUploader_P8_RGB9E5());
		//Uploaders[TextureFormat::R1].reset(new TextureUploader_R1());
		//Uploaders[TextureFormat::RGB10A2_S].reset(new TextureUploader_RGB10A2_S());
		//Uploaders[TextureFormat::RGB10A2_I].reset(new TextureUploader_RGB10A2_I());
		//Uploaders[TextureFormat::R11G11B10_F].reset(new TextureUploader_R11G11B10_F());

		// Normalized BGR.
		//Uploaders[TextureFormat::B5G6R5].reset(new TextureUploader_B5G6R5());
		//Uploaders[TextureFormat::BGR8].reset(new TextureUploader_BGR8());

		// Double precission floating point RGBA.
		//Uploaders[TextureFormat::R64_F].reset(new TextureUploader_R64_F());
		//Uploaders[TextureFormat::RG64_F].reset(new TextureUploader_RG64_F());
		//Uploaders[TextureFormat::RGB64_F].reset(new TextureUploader_RGB64_F());
		//Uploaders[TextureFormat::RGBA64_F].reset(new TextureUploader_RGBA64_F());
	}

	auto it = Uploaders.find(format);
	if (it != Uploaders.end())
		return it->second.get();
	else
		return nullptr;
}

/////////////////////////////////////////////////////////////////////////////

int TextureUploader_P8::GetUploadSize(int x, int y, int w, int h)
{
	return w * h * 4;
}

void TextureUploader_P8::UploadRect(void* d, UnrealMipmap* mip, int x, int y, int w, int h, FColor* palette, bool masked)
{
	int pitch = mip->Width;
	BYTE* src = mip->Data.data() + x + y * pitch;
	FColor* Ptr = (FColor*)d;
	if (masked)
	{
		FColor translucent(0, 0, 0, 0);
		for (int i = 0; i < h; i++)
		{
			for (int j = 0; j < w; j++)
			{
				int idx = src[j];
				*Ptr++ = (idx != 0) ? palette[idx] : translucent;
			}
			src += pitch;
		}
	}
	else
	{
		for (int i = 0; i < h; i++)
		{
			for (int j = 0; j < w; j++)
			{
				int idx = src[j];
				*Ptr++ = palette[idx];
			}
			src += pitch;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////

int TextureUploader_BGRA8_LM::GetUploadSize(int x, int y, int w, int h)
{
	return w * h * 4;
}

void TextureUploader_BGRA8_LM::UploadRect(void* dst, UnrealMipmap* mip, int x, int y, int w, int h, FColor* palette, bool masked)
{
	int pitch = mip->Width;
	FColor* src = ((FColor*)mip->Data.data()) + x + y * pitch;
	auto Ptr = (FColor*)dst;
	for (int i = 0; i < h; i++)
	{
		for (int j = 0; j < w; j++)
		{
			FColor Src = src[j];
			Ptr->R = Src.B << 1;
			Ptr->G = Src.G << 1;
			Ptr->B = Src.R << 1;
			Ptr->A = Src.A << 1;
			Ptr++;
		}
		src += pitch;
	}
}

/////////////////////////////////////////////////////////////////////////////

int TextureUploader_RGB10A2::GetUploadSize(int x, int y, int w, int h)
{
	return w * h * 8;
}

void TextureUploader_RGB10A2::UploadRect(void* dst, UnrealMipmap* mip, int x, int y, int w, int h, FColor* palette, bool masked)
{
	int pitch = mip->Width;
	uint32_t* src = ((uint32_t*)mip->Data.data()) + x + y * pitch;
	uint16_t* Ptr = (uint16_t*)dst;
	for (int i = 0; i < h; i++)
	{
		for (int j = 0; j < w; j++)
		{
			uint32_t c = *Ptr;
			uint32_t r = (c >> 22) & 0x3ff;
			uint32_t g = (c >> 12) & 0x3ff;
			uint32_t b = (c >> 2) & 0x3ff;
			uint32_t a = c & 0x3;

			r = r * 0xffff / 0x3ff;
			g = g * 0xffff / 0x3ff;
			b = b * 0xffff / 0x3ff;
			a = a * 0xffff / 0x3;

			*(Ptr++) = r;
			*(Ptr++) = g;
			*(Ptr++) = b;
			*(Ptr++) = a;
		}
		src += pitch;
	}
}

/////////////////////////////////////////////////////////////////////////////

int TextureUploader_RGB10A2_UI::GetUploadSize(int x, int y, int w, int h)
{
	return w * h * 8;
}

void TextureUploader_RGB10A2_UI::UploadRect(void* dst, UnrealMipmap* mip, int x, int y, int w, int h, FColor* palette, bool masked)
{
	int pitch = mip->Width;
	uint32_t* src = ((uint32_t*)mip->Data.data()) + x + y * pitch;
	uint16_t* Ptr = (uint16_t*)dst;
	for (int i = 0; i < h; i++)
	{
		for (int j = 0; j < w; j++)
		{
			uint32_t c = *Ptr;
			uint32_t r = (c >> 22) & 0x3ff;
			uint32_t g = (c >> 12) & 0x3ff;
			uint32_t b = (c >> 2) & 0x3ff;
			uint32_t a = c & 0x3;

			*(Ptr++) = r;
			*(Ptr++) = g;
			*(Ptr++) = b;
			*(Ptr++) = a;
		}
		src += pitch;
	}
}

/////////////////////////////////////////////////////////////////////////////

int TextureUploader_RGB10A2_LM::GetUploadSize(int x, int y, int w, int h)
{
	return w * h * 8;
}

void TextureUploader_RGB10A2_LM::UploadRect(void* dst, UnrealMipmap* mip, int x, int y, int w, int h, FColor* palette, bool masked)
{
	int pitch = mip->Width;
	uint32_t* src = ((uint32_t*)mip->Data.data()) + x + y * pitch;
	uint16_t* Ptr = (uint16_t*)dst;
	for (int i = 0; i < h; i++)
	{
		for (int j = 0; j < w; j++)
		{
			uint32_t c = *Ptr;
			uint32_t r = (c >> 22) & 0x3ff;
			uint32_t g = (c >> 12) & 0x3ff;
			uint32_t b = (c >> 2) & 0x3ff;
			uint32_t a = c & 0x3;

			r = (r << 1) * 0xffff / 0xff;
			g = (g << 1) * 0xffff / 0xff;
			b = (b << 1) * 0xffff / 0xff;
			a = (a << 1) * 0xffff / 0x3;

			*(Ptr++) = r;
			*(Ptr++) = g;
			*(Ptr++) = b;
			*(Ptr++) = a;
		}
		src += pitch;
	}
}

/////////////////////////////////////////////////////////////////////////////

int TextureUploader_Simple::GetUploadSize(int x, int y, int w, int h)
{
	return w * h * BytesPerPixel;
}

void TextureUploader_Simple::UploadRect(void* d, UnrealMipmap* mip, int x, int y, int w, int h, FColor* palette, bool masked)
{
	int pitch = mip->Width * BytesPerPixel;
	int size = w * BytesPerPixel;
	BYTE* src = mip->Data.data() + x * BytesPerPixel + y * pitch;
	BYTE* dst = (BYTE*)d;
	for (int i = 0; i < h; i++)
	{
		memcpy(dst, src, size);
		dst += size;
		src += pitch;
	}
}

/////////////////////////////////////////////////////////////////////////////

int TextureUploader_4x4Block::GetUploadSize(int x, int y, int w, int h)
{
	int x0 = x / 4;
	int y0 = y / 4;
	int x1 = (x + w + 3) / 4;
	int y1 = (y + h + 3) / 4;
	return (x1 - x0) * (y1 - y0) * BytesPerBlock;
}

void TextureUploader_4x4Block::UploadRect(void* d, UnrealMipmap* mip, int x, int y, int w, int h, FColor* palette, bool masked)
{
	int x0 = x / 4;
	int y0 = y / 4;
	int x1 = (x + w + 3) / 4;
	int y1 = (y + h + 3) / 4;

	int pitch = (mip->Width + 3) / 4 * BytesPerBlock;
	int size = (x1 - x0) * BytesPerBlock;
	BYTE* src = mip->Data.data() + x0 * BytesPerBlock + y0 * pitch;
	BYTE* dst = (BYTE*)d;
	for (int i = y0; i < y1; i++)
	{
		memcpy(dst, src, size);
		dst += size;
		src += pitch;
	}
}


/////////////////////////////////////////////////////////////////////////////

int TextureUploader_2DBlock::GetUploadSize(int x, int y, int w, int h)
{
	int x0 = x / BlockX;
	int y0 = y / BlockY;
	int x1 = (x + w + BlockX - 1) / BlockX;
	int y1 = (y + h + BlockY - 1) / BlockY;
	return (x1 - x0) * (y1 - y0) * BytesPerBlock;
}

void TextureUploader_2DBlock::UploadRect(void* d, UnrealMipmap* mip, int x, int y, int w, int h, FColor* palette, bool masked)
{
	int x0 = x / BlockX;
	int y0 = y / BlockY;
	int x1 = (x + w + BlockX - 1) / BlockX;
	int y1 = (y + h + BlockY - 1) / BlockY;

	int pitch = (mip->Width + BlockX - 1) / BlockX * BytesPerBlock;
	int size = (x1 - x0) * BytesPerBlock;
	BYTE* src = mip->Data.data() + x0 * BytesPerBlock + y0 * pitch;
	BYTE* dst = (BYTE*)d;
	for (int i = y0; i < y1; i++)
	{
		memcpy(dst, src, size);
		dst += size;
		src += pitch;
	}
}
