#pragma once

#include "UObject/UObject.h"

class NNavigationPoint
{
public:
	static void RegisterFunctions();

	static void describeSpec(UObject* Self, int iSpec, UObject*& Start, UObject*& End, int& ReachFlags, int& Distance);
};
