#pragma once

#include "UObject/UObject.h"

class NConsole
{
public:
	static void RegisterFunctions();

	static void ConsoleCommand(UObject* Self, const std::string& S, bool& ReturnValue);
	static void SaveTimeDemo(UObject* Self, const std::string& S);
};
