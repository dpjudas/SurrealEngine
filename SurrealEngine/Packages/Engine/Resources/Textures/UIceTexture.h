#pragma once

#include "UFractalTexture.h"

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
	BitfieldBool MoveIce() { return BoolValue(PropOffsets_IceTexture.MoveIce); }
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
