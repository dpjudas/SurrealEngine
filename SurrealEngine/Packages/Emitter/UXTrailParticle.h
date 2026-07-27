#pragma once

#include "Packages/Engine/Actors/UActor.h"

// A pretty much empty class that gets immediately destroyed on PreBeginPlay()
class UXTrailParticle : public UActor
{
public:
	using UActor::UActor;
};
