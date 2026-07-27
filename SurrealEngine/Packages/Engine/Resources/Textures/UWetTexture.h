#pragma once

#include "UWaterTexture.h"

class UWetTexture : public UWaterTexture
{
public:
	using UWaterTexture::UWaterTexture;

	void UpdateFrame() override;

	int& LocalSourceBitmap() { return Value<int>(PropOffsets_WetTexture.LocalSourceBitmap); }
	UTexture*& OldSourceTex() { return Value<UTexture*>(PropOffsets_WetTexture.OldSourceTex); }
	UTexture*& SourceTexture() { return Value<UTexture*>(PropOffsets_WetTexture.SourceTexture); }
};
