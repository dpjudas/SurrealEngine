#pragma once

#include "UObject/UObject.h"

class NTextWindow
{
public:
	static void RegisterFunctions();

	static void AppendText(UObject* Self, const std::string& NewText);
	static void EnableTextAsAccelerator(UObject* Self, BitfieldBool* bEnable);
	static void GetText(UObject* Self, std::string& ReturnValue);
	static void GetTextLength(UObject* Self, int& ReturnValue);
	static void GetTextPart(UObject* Self, int startPos, int Count, std::string& OutText, int& ReturnValue);
	static void ResetLines(UObject* Self);
	static void ResetMinWidth(UObject* Self);
	static void SetLines(UObject* Self, int newMinLines, int newMaxLines);
	static void SetMaxLines(UObject* Self, int newMaxLines);
	static void SetMinLines(UObject* Self, int newMinLines);
	static void SetMinWidth(UObject* Self, float newMinWidth);
	static void SetText(UObject* Self, const std::string& NewText);
	static void SetTextAlignments(UObject* Self, uint8_t newHAlign, uint8_t newVAlign);
	static void SetTextMargins(UObject* Self, float newHMargin, float newVMargin);
	static void SetWordWrap(UObject* Self, bool bNewWordWrap);
};
