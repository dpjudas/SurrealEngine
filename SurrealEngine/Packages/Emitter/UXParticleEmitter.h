#pragma once

#include "Packages/Engine/Actors/UActor.h"

class USound;
class UXEmitter;

enum class EHitEventType : uint8_t
{
	HIT_DoNothing,
	HIT_Destroy,
	HIT_StopMovement,
	HIT_Bounce,
	HIT_Script
};

struct IntRange
{
	int Min, Max;
};

struct ByteRange
{
	uint8_t Min, Max;
};

struct FloatRange
{
	float Min, Max;
};

struct RangeVector
{
	FloatRange X, Y, Z;
};

struct ParticleSndType
{
	USound* Sounds[8];
	FloatRange SndPitch, SndRadius, SndVolume;
	uint8_t SndCount;
};

class UXParticleEmitter : public UActor
{
public:
	using UActor::UActor;

	int& ActiveCount() { return Value<int>(PropOffsets_XParticleEmitter.ActiveCount); }
	BitfieldBool bHasInitilized() { return BoolValue(PropOffsets_XParticleEmitter.bHasInitilized); }
	BitfieldBool bKillNextTick() { return BoolValue(PropOffsets_XParticleEmitter.bKillNextTick); }
	BitfieldBool bHasSpecialParts() { return BoolValue(PropOffsets_XParticleEmitter.bHasSpecialParts); }
	BitfieldBool bWasPostDestroyed() { return BoolValue(PropOffsets_XParticleEmitter.bWasPostDestroyed); }
	BitfieldBool bHasInitView() { return BoolValue(PropOffsets_XParticleEmitter.bHasInitView); }
	TypedScriptArray<UXEmitter*> PartCombiners() { return DynamicArray<UXEmitter*>(PropOffsets_XParticleEmitter.PartCombiners); }
	BitfieldBool bUSNotifyParticles() { return BoolValue(PropOffsets_XParticleEmitter.bUSNotifyParticles); }
	BitfieldBool bNotifyNetReceive() { return BoolValue(PropOffsets_XParticleEmitter.bNotifyNetReceive); }
	BitfieldBool bUSModifyParticles() { return BoolValue(PropOffsets_XParticleEmitter.bUSModifyParticles); }
	BitfieldBool bNotOnPortals() { return BoolValue(PropOffsets_XParticleEmitter.bNotOnPortals); }

	void SetParticlesProps(std::optional<float> Speed, std::optional<float> Scale);
};
