#pragma once

#include "UXEmitter.h"

enum class EBeamTargetType : uint8_t
{
	BEAM_Velocity,
	BEAM_BeamActor,
	BEAM_Offset,
	BEAM_OffsetAsAbsolute
};

struct FBeamTargetPoint
{
	UActor* TargetActor;
	vec3* Offset;
};

class UXBeamEmitter : public UXEmitter
{
public:
	using UXEmitter::UXEmitter;

	BitfieldBool bDoBeamNoise() { return BoolValue(PropOffsets_XBeamEmitter.bDoBeamNoise); }
	BitfieldBool bDynamicNoise() { return BoolValue(PropOffsets_XBeamEmitter.bDynamicNoise); }
	TypedScriptArray<float> BeamPointScaling() { return DynamicArray<float>(PropOffsets_XBeamEmitter.BeamPointScaling); }
	TypedScriptArray<FBeamTargetPoint> BeamTarget() { return DynamicArray<FBeamTargetPoint>(PropOffsets_XBeamEmitter.BeamTarget); }
	UTexture*& EndTexture() { return Value<UTexture*>(PropOffsets_XBeamEmitter.EndTexture); }
	RangeVector*& NoiseRange() { return Value<RangeVector*>(PropOffsets_XBeamEmitter.NoiseRange); }
	float& NoiseSwapTime() { return Value<float>(PropOffsets_XBeamEmitter.NoiseSwapTime); }
	TypedScriptArray<ScaleRangeType> NoiseTimeScale() { return DynamicArray<ScaleRangeType>(PropOffsets_XBeamEmitter.NoiseTimeScale); }
	UMesh*& RenderDataModel() { return Value<UMesh*>(PropOffsets_XBeamEmitter.RenderDataModel); }
	uint8_t& Segments() { return Value<uint8_t>(PropOffsets_XBeamEmitter.Segments); }
	TypedScriptArray<float> SegmentScales() { return DynamicArray<float>(PropOffsets_XBeamEmitter.SegmentScales); }
	UTexture*& StartTexture() { return Value<UTexture*>(PropOffsets_XBeamEmitter.StartTexture); }
	FixedArrayView<float, 4> TextureUV() { return FixedArray<float, 4>(PropOffsets_XBeamEmitter.TextureUV); }
	float& TurnRate() { return Value<float>(PropOffsets_XBeamEmitter.TurnRate); }
};
