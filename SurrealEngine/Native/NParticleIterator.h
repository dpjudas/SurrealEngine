#pragma once

#include "Packages/Core/UObject.h"

class NParticleIterator
{
public:
	static void RegisterFunctions();

	static void UpdateParticles(UObject* Self, float DeltaTime);
};
