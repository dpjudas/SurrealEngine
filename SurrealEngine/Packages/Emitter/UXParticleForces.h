#pragma once

#include "Packages/Engine/Actors/UActor.h"

struct Box;
struct FloatRange;

class UXParticleForces : public UActor
{
public:
	using UActor::UActor;

	BitfieldBool bEnabled() { return BoolValue(PropOffsets_XParticleForces.bEnabled); }
	BitfieldBool bUseBoxForcePosition() { return BoolValue(PropOffsets_XParticleForces.bUseBoxForcePosition); }
	Box*& EffectingBox() { return Value<Box*>(PropOffsets_XParticleForces.EffectingBox); }
	float& EffectingRadius() { return Value<float>(PropOffsets_XParticleForces.EffectingRadius); }
	FloatRange*& EffectPartLifeTime() { return Value<FloatRange*>(PropOffsets_XParticleForces.EffectPartLifeTime); }
	NameString& OldTagName() { return Value<NameString>(PropOffsets_XParticleForces.OldTagName); }
};
