#pragma once

#include "Packages/Core/UObject.h"

class NDeusExSaveInfo
{
public:
	static void RegisterFunctions();

	static void UpdateTimeStamp(UObject* Self);
};
