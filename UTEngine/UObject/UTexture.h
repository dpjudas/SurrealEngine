#pragma once

#include "UObject.h"

class UPalette;
class USound;
class UActor;

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

	uint8_t& Format() { return Value<uint8_t>(PropOffsets_Bitmap.Format); }
	int& InternalTime() { return Value<int>(PropOffsets_Bitmap.InternalTime); }
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

class UTexture : public UBitmap
{
public:
	using UBitmap::UBitmap;
	void Load(ObjectStream* stream) override;

	virtual void Update() { }

	TextureFormat ActualFormat = TextureFormat::P8;
	std::vector<UnrealMipmap> Mipmaps;
	bool TextureModified = false;

	float& Accumulator() { return Value<float>(PropOffsets_Texture.Accumulator); }
	float& Alpha() { return Value<float>(PropOffsets_Texture.Alpha); }
	UTexture*& AnimCurrent() { return Value<UTexture*>(PropOffsets_Texture.AnimCurrent); }
	UTexture*& AnimNext() { return Value<UTexture*>(PropOffsets_Texture.AnimNext); }
	UTexture*& BumpMap() { return Value<UTexture*>(PropOffsets_Texture.BumpMap); }
	uint8_t& CompFormat() { return Value<uint8_t>(PropOffsets_Texture.CompFormat); }
	std::vector<void*>& CompMips() { return Value<std::vector<void*>>(PropOffsets_Texture.CompMips); } // native
	UTexture*& DetailTexture() { return Value<UTexture*>(PropOffsets_Texture.DetailTexture); }
	float& Diffuse() { return Value<float>(PropOffsets_Texture.Diffuse); }
	float& DrawScale() { return Value<float>(PropOffsets_Texture.DrawScale); }
	USound*& FootstepSound() { return Value<USound*>(PropOffsets_Texture.FootstepSound); }
	float& Friction() { return Value<float>(PropOffsets_Texture.Friction); }
	USound*& HitSound() { return Value<USound*>(PropOffsets_Texture.HitSound); }
	uint8_t& LODSet() { return Value<uint8_t>(PropOffsets_Texture.LODSet); }
	UTexture*& MacroTexture() { return Value<UTexture*>(PropOffsets_Texture.MacroTexture); }
	float& MaxFrameRate() { return Value<float>(PropOffsets_Texture.MaxFrameRate); }
	float& MinFrameRate() { return Value<float>(PropOffsets_Texture.MinFrameRate); }
	float& MipMult() { return Value<float>(PropOffsets_Texture.MipMult); }
	std::vector<void*>& Mips() { return Value<std::vector<void*>>(PropOffsets_Texture.Mips); } // native
	uint8_t& PrimeCount() { return Value<uint8_t>(PropOffsets_Texture.PrimeCount); }
	uint8_t& PrimeCurrent() { return Value<uint8_t>(PropOffsets_Texture.PrimeCurrent); }
	float& Specular() { return Value<float>(PropOffsets_Texture.Specular); }
	bool& bAutoUPan() { return Value<bool>(PropOffsets_Texture.bAutoUPan); }
	bool& bAutoVPan() { return Value<bool>(PropOffsets_Texture.bAutoVPan); }
	bool& bBigWavy() { return Value<bool>(PropOffsets_Texture.bBigWavy); }
	bool& bCloudWavy() { return Value<bool>(PropOffsets_Texture.bCloudWavy); }
	bool& bDirtyShadows() { return Value<bool>(PropOffsets_Texture.bDirtyShadows); }
	bool& bEnvironment() { return Value<bool>(PropOffsets_Texture.bEnvironment); }
	bool& bFakeBackdrop() { return Value<bool>(PropOffsets_Texture.bFakeBackdrop); }
	bool& bGouraud() { return Value<bool>(PropOffsets_Texture.bGouraud); }
	bool& bHasComp() { return Value<bool>(PropOffsets_Texture.bHasComp); }
	bool& bHighColorQuality() { return Value<bool>(PropOffsets_Texture.bHighColorQuality); }
	bool& bHighLedge() { return Value<bool>(PropOffsets_Texture.bHighLedge); }
	bool& bHighShadowDetail() { return Value<bool>(PropOffsets_Texture.bHighShadowDetail); }
	bool& bHighTextureQuality() { return Value<bool>(PropOffsets_Texture.bHighTextureQuality); }
	bool& bInvisible() { return Value<bool>(PropOffsets_Texture.bInvisible); }
	bool& bLowShadowDetail() { return Value<bool>(PropOffsets_Texture.bLowShadowDetail); }
	bool& bMasked() { return Value<bool>(PropOffsets_Texture.bMasked); }
	bool& bMirrored() { return Value<bool>(PropOffsets_Texture.bMirrored); }
	bool& bModulate() { return Value<bool>(PropOffsets_Texture.bModulate); }
	bool& bNoMerge() { return Value<bool>(PropOffsets_Texture.bNoMerge); }
	bool& bNoSmooth() { return Value<bool>(PropOffsets_Texture.bNoSmooth); }
	bool& bNotSolid() { return Value<bool>(PropOffsets_Texture.bNotSolid); }
	bool& bParametric() { return Value<bool>(PropOffsets_Texture.bParametric); }
	bool& bPortal() { return Value<bool>(PropOffsets_Texture.bPortal); }
	bool& bRealtime() { return Value<bool>(PropOffsets_Texture.bRealtime); }
	bool& bRealtimeChanged() { return Value<bool>(PropOffsets_Texture.bRealtimeChanged); }
	bool& bSemisolid() { return Value<bool>(PropOffsets_Texture.bSemisolid); }
	bool& bSmallWavy() { return Value<bool>(PropOffsets_Texture.bSmallWavy); }
	bool& bSpecialLit() { return Value<bool>(PropOffsets_Texture.bSpecialLit); }
	bool& bTransparent() { return Value<bool>(PropOffsets_Texture.bTransparent); }
	bool& bTwoSided() { return Value<bool>(PropOffsets_Texture.bTwoSided); }
	bool& bUnlit() { return Value<bool>(PropOffsets_Texture.bUnlit); }
	bool& bWaterWavy() { return Value<bool>(PropOffsets_Texture.bWaterWavy); }
	bool& bX2() { return Value<bool>(PropOffsets_Texture.bX2); }
	bool& bX3() { return Value<bool>(PropOffsets_Texture.bX3); }
	bool& bX4() { return Value<bool>(PropOffsets_Texture.bX4); }
	bool& bX5() { return Value<bool>(PropOffsets_Texture.bX5); }
	bool& bX6() { return Value<bool>(PropOffsets_Texture.bX6); }
	bool& bX7() { return Value<bool>(PropOffsets_Texture.bX7); }
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

	int& Junk1() { return Value<int>(PropOffsets_ScriptedTexture.Junk1); }
	int& Junk2() { return Value<int>(PropOffsets_ScriptedTexture.Junk2); }
	int& Junk3() { return Value<int>(PropOffsets_ScriptedTexture.Junk3); }
	float& LocalTime() { return Value<float>(PropOffsets_ScriptedTexture.LocalTime); }
	UActor*& NotifyActor() { return Value<UActor*>(PropOffsets_ScriptedTexture.NotifyActor); }
	UTexture*& SourceTexture() { return Value<UTexture*>(PropOffsets_ScriptedTexture.SourceTexture); }
};

class UPalette : public UObject
{
public:
	using UObject::UObject;
	void Load(ObjectStream* stream) override;

	std::vector<uint32_t> Colors;

	// std::vector<void*>& Colors() { return Value<std::vector<void*>>(PropOffsets_Palette.Colors); } // native
};
