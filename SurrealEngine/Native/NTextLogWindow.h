#pragma once

#include "UObject/UObject.h"

class NTextLogWindow
{
public:
	static void RegisterFunctions();

	static void AddLog(UObject* Self, const std::string& NewText, const Color& linecol);
	static void ClearLog(UObject* Self);
	static void PauseLog(UObject* Self, bool bNewPauseState);
	static void SetTextTimeout(UObject* Self, float newTimeout);
};
