#pragma once

#include "UObject/UObject.h"

class NDeusExSaveInfo
{
public:
	static void RegisterFunctions();

	static void UpdateTimeStamp(UObject* Self);
};
