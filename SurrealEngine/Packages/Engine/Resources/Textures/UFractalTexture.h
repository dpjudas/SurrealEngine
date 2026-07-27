#pragma once

#include "UTexture.h"

class UFractalTexture : public UTexture
{
public:
	using UTexture::UTexture;

	void Load(ObjectStream* stream) override;
	void Save(PackageStreamWriter* stream) override;

	uint8_t& AuxPhase() { return Value<uint8_t>(PropOffsets_FractalTexture.AuxPhase); }
	uint8_t& DrawPhase() { return Value<uint8_t>(PropOffsets_FractalTexture.DrawPhase); }
	int& GlobalPhase() { return Value<int>(PropOffsets_FractalTexture.GlobalPhase); }
	int& LightOutput() { return Value<int>(PropOffsets_FractalTexture.LightOutput); }
	int& SoundOutput() { return Value<int>(PropOffsets_FractalTexture.SoundOutput); }
	int& UMask() { return Value<int>(PropOffsets_FractalTexture.UMask); }
	int& VMask() { return Value<int>(PropOffsets_FractalTexture.VMask); }
};
