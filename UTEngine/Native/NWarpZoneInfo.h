#pragma once

#include "UObject/UObject.h"

class NWarpZoneInfo
{
public:
	static void RegisterFunctions();

	static void UnWarp(UObject* Self, vec3& Loc, vec3& Vel, Rotator& R);
	static void Warp(UObject* Self, vec3& Loc, vec3& Vel, Rotator& R);
};
