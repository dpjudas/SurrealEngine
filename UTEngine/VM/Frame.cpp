
#include "Precomp.h"
#include "Frame.h"
#include "Bytecode.h"
#include "ExpressionEvaluator.h"
#include "NativeFunc.h"
#include "UObject/UTextBuffer.h"
#include "UI/Debugger/DebuggerWindow.h"
#include "Engine.h"
#include "Package/PackageManager.h"

DebuggerWindow* Frame::Debugger = nullptr;
std::vector<Expression*> Frame::Breakpoints;
std::vector<Frame*> Frame::Callstack;
FrameRunState Frame::RunState = FrameRunState::Running;
Frame* Frame::StepFrame = nullptr;
Expression* Frame::StepExpression = nullptr;
std::string Frame::ExceptionText;
std::unique_ptr<Iterator> Frame::CreatedIterator;

void Frame::AddBreakpoint(const std::string& packageName, const std::string& clsName, const std::string& funcName, const std::string& stateName)
{
	Package* pkg = engine->packages->GetPackage(packageName);
	UClass* cls = UObject::Cast<UClass>(pkg->GetUObject("Class", clsName));
	if (stateName.empty())
	{
		for (UField* child = cls->Children; child; child = child->Next)
		{
			if (child->Name == funcName && dynamic_cast<UFunction*>(child))
			{
				UFunction* func = static_cast<UFunction*>(child);
				Breakpoints.push_back(func->Code->Statements.front());
				return;
			}
		}
	}
	else
	{
		for (UField* child = cls->Children; child; child = child->Next)
		{
			if (child->Name == stateName && dynamic_cast<UState*>(child))
			{
				UState* state = static_cast<UState*>(child);
				for (UField* stateChild = state->Children; stateChild; stateChild = stateChild->Next)
				{
					if (child->Name == funcName && dynamic_cast<UFunction*>(child))
					{
						UFunction* func = static_cast<UFunction*>(child);
						Breakpoints.push_back(func->Code->Statements.front());
						return;
					}
				}
			}
		}
	}
}

