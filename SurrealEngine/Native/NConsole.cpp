
#include "Precomp.h"
#include "NConsole.h"
#include "VM/NativeFunc.h"
#include "Engine.h"

void NConsole::RegisterFunctions()
{
	RegisterVMNativeFunc_2("Console", "ConsoleCommand", &NConsole::ConsoleCommand, 0);
	RegisterVMNativeFunc_1("Console", "SaveTimeDemo", &NConsole::SaveTimeDemo, 0);
}

void NConsole::ConsoleCommand(UObject* Self, const std::string& S, bool& ReturnValue)
{
	engine->ConsoleCommand(Self, S, ReturnValue);
}

void NConsole::SaveTimeDemo(UObject* Self, const std::string& S)
{
	engine->LogUnimplemented("Console.SaveTimeDemo(" + S + ")");
}
