#pragma once

#include "UObject/UDXSaveInfo.h"

class NDeusExSaveInfo
{
public:
	static void RegisterFunctions();

	static void UpdateTimeStamp(UObject* Self);
};
