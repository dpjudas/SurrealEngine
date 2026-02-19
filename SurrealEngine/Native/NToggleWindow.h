#pragma once

#include "UObject/UObject.h"

class NToggleWindow
{
public:
	static void RegisterFunctions();

	static void ChangeToggle(UObject* Self);
	static void GetToggle(UObject* Self, BitfieldBool& ReturnValue);
	static void SetToggle(UObject* Self, bool bNewToggle);
	static void SetToggleSounds(UObject* Self, UObject** enableSound, UObject** disableSound);
};
