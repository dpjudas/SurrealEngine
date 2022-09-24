#pragma once

#include "UObject/UObject.h"

class NParticleIterator
{
public:
	static void RegisterFunctions();

	static void UpdateParticles(UObject* Self, float DeltaTime);
};
