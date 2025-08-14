
#include "Precomp.h"
#include "Frame.h"
#include "Bytecode.h"
#include "ExpressionEvaluator.h"
#include "NativeFunc.h"
#include "UObject/UTextBuffer.h"
#include "UObject/USubsystem.h"
#include "Engine.h"
#include "Package/PackageManager.h"

std::function<void()> Frame::RunDebugger;
Array<Breakpoint> Frame::Breakpoints;
Array<Frame*> Frame::Callstack;
FrameRunState Frame::RunState = FrameRunState::Running;
Frame* Frame::StepFrame = nullptr;
Expression* Frame::StepExpression = nullptr;
std::string Frame::ExceptionText;
std::unique_ptr<Iterator> Frame::CreatedIterator;

bool Frame::AddBreakpoint(const NameString& packageName, const NameString& clsName, const NameString& funcName, const NameString& stateName)
{
	Breakpoint bp;
	bp.Package = packageName;
	bp.Class = clsName;
	bp.Function = funcName;
	bp.State = stateName;

	Package* pkg = engine->packages->GetPackage(packageName);
	UClass* cls = UObject::Cast<UClass>(pkg->GetUObject("Class", clsName));
	if (stateName.IsNone())
	{
		for (UField* child = cls->Children; child; child = child->Next)
		{
			if (child->Name == funcName && UObject::IsType<UFunction>(child))
			{
				UFunction* func = UObject::Cast<UFunction>(child);
				bp.Expr = func->Code->Statements.front();
				Breakpoints.push_back(bp);
				return true;
			}
		}
	}
	else
	{
		for (UField* child = cls->Children; child; child = child->Next)
		{
			if (child->Name == stateName && UObject::IsType<UState>(child))
			{
				UState* state = UObject::Cast<UState>(child);
				for (UField* stateChild = state->Children; stateChild; stateChild = stateChild->Next)
				{
					if (child->Name == funcName && UObject::IsType<UFunction>(child))
					{
						UFunction* func = UObject::Cast<UFunction>(child);
						bp.Expr = func->Code->Statements.front();
						Breakpoints.push_back(bp);
						return true;
					}
				}
			}
		}
	}
	return false;
}

void Frame::Break()
{
	RunState = FrameRunState::DebugBreak;

	if (RunDebugger)
	{
		engine->audiodev->BreakpointTriggered();
		RunDebugger();
	}
	else
	{
		if (!ExceptionText.empty())
		{
			std::string callstack = Frame::GetCallstack();
			std::string message = "Script execution error:\r\n\r\n";
			message += ExceptionText;
			message += "\r\n\r\nCall stack:\r\n\r\n" + callstack;
			Exception::Throw(message);
		}
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
		UStruct* func = (*it)->Func;
		std::string name;
		for (UStruct* s = func; s != nullptr; s = s->StructParent)
		{
			if (name.empty())
				name = s->Name.ToString();
			else
				name = s->Name.ToString() + "." + name;
		}
		if (func)
			name += " line " + std::to_string(func->Line);
		if (!result.empty()) result += newline;
		result += "at " + name;
	}
	return result;
}

