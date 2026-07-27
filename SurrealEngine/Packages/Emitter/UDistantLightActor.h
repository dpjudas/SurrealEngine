#pragma once

#include "Packages/Engine/Actors/ULight.h"

// This is in Emitter package for some reason.
// Supports unlimited lighting distance.
class UDistantLightActor : public ULight
{
public:
	using ULight::ULight;

	float& NewLightRadius() { return Value<float>(PropOffsets_DistantLightActor.NewLightRadius); }
};
