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

struct Breakpoint
{
	NameString Package;
	NameString Class;
	NameString Function;
	NameString State;
	Expression* Expr = nullptr;
	UProperty* Property = nullptr; // For watchpoints. Not implemented yet.
	bool Enabled = true;
};

class LocalVariables
{
public:
	LocalVariables(UStruct* func);
	~LocalVariables();

	UStruct* Func = nullptr;
	void* Data = nullptr;
};

class Frame
{
public:
	static ExpressionValue Call(UFunction* func, UObject* instance, Array<ExpressionValue> args);
	static std::string GetCallstack();

	static bool AddBreakpoint(const NameString& package, const NameString& cls, const NameString& func, const NameString& state = {});

	static std::function<void()> RunDebugger;
	static Array<Breakpoint> Breakpoints;
	static Array<Frame*> Callstack;
	static FrameRunState RunState;
	static Frame* StepFrame;
	static Expression* StepExpression;
	static std::string ExceptionText;

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

	std::unique_ptr<LocalVariables> Variables;
	UObject* Object = nullptr;
	UStruct* Func = nullptr;
	size_t StatementIndex = 0;
	Array<std::unique_ptr<Iterator>> Iterators;

private:
	ExpressionEvalResult Run();
	void ProcessSwitch(const ExpressionValue& condition);

	struct ActiveCallStackFrame
	{
		ActiveCallStackFrame(Frame* frame) { Frame::Callstack.push_back(frame); }
		~ActiveCallStackFrame() { Frame::Callstack.pop_back(); }
	};
};
