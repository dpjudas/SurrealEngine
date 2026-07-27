#pragma once

#include "Packages/Core/UObject.h"

class NCommandlet
{
public:
	static void RegisterFunctions();

	static void Main(UObject* Self, const std::string& Parms, int& ReturnValue);
};
