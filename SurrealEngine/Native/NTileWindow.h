#pragma once

#include "UObject/UObject.h"

class NTileWindow
{
public:
	static void RegisterFunctions();

	static void EnableWrapping(UObject* Self, bool bWrapOn);
	static void FillParent(UObject* Self, bool FillParent);
	static void MakeHeightsEqual(UObject* Self, bool bEqual);
	static void MakeWidthsEqual(UObject* Self, bool bEqual);
	static void SetChildAlignments(UObject* Self, uint8_t newHAlign, uint8_t newVAlign);
	static void SetDirections(UObject* Self, uint8_t newHDir, uint8_t newVDir);
	static void SetMajorSpacing(UObject* Self, float newSpacing);
	static void SetMargins(UObject* Self, float newHMargin, float newVMargin);
	static void SetMinorSpacing(UObject* Self, float newSpacing);
	static void SetOrder(UObject* Self, uint8_t newOrder);
	static void SetOrientation(UObject* Self, uint8_t newOrientation);
};
