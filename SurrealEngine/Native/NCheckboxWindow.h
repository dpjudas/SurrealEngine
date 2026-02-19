#pragma once

#include "UObject/UObject.h"

class NCheckboxWindow
{
public:
	static void RegisterFunctions();

	static void SetCheckboxColor(UObject* Self, const Color& NewColor);
	static void SetCheckboxSpacing(UObject* Self, float newSpacing);
	static void SetCheckboxStyle(UObject* Self, uint8_t NewStyle);
	static void SetCheckboxTextures(UObject* Self, UObject** toggleOff, UObject** toggleOn, float* textureWidth, float* textureHeight);
	static void ShowCheckboxOnRightSide(UObject* Self, BitfieldBool* bRight);
};
