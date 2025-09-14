
#include "Precomp.h"
#include "NConsole.h"
#include "VM/NativeFunc.h"
#include "VM/ScriptCall.h"
#include "Engine.h"

void NConsole::RegisterFunctions()
{
	RegisterVMNativeFunc_2("Console", "ConsoleCommand", &NConsole::ConsoleCommand, 0);
	RegisterVMNativeFunc_1("Console", "SaveTimeDemo", &NConsole::SaveTimeDemo, 0);
}

void NConsole::ConsoleCommand(UObject* Self, const std::string& S, BitfieldBool& ReturnValue)
{
	std::string result = engine->ConsoleCommand(Self, S, ReturnValue);
	if (!result.empty())
	{
		// XXX: This depends on how Engine.Console.Message is declared
		// might break for other games
		Array<ExpressionValue> exprs =
		{
			ExpressionValue::ObjectValue(nullptr),
			ExpressionValue::StringValue(result),
			ExpressionValue::NameValue("Console")
		};
		CallEvent(Self, "Message", exprs);
	}
}

void NConsole::SaveTimeDemo(UObject* Self, const std::string& S)
{
	LogUnimplemented("Console.SaveTimeDemo(" + S + ")");
}
