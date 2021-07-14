#pragma once

#include "ExpressionValue.h"

class DebuggerWindow;
class Bytecode;
class UObject;
class UFunction;
class Expression;
struct ExpressionEvalResult;

enum class FrameRunState
{
	Running,
	DebugBreak,
	StepInto,
	StepOver,
	StepOut
};

class Frame
{
public:
	static ExpressionValue Call(UFunction* func, UObject* instance, std::vector<ExpressionValue> args);
	static std::string GetCallstack();

	static DebuggerWindow* Debugger;
	static std::vector<Expression*> Breakpoints;
	static std::vector<Frame*> Callstack;
	static FrameRunState RunState;
	static Frame* StepFrame;
	static std::string ExceptionText;

	static void Break();
	static void Resume();
	static void StepInto();
	static void StepOver();
	static void ThrowException(const std::string& text);

	std::unique_ptr<uint64_t[]> Variables;
	UObject* Object = nullptr;
	UFunction* Func = nullptr;
	size_t StatementIndex = 0;

private:
	ExpressionEvalResult Run();
	void ProcessSwitch(const ExpressionValue& condition);
};
