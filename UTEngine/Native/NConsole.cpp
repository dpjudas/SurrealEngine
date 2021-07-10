
#include "Precomp.h"
#include "NConsole.h"
#include "VM/NativeFunc.h"

void NConsole::RegisterFunctions()
{
	RegisterVMNativeFunc_2("Console", "ConsoleCommand", &NConsole::ConsoleCommand, 0);
	RegisterVMNativeFunc_1("Console", "SaveTimeDemo", &NConsole::SaveTimeDemo, 0);
}

void NConsole::ConsoleCommand(UObject* Self, const std::string& S, bool& ReturnValue)
{
	ReturnValue = false;
	// throw std::runtime_error("Console.ConsoleCommand not implemented");
}

void NConsole::SaveTimeDemo(UObject* Self, const std::string& S)
{
	throw std::runtime_error("Console.SaveTimeDemo not implemented");
}
