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

enum class LatentRunState
{
	Continue,
	Stop,
	Sleep,
	FinishAnim,
	FinishInterpolation,
	MoveTo,
	MoveToward,
	StrafeTo,
	StrafeFacing,
	TurnTo,
	TurnToward,
	WaitForLanding
};

class Frame
{
public:
	static ExpressionValue Call(UFunction* func, UObject* instance, std::vector<ExpressionValue> args);
	static std::string GetCallstack();

	static void AddBreakpoint(const NameString& package, const NameString& cls, const NameString& func, const NameString& state = {});

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

	Frame(UObject* instance, UStruct* func);

	void SetState(UStruct* func);

	void GotoLabel(const NameString& label);
	void Tick();

	LatentRunState LatentState = LatentRunState::Continue;

	std::unique_ptr<uint64_t[]> Variables;
	UObject* Object = nullptr;
	UStruct* Func = nullptr;
	size_t StatementIndex = 0;
	std::vector<std::unique_ptr<Iterator>> Iterators;

private:
	ExpressionEvalResult Run();
	void ProcessSwitch(const ExpressionValue& condition);
};
