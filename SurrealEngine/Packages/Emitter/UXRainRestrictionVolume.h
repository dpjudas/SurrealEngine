#pragma once

#include "Packages/Engine/Actors/UActor.h"

class UXRainRestrictionVolume : public UActor
{
public:
	using UActor::UActor;

	vec3*& BoundsMax() { return Value<vec3*>(PropOffsets_XRainRestrictionVolume.BoundsMax); }
	vec3*& BoundsMin() { return Value<vec3*>(PropOffsets_XRainRestrictionVolume.BoundsMin); }
};
