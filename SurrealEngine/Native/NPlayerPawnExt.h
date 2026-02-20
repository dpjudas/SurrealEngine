#pragma once

#include "UObject/UObject.h"

class NPlayerPawnExt
{
public:
	static void RegisterFunctions();

	static void ConstructRootWindow(UObject* Self);
	static void InitRootWindow(UObject* Self);
	static void PostRenderWindows(UObject* Self, UObject* Canvas);
	static void PreRenderWindows(UObject* Self, UObject* Canvas);
};