ExpressionValue Frame::Call(UFunction* func, UObject* instance, Array<ExpressionValue> args)
{
	if (!instance->IsEventEnabled(func->Name))
	{
		return ExpressionValue::NothingValue();
	}

	int argindex = 0;
	for (UField* field = func->Children; field != nullptr; field = field->Next)
	{
		UProperty* prop = UObject::TryCast<UProperty>(field);
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
			UProperty* prop = UObject::TryCast<UProperty>(field);
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

		if (func->NativeFuncIndex != 0)
		{
			auto& callback = NativeFunctions::NativeByIndex[func->NativeFuncIndex];
			if (callback)
			{
				Frame frame(instance, func);
				Callstack.push_back(&frame);
				try
				{
					callback(instance, args.data());
					Callstack.pop_back();
				}
				catch (const std::exception& e)
				{
					Callstack.pop_back();
					LogMessage(std::string("Script error: ") + e.what());
					return ExpressionValue::NothingValue();
				}
				catch (...)
				{
					Callstack.pop_back();
					LogMessage("Script error: Unknown error");
					return ExpressionValue::NothingValue();
				}
			}
			else
			{
				Exception::Throw("Unknown native function " + func->NativeStruct->Name.ToString() + "." + func->Name.ToString());
			}
		}
		else
		{
			auto& callback = NativeFunctions::NativeByName[{ func->Name, func->NativeStruct->Name }];
			if (callback)
			{
				Frame frame(instance, func);
				Callstack.push_back(&frame);
				try
				{
					callback(instance, args.data());
					Callstack.pop_back();
				}
				catch (const std::exception& e)
				{
					Callstack.pop_back();
					LogMessage(std::string("Script error: ") + e.what());
					return ExpressionValue::NothingValue();
				}
				catch (...)
				{
					Callstack.pop_back();
					LogMessage("Script error: Unknown error");
					return ExpressionValue::NothingValue();
				}
			}
			else
			{
				Exception::Throw("Unknown native function " + func->NativeStruct->Name.ToString() + "." + func->Name.ToString());
			}
		}

		return returnparmfound ? std::move(args.back()) : ExpressionValue::NothingValue();
	}
	else
	{
		Frame frame(instance, func);

		int argindex = 0;
		for (UField* field = func->Children; field != nullptr; field = field->Next)
		{
			UProperty* prop = UObject::TryCast<UProperty>(field);
			if (prop)
			{
				ExpressionValue lvalue = ExpressionValue::Variable(frame.Variables.get(), prop);
				lvalue.ConstructVariable();
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
			UProperty* prop = UObject::TryCast<UProperty>(field);
			if (prop)
			{
				ExpressionValue lvalue = ExpressionValue::Variable(frame.Variables.get(), prop);

				if (AllFlags(prop->PropFlags, PropertyFlags::Parm | PropertyFlags::OutParm) && argindex < args.size())
				{
					args[argindex].Store(lvalue);
				}

				if (AllFlags(prop->PropFlags, PropertyFlags::ReturnParm) && result.GetType() == ExpressionValueType::Nothing)
				{
					result = ExpressionValue::DefaultValue(prop);
				}

				if (AllFlags(prop->PropFlags, PropertyFlags::Parm))
					argindex++;

				lvalue.DestructVariable();
			}
		}

		return result;
	}
}

Frame::Frame(UObject* instance, UStruct* func)
{
	Object = instance;
	SetState(func);
}

void Frame::SetState(UStruct* func)
{
	Func = func;
	if (func)
		Variables.reset(new uint64_t[(func->StructSize + 7) / 8]);
	else
		Variables.reset();
}

void Frame::GotoLabel(const NameString& label)
{
	for (UClass* cls = Object->Class; cls != nullptr; cls = static_cast<UClass*>(cls->BaseStruct))
	{
		UState* state = cls->GetState(Func->Name);
		if (state)
		{
			int labelIndex = state->Code->FindLabelIndex(label.IsNone() ? NameString("Begin") : label);
			if (labelIndex != -1)
			{
				Func = state;
				StatementIndex = labelIndex;
				LatentState = LatentRunState::Continue;
				return;
			}
		}
	}
	LatentState = LatentRunState::Stop;
}

void Frame::Tick()
{
	if (LatentState == LatentRunState::Continue)
		Run();
}

ExpressionEvalResult Frame::Run()
{
	if (!Func)
		return {};

	Callstack.push_back(this);

	if (!Func->Code->Statements.empty())
		StepExpression = Func->Code->Statements[StatementIndex];

	if (RunState == FrameRunState::StepInto)
	{
		Break();
	}

	const int maxInstructions = 1'000'000;
	int instructionsRetired = 0;
	while (instructionsRetired < maxInstructions)
	{
		if (StatementIndex >= Func->Code->Statements.size())
			ThrowException("Unexpected end of code statements");

		// Note: GotoState may change StatementIndex (jump to a different location) so we have to increment the index before executing the statement
		size_t curStatementIndex = StatementIndex;
		StatementIndex++;

		StepExpression = Func->Code->Statements[curStatementIndex];

		if (RunState == FrameRunState::StepOver && StepFrame == this)
		{
			Break();
		}

		Expression* statement = Func->Code->Statements[curStatementIndex];
		ExpressionEvalResult result = ExpressionEvaluator::Eval(statement, Object, Object, Variables.get());
		if (!Func)
			return result;
		switch (result.Result)
		{
		case StatementResult::Next:
			break;
		case StatementResult::Jump:
			StatementIndex = Func->Code->FindStatementIndex(result.JumpAddress);
			break;
		case StatementResult::Switch:
			ProcessSwitch(result.Value);
			break;
		case StatementResult::GotoLabel:
			{
				int index = Func->Code->FindLabelIndex(result.Label);
				if (index == -1)
					ThrowException("Could not find label: " + result.Label.ToString());
				StatementIndex = index;
			}
			break;
		case StatementResult::Stop:
			LatentState = LatentRunState::Stop;
			Callstack.pop_back();
			return result;
		case StatementResult::Return:
			// Package 61 and earlier transfered the return value in an out parameter
			if (!static_cast<ReturnExpression*>(statement)->Value)
			{
				for (UField* field = Func->Children; field != nullptr; field = field->Next)
				{
					UProperty* prop = UObject::TryCast<UProperty>(field);
					if (prop && AllFlags(prop->PropFlags, PropertyFlags::Parm | PropertyFlags::ReturnParm))
					{
						result.Value = ExpressionValue::PropertyValue(prop);
						result.Value.Load();
						break;
					}
				}
			}
			Callstack.pop_back();
			return result;
		case StatementResult::Iterator:
			if (!result.Iter)
				ThrowException("Iterator statement without an iterator!");
			Iterators.push_back(std::move(result.Iter));
			Iterators.back()->StartStatementIndex = curStatementIndex + 1;
			Iterators.back()->EndStatementIndex = Func->Code->FindStatementIndex(result.JumpAddress);
			if (Iterators.back()->Next())
				StatementIndex = Iterators.back()->StartStatementIndex;
			else
				StatementIndex = Iterators.back()->EndStatementIndex;
			break;
		case StatementResult::IteratorNext:
			if (Iterators.empty())
				ThrowException("Iterator next statement without an iterator!");
			if (Iterators.back()->Next())
				StatementIndex = Iterators.back()->StartStatementIndex;
			else
				StatementIndex = Iterators.back()->EndStatementIndex;
			break;
		case StatementResult::IteratorPop:
			if (Iterators.empty())
				ThrowException("Iterator pop statement without an iterator!");
			Iterators.pop_back();
			break;
		case StatementResult::AccessedNone:
			LogMessage("Accessed None in function" + Object->Name.ToString() + "." + Func->Name.ToString());
			break;
		}

		if (Object->StateFrame.get() == this && LatentState != LatentRunState::Continue)
		{
			Callstack.pop_back();
			return result;
		}

		instructionsRetired++;
	}

	ThrowException("Unreal script code ran for too long!");
	return {};
}

void Frame::ProcessSwitch(const ExpressionValue& condition)
{
	SwitchExpression* switchexpr = static_cast<SwitchExpression*>(Func->Code->Statements[StatementIndex - 1]);
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
