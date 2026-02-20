#pragma once

#include "UObject/UObject.h"

class NModalWindow
{
public:
	static void RegisterFunctions();

	static void IsCurrentModal(UObject* Self, BitfieldBool& ReturnValue);
	static void SetMouseFocusMode(UObject* Self, uint8_t newFocusMode);
};
