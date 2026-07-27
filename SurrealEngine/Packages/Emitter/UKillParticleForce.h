#pragma once

#include "UXParticleForces.h"

class UKillParticleForce : public UXParticleForces
{
public:
	using UXParticleForces::UXParticleForces;

	float& LifeTimeDrainAmount() { return Value<float>(PropOffsets_KillParticleForce.LifeTimeDrainAmount); }
};
