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

	UTexture* GetAnimTexture() { return AnimCurrent() ? AnimCurrent() : this; }

	int GetAnimTextureCount()
	{
		int count = 1;
		UTexture* cur = AnimNext();
		while (cur && cur != this)
		{
			cur = cur->AnimNext();
			count++;
		}
		return count;
	}

	virtual void Update(float elapsed);
	virtual void UpdateFrame();

	TextureFormat ActualFormat = TextureFormat::P8;
	std::vector<UnrealMipmap> Mipmaps;
	bool TextureModified = false;
	int RealtimeChangeCount = 0;

	int FrameCounter = -1;

	uint32_t PolyFlags()
	{
		// To do: implement packed booleans in the VM so that this can be done as a single uint32_t
		uint32_t flags = ((uint32_t)bInvisible()) << 0;
		flags |= ((uint32_t)bMasked()) << 1;
		flags |= ((uint32_t)bTransparent()) << 2;
		flags |= ((uint32_t)bNotSolid()) << 3;
		flags |= ((uint32_t)bEnvironment()) << 4;
		flags |= ((uint32_t)bSemisolid()) << 5;
		flags |= ((uint32_t)bModulate()) << 6;
		flags |= ((uint32_t)bFakeBackdrop()) << 7;
		flags |= ((uint32_t)bTwoSided()) << 8;
		flags |= ((uint32_t)bAutoUPan()) << 9;
		flags |= ((uint32_t)bAutoVPan()) << 10;
		flags |= ((uint32_t)bNoSmooth()) << 11;
		flags |= ((uint32_t)bBigWavy()) << 12;
		flags |= ((uint32_t)bSmallWavy()) << 13;
		flags |= ((uint32_t)bWaterWavy()) << 14;
		flags |= ((uint32_t)bLowShadowDetail()) << 15;
		flags |= ((uint32_t)bNoMerge()) << 16;
		flags |= ((uint32_t)bCloudWavy()) << 17;
		flags |= ((uint32_t)bDirtyShadows()) << 18;
		flags |= ((uint32_t)bHighLedge()) << 19;
		flags |= ((uint32_t)bSpecialLit()) << 20;
		flags |= ((uint32_t)bGouraud()) << 21;
		flags |= ((uint32_t)bUnlit()) << 22;
		flags |= ((uint32_t)bHighShadowDetail()) << 23;
		flags |= ((uint32_t)bPortal()) << 24;
		flags |= ((uint32_t)bMirrored()) << 25;
		flags |= ((uint32_t)bX2()) << 26;
		flags |= ((uint32_t)bX3()) << 27;
		flags |= ((uint32_t)bX4()) << 28;
		flags |= ((uint32_t)bX5()) << 29;
		flags |= ((uint32_t)bX6()) << 30;
		flags |= ((uint32_t)bX7()) << 31;
		return flags;
	}

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

	uint8_t& AuxPhase() { return Value<uint8_t>(PropOffsets_FractalTexture.AuxPhase); }
	uint8_t& DrawPhase() { return Value<uint8_t>(PropOffsets_FractalTexture.DrawPhase); }
	int& GlobalPhase() { return Value<int>(PropOffsets_FractalTexture.GlobalPhase); }
	int& LightOutput() { return Value<int>(PropOffsets_FractalTexture.LightOutput); }
	int& SoundOutput() { return Value<int>(PropOffsets_FractalTexture.SoundOutput); }
	int& UMask() { return Value<int>(PropOffsets_FractalTexture.UMask); }
	int& VMask() { return Value<int>(PropOffsets_FractalTexture.VMask); }
};

enum class ESpark : uint8_t
{
	Burn,
	Sparkle,
	Pulse,
	Signal,
	Blaze,
	OzHasSpoken,
	Cone,
	BlazeRight,
	BlazeLeft,
	Cylinder,
	Cylinder3D,
	Lissajous,
	Jugglers,
	Emit,
	Fountain,
	Flocks,
	Eels,
	Organic,
	WanderOrganic,
	RandmCloud,
	CustomCloud,
	LocalCloud,
	Stars,
	LineLightning,
	RampLightning,
	SphereLightning,
	Wheel,
	Gametes,
	Sprinkler
};

enum class FireDrawMode : uint8_t
{
	Normal,
	Lathe,
	Lathe_2,
	Lathe_3,
	Lathe_4,
};

struct Spark
{
	ESpark Type;
	uint8_t Heat;
	uint8_t X;
	uint8_t Y;
	union
	{
		struct { uint8_t ByteA, ByteB, ByteC, ByteD; };
		struct { uint8_t A, B, C, D; } Burn, OzHasSpoken;
		struct { uint8_t Angle, TwirlAge, RotSpeed, TwirlRotSpeed; } Wheel;
		struct { uint8_t SpeedX, SpeedY, C, HeatDecay; } Emit;
		struct { uint8_t A, B, C, HeatDecay; } Blaze;
		struct { uint8_t A, B, Radius, Frequency; } SphereLightning;
	};
};

enum class SparkParticleType
{
	Twirl,
	Drift
};

