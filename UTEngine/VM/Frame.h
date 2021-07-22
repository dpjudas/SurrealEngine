#pragma once

#include "ExpressionValue.h"
#include "Iterator.h"

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

	static void AddBreakpoint(const std::string& package, const std::string& cls, const std::string& func, const std::string& state = {});

	static DebuggerWindow* Debugger;
	static std::vector<Expression*> Breakpoints;
	static std::vector<Frame*> Callstack;
	static FrameRunState RunState;
	static Frame* StepFrame;
	static Expression* StepExpression;
	static std::string ExceptionText;

	static void ShowDebuggerWindow();
	static void Break();
	static void Resume();
	static void StepInto();
	static void StepOver();
	static void ThrowException(const std::string& text);

	static std::unique_ptr<Iterator> CreatedIterator;

	std::unique_ptr<uint64_t[]> Variables;
	UObject* Object = nullptr;
	UFunction* Func = nullptr;
	size_t StatementIndex = 0;
	std::vector<std::unique_ptr<Iterator>> Iterators;

private:
	ExpressionEvalResult Run();
	void ProcessSwitch(const ExpressionValue& condition);
};
