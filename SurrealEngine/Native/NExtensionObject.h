#pragma once

#include "Packages/Core/UObject.h"

class NExtensionObject
{
public:
	static void RegisterFunctions();

	static void StringToName(UObject* Self, const std::string& str, NameString& ReturnValue);
};
