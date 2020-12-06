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

class UTexture : public UObject
{
public:
	UTexture() = default;
	UTexture(ObjectStream* stream);

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
	UFractalTexture(ObjectStream* stream);
};

class UFireTexture : public UFractalTexture
{
public:
	UFireTexture(ObjectStream* stream);

	void Update() override;
};

class UIceTexture : public UFractalTexture
{
public:
	UIceTexture(ObjectStream* stream) : UFractalTexture(stream) { }
};

class UWaterTexture : public UFractalTexture
{
public:
	UWaterTexture(ObjectStream* stream) : UFractalTexture(stream) { }
};

class UWaveTexture : public UFractalTexture
{
public:
	UWaveTexture(ObjectStream* stream) : UFractalTexture(stream) { }
};

class UScriptedTexture : public UTexture
{
public:
	UScriptedTexture(ObjectStream* stream) : UTexture(stream) { }
};

class UPalette : public UObject
{
public:
	UPalette(ObjectStream* stream);

	std::vector<uint32_t> Colors;
};
