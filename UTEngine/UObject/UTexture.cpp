
#include "Precomp.h"
#include "UTexture.h"

void UTexture::Load(ObjectStream* stream)
{
	UBitmap::Load(stream);

	ActualFormat = (TextureFormat)GetByte("Format");

	int mipsCount = stream->ReadUInt8();
	Mipmaps.resize(mipsCount);

	for (UnrealMipmap& mipmap : Mipmaps)
	{
		uint32_t widthoffset = 0;
		if (stream->GetVersion() >= 63)
			widthoffset = stream->ReadInt32();
		int bytes = stream->ReadIndex();
		mipmap.Data.resize(bytes);
		stream->ReadBytes(mipmap.Data.data(), bytes);
		mipmap.Width = stream->ReadUInt32();
		mipmap.Height = stream->ReadUInt32();
		uint8_t UBits = stream->ReadUInt8();
		uint8_t VBits = stream->ReadUInt8();
	}

	if (GetBool("bHasComp"))
	{
		ActualFormat = (TextureFormat)GetByte("CompFormat");

		mipsCount = stream->ReadUInt8();
		Mipmaps.resize(mipsCount);
		for (UnrealMipmap& mipmap : Mipmaps)
		{
			uint32_t widthoffset = 0;
			if (stream->GetVersion() >= 68)
				widthoffset = stream->ReadInt32();
			int bytes = stream->ReadIndex();
			mipmap.Data.resize(bytes);
			stream->ReadBytes(mipmap.Data.data(), bytes);
			mipmap.Width = stream->ReadUInt32();
			mipmap.Height = stream->ReadUInt32();
			uint8_t UBits = stream->ReadUInt8();
			uint8_t VBits = stream->ReadUInt8();
		}
	}
}

/////////////////////////////////////////////////////////////////////////////

void UFractalTexture::Load(ObjectStream* stream)
{
	UTexture::Load(stream);

	ActualFormat = TextureFormat::P8;
	Mipmaps.resize(1);

	int width = GetInt("UClamp");
	int height = GetInt("VClamp");

	UnrealMipmap& mipmap = Mipmaps.front();
	mipmap.Width = width;
	mipmap.Height = height;
	mipmap.Data.resize((size_t)mipmap.Width * mipmap.Height);
	uint8_t* pixels = (uint8_t*)mipmap.Data.data();
	memset(pixels, 0, (size_t)width * height);
}

/////////////////////////////////////////////////////////////////////////////

void UFireTexture::Update()
{
	if (!TextureModified)
	{
		UnrealMipmap& mipmap = Mipmaps.front();

		int width = mipmap.Width;
		int height = mipmap.Height;
		uint8_t* pixels = (uint8_t*)mipmap.Data.data();

		uint8_t* line = pixels + (size_t)width * (height - 1);
		for (int x = 0; x < width; x++)
		{
			line[x] = (uint8_t)clamp(std::max(std::cos(radians(90.0f + (x - width / 2) * 200.0f / width)), 0.0f) * 100 + rand() * 155 / RAND_MAX, 0.0f, 255.0f);
		}

		uint8_t* src = pixels;
		for (int y = 0; y < height - 1; y++)
		{
			uint8_t* dest = src;
			src += width;

			for (int x = 0; x < width; x++)
			{
				int value = src[x];
				for (int i = -3; i < 3; i++)
				{
					int xx = (x + i) % width;
					if (xx < 0) xx += width;
					value += src[xx];
				}
				value -= 8;
				value = value / 7;
				if (value < 0) value = 0;
				dest[x] = value;
			}
		}

		TextureModified = true;
	}
}

/////////////////////////////////////////////////////////////////////////////

void UPalette::Load(ObjectStream* stream)
{
	UObject::Load(stream);
	int count = stream->ReadIndex();
	Colors.resize(count);
	stream->ReadBytes(Colors.data(), count * 4);
}
