#pragma once

#include "UXEmitter.h"

enum class ESprPartRotType : uint8_t
{
	SPR_DesiredRot,
	SPR_RelFacingVelocity,
	SPR_AbsFacingVelocity,
	SPR_RelFacingNormal,
	SPR_AbsFacingNormal
};

class UXSpriteEmitter : public UXEmitter
{
public:
	using UXEmitter::UXEmitter;

	RangeVector*& InitialRot() { return Value<RangeVector*>(PropOffsets_XSpriteEmitter.InitialRot); }
	ESprPartRotType ParticleRotation() { return static_cast<ESprPartRotType>(Value<uint8_t>(PropOffsets_XSpriteEmitter.ParticleRotation)); }
	RangeVector*& RotationsPerSec() { return Value<RangeVector*>(PropOffsets_XSpriteEmitter.RotationsPerSec); }
	float& RotateByVelocityScale() { return Value<float>(PropOffsets_XSpriteEmitter.RotateByVelocityScale); }
	vec3*& RotNormal() { return Value<vec3*>(PropOffsets_XSpriteEmitter.RotNormal); }
};
