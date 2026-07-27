#pragma once

#include "UXEmitter.h"

class UXTrailParticle;

enum class ETrailType : uint8_t
{
	TRAIL_Sheet,      // Single sheet trailer
	TRAIL_DoubleSheet // Cross shaped trailer
};

struct TrailOffsetPart
{
	vec3 Location, Velocity, Color, Accel;
	float LifeSpan[3], Scale, X;
};

class UXTrailEmitter : public UXEmitter
{
public:
	using UXEmitter::UXEmitter;

	BitfieldBool bDynamicParticleCount() { return BoolValue(PropOffsets_XTrailEmitter.bDynamicParticleCount); }
	BitfieldBool bSettingTrail() { return BoolValue(PropOffsets_XTrailEmitter.bSettingTrail); }
	BitfieldBool bSmoothEntryPoint() { return BoolValue(PropOffsets_XTrailEmitter.bSmoothEntryPoint); }
	BitfieldBool bTexContinous() { return BoolValue(PropOffsets_XTrailEmitter.bTexContinous); }
	float& MaxTrailLength() { return Value<float>(PropOffsets_XTrailEmitter.MaxTrailLength); }
	vec3*& OldTrailSport() { return Value<vec3*>(PropOffsets_XTrailEmitter.OldTrailSport); }
	UXTrailParticle*& ParticleData() { return Value<UXTrailParticle*>(PropOffsets_XTrailEmitter.ParticleData); }
	float& TexOffset() { return Value<float>(PropOffsets_XTrailEmitter.TexOffset); }
	FixedArrayView<float, 4> TextureUV() { return FixedArray<float, 4>(PropOffsets_XTrailEmitter.TextureUV); }
	TypedScriptArray<TrailOffsetPart> Trail() { return DynamicArray<TrailOffsetPart>(PropOffsets_XTrailEmitter.Trail); }
	float& TrailThreshold() { return Value<float>(PropOffsets_XTrailEmitter.TrailThreshold); }
	ETrailType TrailType() { return static_cast<ETrailType>(Value<uint8_t>(PropOffsets_XTrailEmitter.TrailType)); }
};
