#pragma once

#include "UObject/UObject.h"

class NScrollAreaWindow
{
public:
	static void RegisterFunctions();

	static void AutoHideScrollbars(UObject* Self, std::optional<bool> bHide);
	static void EnableScrolling(UObject* Self, std::optional<bool> bHScrolling, std::optional<bool> bVScrolling);
	static void SetAreaMargins(UObject* Self, float newMarginWidth, float newMarginHeight);
	static void SetScrollbarDistance(UObject* Self, float newDistance);
};
