#pragma once

#include "UXParticleForces.h"

class UParticleConcentrateForce : public UXParticleForces
{
public:
	using UXParticleForces::UXParticleForces;

	BitfieldBool bActorDistanceSuckIn() { return BoolValue(PropOffsets_ParticleConcentrateForce.bActorDistanceSuckIn); }
	BitfieldBool bSetsAcceleration() { return BoolValue(PropOffsets_ParticleConcentrateForce.bSetsAcceleration); }
	vec3*& CenterPointOffset() { return Value<vec3*>(PropOffsets_ParticleConcentrateForce.CenterPointOffset); }
	float& DrainSpeed() { return Value<float>(PropOffsets_ParticleConcentrateForce.DrainSpeed); }
	float& MaxDistance() { return Value<float>(PropOffsets_ParticleConcentrateForce.MaxDistance); }
};
