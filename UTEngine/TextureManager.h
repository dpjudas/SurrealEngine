#pragma once

#include "Package.h"

class PackageManager;
class ObjectInfo;

class UnrealPalette
{
public:
	std::vector<uint32_t> Colors;
};

class UnrealMipmap
{
public:
	int Width;
	int Height;
	std::vector<uint8_t> Data;
};

enum ETextureFormat
{
	TEXF_P8 = 0x00,
	TEXF_RGBA7 = 0x01,
	TEXF_RGB16 = 0x02,
	TEXF_DXT1 = 0x03,
	TEXF_RGB8 = 0x04,
	TEXF_RGBA8 = 0x05
};

class UnrealTexture
{
public:
	std::vector<UnrealProperty> Properties;

	ETextureFormat Format = TEXF_P8;
	std::vector<UnrealMipmap> Mipmaps;

	float DrawScale = 1.0f;

	UnrealPalette* Palette = nullptr;
	UnrealTexture* DetailTexture = nullptr;
	UnrealTexture* MacroTexture = nullptr;
};

class TextureManager
{
public:
	TextureManager(PackageManager* packages);

	UnrealTexture* GetTexture(ExportTableEntry* entry);
	UnrealPalette* GetPalette(ExportTableEntry* entry);

private:
	PackageManager* Packages = nullptr;
	std::map<ExportTableEntry*, std::unique_ptr<UnrealPalette>> Palettes;
	std::map<ExportTableEntry*, std::unique_ptr<UnrealTexture>> Textures;
};