void Frame::Break()
{
	RunState = FrameRunState::DebugBreak;

#ifdef WIN32
	if (!Debugger)
	{
		Debugger = new DebuggerWindow([]() {
			delete Debugger;
			Debugger = nullptr;
		});
		Debugger->show();
	}

	Debugger->onBreakpointTriggered();

	while (Debugger && RunState == FrameRunState::DebugBreak)
	{
		MSG msg = {};
		int result = GetMessage(&msg, 0, 0, 0);
		if (result <= 0 || msg.message == WM_QUIT)
			break;
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
#endif

	if (!ExceptionText.empty())
	{
		std::string callstack = Frame::GetCallstack();
		std::string message = "Script execution error:\r\n\r\n";
		message += ExceptionText;
		message += "\r\n\r\nCall stack:\r\n\r\n" + callstack;
		throw std::runtime_error(message);
	}
}

void Frame::Resume()
{
	RunState = FrameRunState::Running;
}

void Frame::StepInto()
{
	StepFrame = Callstack.back();
	RunState = FrameRunState::StepInto;
}

void Frame::StepOver()
{
	StepFrame = Callstack.back();
	RunState = FrameRunState::StepOver;
}

void Frame::ThrowException(const std::string& text)
{
#if defined(_DEBUG) && defined(WIN32)
	DebugBreak();
#endif

	ExceptionText = text;
	Break();
}

std::string Frame::GetCallstack()
{
	std::string result;

#ifdef WIN32
	std::string newline = "\r\n";
#else
	std::string newline = "\n";
#endif

	for (auto it = Callstack.rbegin(); it != Callstack.rend(); ++it)
	{
		UFunction* func = (*it)->Func;
		std::string name;
		for (UStruct* s = func; s != nullptr; s = s->StructParent)
		{
			if (name.empty())
				name = s->Name;
			else
				name = s->Name + "." + name;
		}
		if (func)
			name += " line " + std::to_string(func->Line);
		if (!result.empty()) result += newline;
		result += name;
	}
	return result;
}

ExpressionValue Frame::Call(UFunction* func, UObject* instance, std::vector<ExpressionValue> args)
{
	int argindex = 0;
	for (UField* field = func->Children; field != nullptr; field = field->Next)
	{
		UProperty* prop = dynamic_cast<UProperty*>(field);
		if (prop)
		{
			if (argindex == args.size() && AllFlags(prop->PropFlags, PropertyFlags::Parm | PropertyFlags::OptionalParm))
				args.push_back(ExpressionValue::NothingValue());

			if (AllFlags(prop->PropFlags, PropertyFlags::Parm))
				argindex++;
		}
	}

	if (AllFlags(func->FuncFlags, FunctionFlags::Native))
	{
		bool returnparmfound = false;
		argindex = 0;
		for (UField* field = func->Children; field != nullptr; field = field->Next)
		{
			UProperty* prop = dynamic_cast<UProperty*>(field);
			if (prop)
			{
				if (AllFlags(prop->PropFlags, PropertyFlags::Parm | PropertyFlags::ReturnParm))
				{
					ExpressionValue retval = ExpressionValue::PropertyValue(prop);
					args.push_back(std::move(retval));
					returnparmfound = true;
				}
				if (AllFlags(prop->PropFlags, PropertyFlags::Parm))
					argindex++;
			}
		}

		try
		{
			if (func->NativeFuncIndex != 0)
			{
				NativeFunctions::NativeByIndex[func->NativeFuncIndex](instance, args.data());
			}
			else
			{
				auto& callback = NativeFunctions::NativeByName[{ func->Name, func->NativeStruct->Name }];
				if (callback)
					callback(instance, args.data());
				else
					throw std::runtime_error("Unknown native function " + func->NativeStruct->Name + "." + func->Name);
			}
		}
		catch (const std::exception& e)
		{
			Frame::ThrowException(e.what());
		}

		return returnparmfound ? std::move(args.back()) : ExpressionValue::NothingValue();
	}
	else
	{
		Frame frame;
		frame.Variables.reset(new uint64_t[(func->StructSize + 7) / 8]);
		frame.Object = instance;
		frame.Func = func;

		int argindex = 0;
		for (UField* field = func->Children; field != nullptr; field = field->Next)
		{
			UProperty* prop = dynamic_cast<UProperty*>(field);
			if (prop)
			{
				ExpressionValue lvalue = ExpressionValue::Variable(frame.Variables.get(), prop);
				lvalue.VariableProperty->Construct(lvalue.VariablePtr);
				if (AllFlags(prop->PropFlags, PropertyFlags::Parm))
				{
					if (argindex < args.size())
					{
						lvalue.Store(args[argindex]);
					}

					argindex++;
				}
			}
		}

		ExpressionValue result = frame.Run().Value;
		result.Load();

		argindex = 0;
		for (UField* field = func->Children; field != nullptr; field = field->Next)
		{
			UProperty* prop = dynamic_cast<UProperty*>(field);
			if (prop)
			{
				ExpressionValue lvalue = ExpressionValue::Variable(frame.Variables.get(), prop);

				if (AllFlags(prop->PropFlags, PropertyFlags::Parm | PropertyFlags::OutParm) && argindex < args.size())
				{
					args[argindex].Store(lvalue);
				}

				if (AllFlags(prop->PropFlags, PropertyFlags::Parm))
					argindex++;

				lvalue.VariableProperty->Destruct(lvalue.VariablePtr);
			}
		}

		return result;
	}
}

ExpressionEvalResult Frame::Run()
{
	Callstack.push_back(this);

	if (!Func->Code->Statements.empty())
		StepExpression = Func->Code->Statements[StatementIndex];

	if (RunState == FrameRunState::StepInto)
	{
		Break();
	}

	while (true)
	{
		if (StatementIndex >= Func->Code->Statements.size())
			throw std::runtime_error("Unexpected end of code statements");

		StepExpression = Func->Code->Statements[StatementIndex];

		if (RunState == FrameRunState::StepOver && StepFrame == this)
		{
			Break();
		}

		Expression* statement = Func->Code->Statements[StatementIndex];
		ExpressionEvalResult result = ExpressionEvaluator::Eval(statement, Object, Object, Variables.get());
		switch (result.Result)
		{
		case StatementResult::Next:
			StatementIndex++;
			break;
		case StatementResult::Jump:
			StatementIndex = Func->Code->FindStatementIndex(result.JumpAddress);
			break;
		case StatementResult::Switch:
			ProcessSwitch(result.Value);
			break;
		case StatementResult::LatentWait:
		case StatementResult::Return:
		case StatementResult::Stop:
		case StatementResult::GotoLabel:
			Callstack.pop_back();
			return result;

		case StatementResult::Iterator:
			if (!result.Iter)
				throw std::runtime_error("Iterator statement without an iterator!");
			Iterators.push_back(std::move(result.Iter));
			Iterators.back()->StartStatementIndex = StatementIndex + 1;
			Iterators.back()->EndStatementIndex = Func->Code->FindStatementIndex(result.JumpAddress);
			if (Iterators.back()->Next())
				StatementIndex = Iterators.back()->StartStatementIndex;
			else
				StatementIndex = Iterators.back()->EndStatementIndex;
			break;
		case StatementResult::IteratorNext:
			if (Iterators.empty())
				throw std::runtime_error("Iterator next statement without an iterator!");
			if (Iterators.back()->Next())
				StatementIndex = Iterators.back()->StartStatementIndex;
			else
				StatementIndex = Iterators.back()->EndStatementIndex;
			break;
		case StatementResult::IteratorPop:
			if (Iterators.empty())
				throw std::runtime_error("Iterator pop statement without an iterator!");
			Iterators.pop_back();
			StatementIndex++;
			break;
		}
	}

	Callstack.pop_back();
}

void Frame::ProcessSwitch(const ExpressionValue& condition)
{
	SwitchExpression* switchexpr = static_cast<SwitchExpression*>(Func->Code->Statements[StatementIndex++]);
	while (true)
	{
		CaseExpression* caseexpr = static_cast<CaseExpression*>(Func->Code->Statements[StatementIndex++]);
		if (caseexpr->Value)
		{
			ExpressionValue casevalue = ExpressionEvaluator::Eval(caseexpr->Value, Object, Object, Variables.get()).Value;
			if (condition.IsEqual(casevalue))
				break;
			else
				StatementIndex = Func->Code->FindStatementIndex(caseexpr->NextOffset);
		}
		else
		{
			break;
		}
	}
}
