#pragma once

#include "UObject/UObject.h"

class NDebugInfo
{
public:
	static void RegisterFunctions();

	static void AddTimingData(UObject* Self, const std::string& obj, const std::string& objName, int Time);
	static void Command(UObject* Self, const std::string& Cmd);
	static void GetString(UObject* Self, const std::string& Hash, std::string& ReturnValue);
	static void SetString(UObject* Self, const std::string& Hash, const std::string& Value);
};
