#pragma once

#include "UObject/UObject.h"

class NScrollAreaWindow
{
public:
	static void RegisterFunctions();

	static void AutoHideScrollbars(UObject* Self, BitfieldBool* bHide);
	static void EnableScrolling(UObject* Self, BitfieldBool* bHScrolling, BitfieldBool* bVScrolling);
	static void SetAreaMargins(UObject* Self, float newMarginWidth, float newMarginHeight);
	static void SetScrollbarDistance(UObject* Self, float newDistance);
};
