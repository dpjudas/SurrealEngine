#pragma once

#include "UObject/UObject.h"

class NScaleManagerWindow
{
public:
	static void RegisterFunctions();

	static void SetManagerAlignments(UObject* Self, uint8_t newHAlign, uint8_t newVAlign);
	static void SetManagerMargins(UObject* Self, float* newMarginWidth, float* newMarginHeight);
	static void SetManagerOrientation(UObject* Self, uint8_t newOrientation);
	static void SetMarginSpacing(UObject* Self, float* newSpacing);
	static void SetScale(UObject* Self, UObject* NewScale);
	static void SetScaleButtons(UObject* Self, UObject* newDecButton, UObject* newIncButton);
	static void SetValueField(UObject* Self, UObject* newValueField);
	static void StretchScaleField(UObject* Self, BitfieldBool* bNewStretch);
	static void StretchValueField(UObject* Self, BitfieldBool* bNewStretch);
};
