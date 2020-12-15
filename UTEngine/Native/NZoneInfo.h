#pragma once

#include "UObject/UObject.h"

class NZoneInfo
{
public:
	static void RegisterFunctions();

	static void ZoneActors(UObject* Self, UObject* BaseClass, UObject*& Actor);
};
