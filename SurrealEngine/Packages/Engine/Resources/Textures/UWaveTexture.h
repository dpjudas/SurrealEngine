#pragma once

#include "UWaterTexture.h"

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
