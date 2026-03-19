#pragma once

#include "UObject/UObject.h"

class NLargeTextWindow
{
public:
	static void RegisterFunctions();

	static void SetVerticalSpacing(UObject* Self, std::optional<float> newVSpace);
};
