#pragma once

#include "UObject/UObject.h"

class NDecal
{
public:
	static void RegisterFunctions();

	static void AttachDecal(UObject* Self, float TraceDistance, vec3* DecalDir, UObject*& ReturnValue);
	static void AttachDecal_Deus(UObject* Self, float TraceDistance, vec3* DecalDir, BitfieldBool& ReturnValue);
	static void DetachDecal(UObject* Self);
};
