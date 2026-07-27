#pragma once

#include "UFractalTexture.h"

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
	uint8_t ByteA, ByteB, ByteC, ByteD;
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
	FixedArrayView<ADrop, 256> Drops() { return FixedArray<ADrop, 256>(PropOffsets_WaterTexture.Drops); }
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
	FixedArrayView<uint8_t, 1028> RenderTable() { return FixedArray<uint8_t, 1028>(PropOffsets_WaterTexture.RenderTable); }
	int& SourceFields() { return Value<int>(PropOffsets_WaterTexture.SourceFields); }
	uint8_t& WaterParity() { return Value<uint8_t>(PropOffsets_WaterTexture.WaterParity); }
	FixedArrayView<uint8_t, 1536> WaterTable() { return FixedArray<uint8_t, 1536>(PropOffsets_WaterTexture.WaterTable); }
	uint8_t& WaveAmp() { return Value<uint8_t>(PropOffsets_WaterTexture.WaveAmp); }

protected:
	void UpdateWater();

	Array<WaterPixel> WaterDepth[2];
	int CurrentWaterDepth = 0;
};
