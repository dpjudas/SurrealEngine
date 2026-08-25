#pragma once

#include "UBitmap.h"

class UTexture : public UBitmap
{
public:
	using UBitmap::UBitmap;

	void Load(ObjectStream* stream) override;
	void Save(PackageStreamWriter* stream) override;

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

	TextureFormat UsedFormat = TextureFormat::P8;
	Array<UnrealMipmap> UsedMipmaps, UncompressedMipmaps, CompressedMipmaps;
	bool TextureModified = false;
	int RealtimeChangeCount = 0;

	int FrameCounter = -1;

	bool Primed = false;

	uint32_t PolyFlags()
	{
		// UE1 has this as a native uint32_t that is declared in unreal script as bitfield booleans.
		BitfieldBool flagsBool = bInvisible();
		return *flagsBool.Ptr;
	}

	float& Accumulator() { return Value<float>(PropOffsets_Texture.Accumulator); }
	float& Alpha() { return Value<float>(PropOffsets_Texture.Alpha); }
	UTexture*& AnimCurrent() { return Value<UTexture*>(PropOffsets_Texture.AnimCurrent); }
	UTexture*& AnimNext() { return Value<UTexture*>(PropOffsets_Texture.AnimNext); }
	UTexture*& BumpMap() { return Value<UTexture*>(PropOffsets_Texture.BumpMap); }
	uint8_t& CompFormat() { return Value<uint8_t>(PropOffsets_Texture.CompFormat); }
	TypedScriptArray<void*> CompMips() { return DynamicArray<void*>(PropOffsets_Texture.CompMips); } // native
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
	TypedScriptArray<void*> Mips() { return DynamicArray<void*>(PropOffsets_Texture.Mips); } // native
	uint8_t& PrimeCount() { return Value<uint8_t>(PropOffsets_Texture.PrimeCount); }
	uint8_t& PrimeCurrent() { return Value<uint8_t>(PropOffsets_Texture.PrimeCurrent); }
	float& Specular() { return Value<float>(PropOffsets_Texture.Specular); }
	BitfieldBool bAutoUPan() { return BoolValue(PropOffsets_Texture.bAutoUPan); }
	BitfieldBool bAutoVPan() { return BoolValue(PropOffsets_Texture.bAutoVPan); }
	BitfieldBool bBigWavy() { return BoolValue(PropOffsets_Texture.bBigWavy); }
	BitfieldBool bCloudWavy() { return BoolValue(PropOffsets_Texture.bCloudWavy); }
	BitfieldBool bDirtyShadows() { return BoolValue(PropOffsets_Texture.bDirtyShadows); }
	BitfieldBool bEnvironment() { return BoolValue(PropOffsets_Texture.bEnvironment); }
	BitfieldBool bFakeBackdrop() { return BoolValue(PropOffsets_Texture.bFakeBackdrop); }
	BitfieldBool bGouraud() { return BoolValue(PropOffsets_Texture.bGouraud); }
	BitfieldBool bHasComp() { return BoolValue(PropOffsets_Texture.bHasComp); }
	BitfieldBool bHighColorQuality() { return BoolValue(PropOffsets_Texture.bHighColorQuality); }
	BitfieldBool bHighLedge() { return BoolValue(PropOffsets_Texture.bHighLedge); }
	BitfieldBool bHighShadowDetail() { return BoolValue(PropOffsets_Texture.bHighShadowDetail); }
	BitfieldBool bHighTextureQuality() { return BoolValue(PropOffsets_Texture.bHighTextureQuality); }
	BitfieldBool bInvisible() { return BoolValue(PropOffsets_Texture.bInvisible); }
	BitfieldBool bLowShadowDetail() { return BoolValue(PropOffsets_Texture.bLowShadowDetail); }
	BitfieldBool bMasked() { return BoolValue(PropOffsets_Texture.bMasked); }
	BitfieldBool bMirrored() { return BoolValue(PropOffsets_Texture.bMirrored); }
	BitfieldBool bModulate() { return BoolValue(PropOffsets_Texture.bModulate); }
	BitfieldBool bNoMerge() { return BoolValue(PropOffsets_Texture.bNoMerge); }
	BitfieldBool bNoSmooth() { return BoolValue(PropOffsets_Texture.bNoSmooth); }
	BitfieldBool bNotSolid() { return BoolValue(PropOffsets_Texture.bNotSolid); }
	BitfieldBool bParametric() { return BoolValue(PropOffsets_Texture.bParametric); }
	BitfieldBool bPortal() { return BoolValue(PropOffsets_Texture.bPortal); }
	BitfieldBool bRealtime() { return BoolValue(PropOffsets_Texture.bRealtime); }
	BitfieldBool bRealtimeChanged() { return BoolValue(PropOffsets_Texture.bRealtimeChanged); }
	BitfieldBool bSemisolid() { return BoolValue(PropOffsets_Texture.bSemisolid); }
	BitfieldBool bSmallWavy() { return BoolValue(PropOffsets_Texture.bSmallWavy); }
	BitfieldBool bSpecialLit() { return BoolValue(PropOffsets_Texture.bSpecialLit); }
	BitfieldBool bTransparent() { return BoolValue(PropOffsets_Texture.bTransparent); }
	BitfieldBool bTwoSided() { return BoolValue(PropOffsets_Texture.bTwoSided); }
	BitfieldBool bUnlit() { return BoolValue(PropOffsets_Texture.bUnlit); }
	BitfieldBool bWaterWavy() { return BoolValue(PropOffsets_Texture.bWaterWavy); }
	BitfieldBool bX2() { return BoolValue(PropOffsets_Texture.bX2); }
	BitfieldBool bX3() { return BoolValue(PropOffsets_Texture.bX3); }
	BitfieldBool bX4() { return BoolValue(PropOffsets_Texture.bX4); }
	BitfieldBool bX5() { return BoolValue(PropOffsets_Texture.bX5); }
	BitfieldBool bX6() { return BoolValue(PropOffsets_Texture.bX6); }
	BitfieldBool bX7() { return BoolValue(PropOffsets_Texture.bX7); }
};
