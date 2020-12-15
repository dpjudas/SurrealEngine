#pragma once

#include "UObject/UObject.h"

class NLevelInfo
{
public:
	static void RegisterFunctions();

	static void GetAddressURL(UObject* Self, std::string& ReturnValue);
	static void GetLocalURL(UObject* Self, std::string& ReturnValue);
};
