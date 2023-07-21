#pragma once

#include "UObject/UObject.h"

class NDecal
{
public:
	static void RegisterFunctions();

	static void AttachDecal(UObject* Self, float TraceDistance, vec3* DecalDir, UObject*& ReturnValue);
	static void DetachDecal(UObject* Self);
};
