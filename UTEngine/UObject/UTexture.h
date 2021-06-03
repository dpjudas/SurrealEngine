#pragma once

#include "UObject.h"

class UPalette;

class UnrealMipmap
{
public:
	int Width;
	int Height;
	std::vector<uint8_t> Data;
};

enum class TextureFormat : uint32_t
{
	P8 = 0x00,
	RGBA7 = 0x01,
	RGB16 = 0x02,
	DXT1 = 0x03,
	RGB8 = 0x04,
	RGBA8 = 0x05,
	BC2 = 0x06,
	BC3 = 0x07,
	BC1_PA = 0x1a,
	BC7 = 0x0c,
	BC6H_S = 0x0d,
	BC6H = 0x0e
};

class UBitmap : public UObject
{
public:
	using UObject::UObject;
};

class UTexture : public UBitmap
{
public:
	using UBitmap::UBitmap;
	void Load(ObjectStream* stream) override;

	virtual void Update() { }

	TextureFormat Format = TextureFormat::P8;
	std::vector<UnrealMipmap> Mipmaps;

	float DrawScale = 1.0f;

	UPalette* Palette = nullptr;
	UTexture* DetailTexture = nullptr;
	UTexture* MacroTexture = nullptr;

	bool TextureModified = false;
};

class UFractalTexture : public UTexture
{
public:
	using UTexture::UTexture;
	void Load(ObjectStream* stream) override;
};

class UFireTexture : public UFractalTexture
{
public:
	using UFractalTexture::UFractalTexture;

	void Update() override;
};

class UIceTexture : public UFractalTexture
{
public:
	using UFractalTexture::UFractalTexture;
};

class UWaterTexture : public UFractalTexture
{
public:
	using UFractalTexture::UFractalTexture;
};

class UWaveTexture : public UFractalTexture
{
public:
	using UFractalTexture::UFractalTexture;
};

class UScriptedTexture : public UTexture
{
public:
	using UTexture::UTexture;
};

class UPalette : public UObject
{
public:
	using UObject::UObject;
	void Load(ObjectStream* stream) override;

	std::vector<uint32_t> Colors;
};
