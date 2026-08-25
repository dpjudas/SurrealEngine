
#include "Precomp.h"
#include "UTexture.h"

void UTexture::Load(ObjectStream* stream)
{
	UBitmap::Load(stream);

	int mipsCount = stream->ReadUInt8();
	UncompressedMipmaps.resize(mipsCount);
	for (UnrealMipmap& mipmap : UncompressedMipmaps)
	{
		uint32_t widthoffset = 0;
		if (stream->GetVersion() >= 63)
			widthoffset = stream->ReadInt32();
		int bytes = stream->ReadIndex();
		mipmap.Data.resize(bytes);
		stream->ReadBytes(mipmap.Data.data(), bytes);
		mipmap.Width = stream->ReadUInt32();
		mipmap.Height = stream->ReadUInt32();
		mipmap.UBits = stream->ReadUInt8();
		mipmap.VBits = stream->ReadUInt8();
	}

	if (HasProperty("bHasComp") && GetBool("bHasComp"))
	{
		mipsCount = stream->ReadUInt8();
		CompressedMipmaps.resize(mipsCount);
		for (UnrealMipmap& mipmap : CompressedMipmaps)
		{
			uint32_t widthoffset = 0;
			if (stream->GetVersion() >= 68)
				widthoffset = stream->ReadInt32();
			int bytes = stream->ReadIndex();
			mipmap.Data.resize(bytes);
			stream->ReadBytes(mipmap.Data.data(), bytes);
			mipmap.Width = stream->ReadUInt32();
			mipmap.Height = stream->ReadUInt32();
			mipmap.UBits = stream->ReadUInt8();
			mipmap.VBits = stream->ReadUInt8();
		}

		UsedFormat = (TextureFormat)GetByte("CompFormat");
		UsedMipmaps = CompressedMipmaps;
	}
	else
	{
		UsedFormat = (TextureFormat)GetByte("Format");
		UsedMipmaps = UncompressedMipmaps;
	}
}

void UTexture::Save(PackageStreamWriter* stream)
{
	UBitmap::Save(stream);

	stream->WriteUInt8((uint8_t)UncompressedMipmaps.size());
	for (const UnrealMipmap& mipmap : UncompressedMipmaps)
	{
		if (stream->GetVersion() >= 63)
			stream->BeginSkipOffset();
		stream->WriteIndex((int)mipmap.Data.size());
		stream->WriteBytes(mipmap.Data.data(), (int)mipmap.Data.size());
		stream->WriteUInt32(mipmap.Width);
		stream->WriteUInt32(mipmap.Height);
		stream->WriteUInt8(mipmap.UBits);
		stream->WriteUInt8(mipmap.VBits);
		if (stream->GetVersion() >= 63)
			stream->EndSkipOffset();
	}

	if (HasProperty("bHasComp") && GetBool("bHasComp"))
	{
		stream->WriteUInt8((uint8_t)CompressedMipmaps.size());
		for (UnrealMipmap& mipmap : CompressedMipmaps)
		{
			if (stream->GetVersion() >= 68)
				stream->BeginSkipOffset();
			stream->WriteIndex((int)mipmap.Data.size());
			stream->WriteBytes(mipmap.Data.data(), (int)mipmap.Data.size());
			stream->WriteUInt32(mipmap.Width);
			stream->WriteUInt32(mipmap.Height);
			stream->WriteUInt8(mipmap.UBits);
			stream->WriteUInt8(mipmap.VBits);
			if (stream->GetVersion() >= 68)
				stream->EndSkipOffset();
		}
	}
}

void UTexture::Update(float elapsed)
{
	if (!Primed)
	{
		// Fire textures needs to run for a bit before showing them for the first time
		for (int i = 0, count = PrimeCount(); i < count; i++)
		{
			TextureModified = false;
			UpdateFrame();
		}
		PrimeCurrent() = PrimeCount();
		Primed = true;
	}

	float maxFrameRate = MaxFrameRate();
	if (maxFrameRate <= 0.0f)
		maxFrameRate = 25.0f; // Original game wasn't designed for 240 hz monitors!

	Accumulator() += elapsed;

	float animationSpeed = 1.0f / maxFrameRate;
	for (int iteration = 0; Accumulator() > animationSpeed; iteration++)
	{
		UpdateFrame();
		Accumulator() -= animationSpeed;
		if (iteration == 10)
		{
			Accumulator() = 0.0f;
			break;
		}
	}
}

void UTexture::UpdateFrame()
{
	// Loop textures
	UTexture* cur = AnimCurrent();
	if (!cur) cur = this;
	cur = cur->AnimNext();
	if (!cur) cur = this;
	AnimCurrent() = cur;
}
