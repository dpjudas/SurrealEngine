
#include "Precomp.h"
#include "D3D11TextureUploader.h"
#include "UObject/UTexture.h"
#include "RenderDevice/RenderDevice.h"
#include <map>

#ifdef USE_SSE2
#include <immintrin.h>
#endif

D3D11TextureUploader* D3D11TextureUploader::GetUploader(TextureFormat format)
{
	static std::map<TextureFormat, std::unique_ptr<D3D11TextureUploader>> Uploaders;
	if (Uploaders.empty())
	{
		Uploaders[TextureFormat::P8].reset(new D3D11TextureUploader_P8());
		Uploaders[TextureFormat::BGRA8_LM].reset(new D3D11TextureUploader_BGRA8_LM());
		Uploaders[TextureFormat::R5G6B5].reset(new D3D11TextureUploader_Simple(DXGI_FORMAT_B5G6R5_UNORM, 2));
		Uploaders[TextureFormat::BC1].reset(new D3D11TextureUploader_4x4Block(DXGI_FORMAT_BC1_UNORM, 8));
		Uploaders[TextureFormat::RGB8].reset(new D3D11TextureUploader_RGB8());
		Uploaders[TextureFormat::BGRA8].reset(new D3D11TextureUploader_Simple(DXGI_FORMAT_B8G8R8A8_UNORM, 4));
		Uploaders[TextureFormat::RGBA32_F].reset(new D3D11TextureUploader_Simple(DXGI_FORMAT_R32G32B32A32_FLOAT, 16));
	}

	auto it = Uploaders.find(format);
	if (it != Uploaders.end())
		return it->second.get();
	else
		return nullptr;
}

/////////////////////////////////////////////////////////////////////////////

int D3D11TextureUploader_P8::GetUploadSize(int x, int y, int w, int h)
{
	return w * h * 4;
}

