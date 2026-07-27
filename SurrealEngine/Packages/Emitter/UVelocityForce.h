#pragma once

#include "UXParticleForces.h"

class UVelocityForce : public UXParticleForces
{
public:
	using UXParticleForces::UXParticleForces;

	BitfieldBool bChangeAcceleration() { return BoolValue(PropOffsets_VelocityForce.bChangeAcceleration); }
	BitfieldBool bInstantChange() { return BoolValue(PropOffsets_VelocityForce.bInstantChange); }
	vec3*& VelocityToAdd() { return Value<vec3*>(PropOffsets_VelocityForce.VelocityToAdd); }
};
