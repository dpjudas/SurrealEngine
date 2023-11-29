#pragma once

#include "UObject/UObject.h"

class NConsole
{
public:
	static void RegisterFunctions();

	static void ConsoleCommand(UObject* Self, const std::string& S, BitfieldBool& ReturnValue);
	static void SaveTimeDemo(UObject* Self, const std::string& S);
};