void D3D11TextureUploader_P8::UploadRect(void* d, UnrealMipmap* mip, int x, int y, int w, int h, FColor* palette, bool masked)
{
	int pitch = mip->Width;
	uint8_t* src = mip->Data.data() + x + y * pitch;
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

int D3D11TextureUploader_RGB8::GetUploadSize(int x, int y, int w, int h)
{
	return w * h * 4;
}

void D3D11TextureUploader_RGB8::UploadRect(void* dst, UnrealMipmap* mip, int x, int y, int w, int h, FColor* palette, bool masked)
{
	int pitch = mip->Width * 3;
	uint8_t* src = ((uint8_t*)mip->Data.data()) + x + y * pitch;
	auto Ptr = (FColor*)dst;
	for (int i = 0; i < h; i++)
	{
		int k = 0;
		for (int j = 0; j < w; j++)
		{
			Ptr->R = src[k++];
			Ptr->G = src[k++];
			Ptr->B = src[k++];
			Ptr->A = 255;
			Ptr++;
		}
		src += pitch;
	}
}

/////////////////////////////////////////////////////////////////////////////

int D3D11TextureUploader_BGRA8_LM::GetUploadSize(int x, int y, int w, int h)
{
	return w * h * 4;
}

void D3D11TextureUploader_BGRA8_LM::UploadRect(void* dst, UnrealMipmap* mip, int x, int y, int w, int h, FColor* palette, bool masked)
{
#ifdef USE_SSE2
	int pitch = mip->Width;
	FColor* src = ((FColor*)mip->Data.data()) + x + y * pitch;
	auto Ptr = (FColor*)dst;
	if (w % 4 == 0)
	{
		for (int i = 0; i < h; i++)
		{
			for (int j = 0; j < w; j += 4)
			{
				__m128i p = _mm_loadu_si128((const __m128i*)(src + j));
				__m128i p_hi = _mm_unpackhi_epi8(p, _mm_setzero_si128());
				__m128i p_lo = _mm_unpacklo_epi8(p, _mm_setzero_si128());
				p_hi = _mm_shufflehi_epi16(p_hi, _MM_SHUFFLE(3, 0, 1, 2));
				p_hi = _mm_shufflelo_epi16(p_hi, _MM_SHUFFLE(3, 0, 1, 2));
				p_hi = _mm_slli_epi16(p_hi, 1);
				p_lo = _mm_shufflehi_epi16(p_lo, _MM_SHUFFLE(3, 0, 1, 2));
				p_lo = _mm_shufflelo_epi16(p_lo, _MM_SHUFFLE(3, 0, 1, 2));
				p_lo = _mm_slli_epi16(p_lo, 1);
				p = _mm_packus_epi16(p_lo, p_hi);
				_mm_storeu_si128((__m128i*)(Ptr + j), p);
			}
			Ptr += w;
			src += pitch;
		}
	}
	else
	{
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
#else
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
#endif
}

/////////////////////////////////////////////////////////////////////////////

int D3D11TextureUploader_RGB10A2::GetUploadSize(int x, int y, int w, int h)
{
	return w * h * 8;
}

void D3D11TextureUploader_RGB10A2::UploadRect(void* dst, UnrealMipmap* mip, int x, int y, int w, int h, FColor* palette, bool masked)
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

int D3D11TextureUploader_RGB10A2_UI::GetUploadSize(int x, int y, int w, int h)
{
	return w * h * 8;
}

void D3D11TextureUploader_RGB10A2_UI::UploadRect(void* dst, UnrealMipmap* mip, int x, int y, int w, int h, FColor* palette, bool masked)
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

int D3D11TextureUploader_RGB10A2_LM::GetUploadSize(int x, int y, int w, int h)
{
	return w * h * 8;
}

void D3D11TextureUploader_RGB10A2_LM::UploadRect(void* dst, UnrealMipmap* mip, int x, int y, int w, int h, FColor* palette, bool masked)
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

int D3D11TextureUploader_Simple::GetUploadSize(int x, int y, int w, int h)
{
	return w * h * BytesPerPixel;
}

void D3D11TextureUploader_Simple::UploadRect(void* d, UnrealMipmap* mip, int x, int y, int w, int h, FColor* palette, bool masked)
{
	int pitch = mip->Width * BytesPerPixel;
	int size = w * BytesPerPixel;
	uint8_t* src = mip->Data.data() + x * BytesPerPixel + y * pitch;
	uint8_t* dst = (uint8_t*)d;
	for (int i = 0; i < h; i++)
	{
		memcpy(dst, src, size);
		dst += size;
		src += pitch;
	}
}

/////////////////////////////////////////////////////////////////////////////

int D3D11TextureUploader_4x4Block::GetUploadSize(int x, int y, int w, int h)
{
	int x0 = x / 4;
	int y0 = y / 4;
	int x1 = (x + w + 3) / 4;
	int y1 = (y + h + 3) / 4;
	return (x1 - x0) * (y1 - y0) * BytesPerBlock;
}

void D3D11TextureUploader_4x4Block::UploadRect(void* d, UnrealMipmap* mip, int x, int y, int w, int h, FColor* palette, bool masked)
{
	int x0 = x / 4;
	int y0 = y / 4;
	int x1 = (x + w + 3) / 4;
	int y1 = (y + h + 3) / 4;

	int pitch = (mip->Width + 3) / 4 * BytesPerBlock;
	int size = (x1 - x0) * BytesPerBlock;
	uint8_t* src = mip->Data.data() + x0 * BytesPerBlock + y0 * pitch;
	uint8_t* dst = (uint8_t*)d;
	for (int i = y0; i < y1; i++)
	{
		memcpy(dst, src, size);
		dst += size;
		src += pitch;
	}
}


/////////////////////////////////////////////////////////////////////////////

int D3D11TextureUploader_2DBlock::GetUploadSize(int x, int y, int w, int h)
{
	int x0 = x / BlockX;
	int y0 = y / BlockY;
	int x1 = (x + w + BlockX - 1) / BlockX;
	int y1 = (y + h + BlockY - 1) / BlockY;
	return (x1 - x0) * (y1 - y0) * BytesPerBlock;
}

void D3D11TextureUploader_2DBlock::UploadRect(void* d, UnrealMipmap* mip, int x, int y, int w, int h, FColor* palette, bool masked)
{
	int x0 = x / BlockX;
	int y0 = y / BlockY;
	int x1 = (x + w + BlockX - 1) / BlockX;
	int y1 = (y + h + BlockY - 1) / BlockY;

	int pitch = (mip->Width + BlockX - 1) / BlockX * BytesPerBlock;
	int size = (x1 - x0) * BytesPerBlock;
	uint8_t* src = mip->Data.data() + x0 * BytesPerBlock + y0 * pitch;
	uint8_t* dst = (uint8_t*)d;
	for (int i = y0; i < y1; i++)
	{
		memcpy(dst, src, size);
		dst += size;
		src += pitch;
	}
}
