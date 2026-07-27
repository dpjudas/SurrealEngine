#pragma once

#include "Packages/Core/UObject.h"

class UPalette;
class USound;
class UActor;
class UFont;

class UnrealMipmap
{
public:
	int Width = 0;
	int Height = 0;
	int UBits = 0;
	int VBits = 0;
	Array<uint8_t> Data;
};

enum class TextureFormat : uint32_t
{
	P8 = 0x00,
	BGRA8_LM = 0x01,
	R5G6B5 = 0x02,
	BC1 = 0x03,
	RGB8 = 0x04,
	BGRA8 = 0x05,
	BC2 = 0x06,
	BC3 = 0x07,
	BC4 = 0x08,
	BC4_S = 0x09,
	BC5 = 0x0a,
	BC5_S = 0x0b,
	BC7 = 0x0c,
	BC6H_S = 0x0d,
	BC6H = 0x0e,
	RGBA16 = 0x0f,
	RGBA16_S = 0x10,
	RGBA32 = 0x11,
	RGBA32_S = 0x12,
	NODATA = 0x13,
	UNCOMPRESSED = 0x14,
	UNCOMPRESSED_LOW = 0x15,
	UNCOMPRESSED_HIGH = 0x16,
	COMPRESSED = 0x17,
	COMPRESSED_LOW = 0x18,
	COMPRESSED_HIGH = 0x19,
	BC1_PA = 0x1a,
	R8 = 0x1b,
	R8_S = 0x1c,
	R16 = 0x1d,
	R16_S = 0x1e,
	R32 = 0x1f,
	R32_S = 0x20,
	RG8 = 0x21,
	RG8_S = 0x22,
	RG16 = 0x23,
	RG16_S = 0x24,
	RG32 = 0x25,
	RG32_S = 0x26,
	RGB8_S = 0x27,
	RGB16_ = 0x28,
	RGB16_S = 0x29,
	RGB32 = 0x2a,
	RGB32_S = 0x2b,
	RGBA8_ = 0x2c,
	RGBA8_S = 0x2d,
	R16_F = 0x2e,
	R32_F = 0x2f,
	RG16_F = 0x30,
	RG32_F = 0x31,
	RGB16_F = 0x32,
	RGB32_F = 0x33,
	RGBA16_F = 0x34,
	RGBA32_F = 0x35,
	ETC1 = 0x36,
	ETC2 = 0x37,
	ETC2_PA = 0x38,
	ETC2_RGB_EAC_A = 0x39,
	EAC_R = 0x40,
	EAC_R_S = 0x41,
	EAC_RG = 0x42,
	EAC_RG_S = 0x43,
	ASTC_4x4 = 0x44,
	ASTC_5x4 = 0x45,
	ASTC_5x5 = 0x46,
	ASTC_6x5 = 0x47,
	ASTC_6x6 = 0x48,
	ASTC_8x5 = 0x49,
	ASTC_8x6 = 0x4a,
	ASTC_8x8 = 0x4b,
	ASTC_10x5 = 0x4c,
	ASTC_10x6 = 0x4d,
	ASTC_10x8 = 0x4e,
	ASTC_10x10 = 0x4f,
	ASTC_12x10 = 0x50,
	ASTC_12x12 = 0x51,
	ASTC_3x3x3 = 0x52,
	ASTC_4x3x3 = 0x53,
	ASTC_4x4x3 = 0x54,
	ASTC_4x4x4 = 0x55,
	ASTC_5x4x4 = 0x56,
	ASTC_5x5x4 = 0x57,
	ASTC_5x5x5 = 0x58,
	ASTC_6x5x5 = 0x59,
	ASTC_6x6x5 = 0x60,
	ASTC_6x6x6 = 0x6a,
	PVRTC1_2BPP = 0x6b,
	PVRTC1_4BPP = 0x6c,
	PVRTC2_2BPP = 0x6d,
	PVRTC2_4BPP = 0x6e,
	R8_UI = 0x6f,
	R8_I = 0x70,
	R16_UI = 0x71,
	R16_I = 0x72,
	R32_UI = 0x73,
	R32_I = 0x74,
	RG8_UI = 0x75,
	RG8_I = 0x76,
	RG16_UI = 0x77,
	RG16_I = 0x78,
	RG32_UI = 0x79,
	RG32_I = 0x7a,
	RGB8_UI = 0x7b,
	RGB8_I = 0x7c,
	RGB16_UI = 0x7d,
	RGB16_I = 0x7e,
	RGB32_UI = 0x7f,
	RGB32_I = 0x80,
	RGBA8_UI = 0x81,
	RGBA8_I = 0x82,
	RGBA16_UI = 0x83,
	RGBA16_I = 0x84,
	RGBA32_UI = 0x85,
	RGBA32_I = 0x86,
	ARGB8 = 0x87,
	ABGR8 = 0x88,
	RGB10A2 = 0x89,
	RGB10A2_UI = 0x8a,
	RGB10A2_LM = 0x8b,
	RGB9E5 = 0x8c,
	P8_RGB9E5 = 0x8d,
	R1 = 0x8e,
	RGB10A2_S = 0x8f,
	RGB10A2_I = 0x90,
	R11G11B10_F = 0x91,
	B5G6R5 = 0x92,
	BGR8 = 0x93,
	R64_F = 0x94,
	RG64_F = 0x95,
	RGB64_F = 0x96,
	RGBA64_F = 0x97
};

class UBitmap : public UObject
{
public:
	using UObject::UObject;

	uint8_t& Format() { return Value<uint8_t>(PropOffsets_Bitmap.Format); }
	FixedArrayView<int, 2> InternalTime() { return FixedArray<int, 2>(PropOffsets_Bitmap.InternalTime); }
	Color& MaxColor() { return Value<Color>(PropOffsets_Bitmap.MaxColor); }
	Color& MipZero() { return Value<Color>(PropOffsets_Bitmap.MipZero); }
	UPalette*& Palette() { return Value<UPalette*>(PropOffsets_Bitmap.Palette); }
	uint8_t& UBits() { return Value<uint8_t>(PropOffsets_Bitmap.UBits); }
	int& UClamp() { return Value<int>(PropOffsets_Bitmap.UClamp); }
	int& USize() { return Value<int>(PropOffsets_Bitmap.USize); }
	uint8_t& VBits() { return Value<uint8_t>(PropOffsets_Bitmap.VBits); }
	int& VClamp() { return Value<int>(PropOffsets_Bitmap.VClamp); }
	int& VSize() { return Value<int>(PropOffsets_Bitmap.VSize); }
};