struct SparkParticle
{
	SparkParticleType Type;
	union
	{
		struct { float X, Y, Angle, RotSpeed; uint8_t Heat, Age; } Twirl;
		struct { float X, Y, SpeedX, SpeedY; int Heat, HeatDecay; } Drift;
	};
};

class UFireTexture : public UFractalTexture
{
public:
	using UFractalTexture::UFractalTexture;

	void Load(ObjectStream* stream) override;
	void UpdateFrame() override;

	FireDrawMode& DrawMode() { return Value<FireDrawMode>(PropOffsets_FireTexture.DrawMode); }
	uint8_t& FX_Area() { return Value<uint8_t>(PropOffsets_FireTexture.FX_Area); }
	uint8_t& FX_AuxSize() { return Value<uint8_t>(PropOffsets_FireTexture.FX_AuxSize); }
	uint8_t& FX_Frequency() { return Value<uint8_t>(PropOffsets_FireTexture.FX_Frequency); }
	uint8_t& FX_Heat() { return Value<uint8_t>(PropOffsets_FireTexture.FX_Heat); }
	uint8_t& FX_HorizSpeed() { return Value<uint8_t>(PropOffsets_FireTexture.FX_HorizSpeed); }
	uint8_t& FX_Phase() { return Value<uint8_t>(PropOffsets_FireTexture.FX_Phase); }
	uint8_t& FX_Size() { return Value<uint8_t>(PropOffsets_FireTexture.FX_Size); }
	uint8_t& FX_VertSpeed() { return Value<uint8_t>(PropOffsets_FireTexture.FX_VertSpeed); }
	int& NumSparks() { return Value<int>(PropOffsets_FireTexture.NumSparks); }
	int& OldRenderHeat() { return Value<int>(PropOffsets_FireTexture.OldRenderHeat); }
	uint8_t& PenDownX() { return Value<uint8_t>(PropOffsets_FireTexture.PenDownX); }
	uint8_t& PenDownY() { return Value<uint8_t>(PropOffsets_FireTexture.PenDownY); }
	uint8_t& RenderHeat() { return Value<uint8_t>(PropOffsets_FireTexture.RenderHeat); }
	uint8_t& RenderTable() { return Value<uint8_t>(PropOffsets_FireTexture.RenderTable); }
	uint8_t& SparkType() { return Value<uint8_t>(PropOffsets_FireTexture.SparkType); }
	//std::vector<Spark*>& Sparks() { return Value<std::vector<Spark*>>(PropOffsets_FireTexture.Sparks); }
	int& SparksLimit() { return Value<int>(PropOffsets_FireTexture.SparksLimit); }
	uint8_t& StarStatus() { return Value<uint8_t>(PropOffsets_FireTexture.StarStatus); }
	bool& bRising() { return Value<bool>(PropOffsets_FireTexture.bRising); }

private:
	int RandomByteValue() { return (int)(rand() * 256LL / (RAND_MAX + 1LL)); }

	std::vector<uint8_t> WorkBuffer;
	uint8_t FadeTable[4 * 256];
	int CurrentRenderHeat = -1;
	std::vector<Spark> Sparks;
	std::vector<SparkParticle> Particles;
};

class UIceTexture : public UFractalTexture
{
public:
	using UFractalTexture::UFractalTexture;

	void UpdateFrame() override;

	uint8_t& Amplitude() { return Value<uint8_t>(PropOffsets_IceTexture.Amplitude); }
	int& ForceRefresh() { return Value<int>(PropOffsets_IceTexture.ForceRefresh); }
	uint8_t& Frequency() { return Value<uint8_t>(PropOffsets_IceTexture.Frequency); }
	UTexture*& GlassTexture() { return Value<UTexture*>(PropOffsets_IceTexture.GlassTexture); }
	uint8_t& HorizPanSpeed() { return Value<uint8_t>(PropOffsets_IceTexture.HorizPanSpeed); }
	int& LocalSource() { return Value<int>(PropOffsets_IceTexture.LocalSource); }
	float& MasterCount() { return Value<float>(PropOffsets_IceTexture.MasterCount); }
	bool& MoveIce() { return Value<bool>(PropOffsets_IceTexture.MoveIce); }
	UTexture*& OldGlassTex() { return Value<UTexture*>(PropOffsets_IceTexture.OldGlassTex); }
	UTexture*& OldSourceTex() { return Value<UTexture*>(PropOffsets_IceTexture.OldSourceTex); }
	int& OldUDisplace() { return Value<int>(PropOffsets_IceTexture.OldUDisplace); }
	int& OldVDisplace() { return Value<int>(PropOffsets_IceTexture.OldVDisplace); }
	uint8_t& PanningStyle() { return Value<uint8_t>(PropOffsets_IceTexture.PanningStyle); }
	UTexture*& SourceTexture() { return Value<UTexture*>(PropOffsets_IceTexture.SourceTexture); }
	float& TickAccu() { return Value<float>(PropOffsets_IceTexture.TickAccu); }
	uint8_t& TimeMethod() { return Value<uint8_t>(PropOffsets_IceTexture.TimeMethod); }
	float& UDisplace() { return Value<float>(PropOffsets_IceTexture.UDisplace); }
	float& UPosition() { return Value<float>(PropOffsets_IceTexture.UPosition); }
	float& VDisplace() { return Value<float>(PropOffsets_IceTexture.VDisplace); }
	float& VPosition() { return Value<float>(PropOffsets_IceTexture.VPosition); }
	uint8_t& VertPanSpeed() { return Value<uint8_t>(PropOffsets_IceTexture.VertPanSpeed); }
};

