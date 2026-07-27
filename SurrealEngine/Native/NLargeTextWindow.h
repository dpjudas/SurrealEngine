#pragma once

#include "Packages/Core/UObject.h"

class NLargeTextWindow
{
public:
	static void RegisterFunctions();

	static void SetVerticalSpacing(UObject* Self, std::optional<float> newVSpace);
};
