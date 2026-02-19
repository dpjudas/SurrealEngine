#pragma once

#include "UObject/UObject.h"

class NConEvent
{
public:
	static void RegisterFunctions();

	static void GetSoundLength(UObject* Self, UObject* ASound, float& ReturnValue);
};