enum class ADropType : uint8_t
{
	FixedDepth,
	PhaseSpot,
	ShallowSpot,
	HalfAmpl,
	RandomMover,
	FixedRandomSpot,
	WhirlyThing,
	BigWhirly,
	HorizontalLine,
	VerticalLine,
	DiagonalLine1,
	DiagonalLine2,
	HorizontalOsc,
	VerticalOsc,
	DiagonalOsc1,
	DiagonalOsc2,
	RainDrops,
	AreaClamp,
	LeakyTap,
	DrippyTap
};

struct ADrop
{
	ADropType Type;
	uint8_t Depth;
	uint8_t X;
	uint8_t Y;
	uint8_t ByteA,ByteB, ByteC, ByteD;
};

struct WaterPixel
{
	float Pressure = 0.0f;
	float Velocity = 0.0f;
	float XGradient = 0.0f;
	float YGradient = 0.0f;
};

class UWaterTexture : public UFractalTexture
{
public:
	using UFractalTexture::UFractalTexture;

	void UpdateFrame() override;

	uint8_t& DropType() { return Value<uint8_t>(PropOffsets_WaterTexture.DropType); }
	ADrop* Drops() { return FixedArray<ADrop>(PropOffsets_WaterTexture.Drops); }
	// ADrop& Drops() { return Value<ADrop>(PropOffsets_WaterTexture.Drops); }
	uint8_t& FX_Amplitude() { return Value<uint8_t>(PropOffsets_WaterTexture.FX_Amplitude); }
	uint8_t& FX_Depth() { return Value<uint8_t>(PropOffsets_WaterTexture.FX_Depth); }
	uint8_t& FX_Frequency() { return Value<uint8_t>(PropOffsets_WaterTexture.FX_Frequency); }
	uint8_t& FX_Phase() { return Value<uint8_t>(PropOffsets_WaterTexture.FX_Phase); }
	uint8_t& FX_Radius() { return Value<uint8_t>(PropOffsets_WaterTexture.FX_Radius); }
	uint8_t& FX_Size() { return Value<uint8_t>(PropOffsets_WaterTexture.FX_Size); }
	uint8_t& FX_Speed() { return Value<uint8_t>(PropOffsets_WaterTexture.FX_Speed); }
	uint8_t& FX_Time() { return Value<uint8_t>(PropOffsets_WaterTexture.FX_Time); }
	int& NumDrops() { return Value<int>(PropOffsets_WaterTexture.NumDrops); }
	int& OldWaveAmp() { return Value<int>(PropOffsets_WaterTexture.OldWaveAmp); }
	uint8_t& RenderTable() { return Value<uint8_t>(PropOffsets_WaterTexture.RenderTable); }
	int& SourceFields() { return Value<int>(PropOffsets_WaterTexture.SourceFields); }
	uint8_t& WaterParity() { return Value<uint8_t>(PropOffsets_WaterTexture.WaterParity); }
	uint8_t& WaterTable() { return Value<uint8_t>(PropOffsets_WaterTexture.WaterTable); }
	uint8_t& WaveAmp() { return Value<uint8_t>(PropOffsets_WaterTexture.WaveAmp); }

protected:
	void UpdateWater();

	std::vector<WaterPixel> WaterDepth[2];
	int CurrentWaterDepth = 0;
};

class UWaveTexture : public UWaterTexture
{
public:
	using UWaterTexture::UWaterTexture;

	void UpdateFrame() override;

	uint8_t& BumpMapAngle() { return Value<uint8_t>(PropOffsets_WaveTexture.BumpMapAngle); }
	uint8_t& BumpMapLight() { return Value<uint8_t>(PropOffsets_WaveTexture.BumpMapLight); }
	uint8_t& PhongRange() { return Value<uint8_t>(PropOffsets_WaveTexture.PhongRange); }
	uint8_t& PhongSize() { return Value<uint8_t>(PropOffsets_WaveTexture.PhongSize); }
};

class UWetTexture : public UWaterTexture
{
public:
	using UWaterTexture::UWaterTexture;

	void UpdateFrame() override;

	int& LocalSourceBitmap() { return Value<int>(PropOffsets_WetTexture.LocalSourceBitmap); }
	UTexture*& OldSourceTex() { return Value<UTexture*>(PropOffsets_WetTexture.OldSourceTex); }
	UTexture*& SourceTexture() { return Value<UTexture*>(PropOffsets_WetTexture.SourceTexture); }
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
