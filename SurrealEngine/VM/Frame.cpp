
#include "Precomp.h"
#include "Frame.h"
#include "Bytecode.h"
#include "ExpressionEvaluator.h"
#include "NativeFunc.h"
#include "Packages/Core/UTextBuffer.h"
#include "Packages/Core/UFunction.h"
#include "Packages/Engine/Subsystems/USurrealAudioDevice.h"
#include "Engine.h"
#include "Package/PackageManager.h"
#include "Utils/AlignedAlloc.h"
#include "Commandlet/VM/DisassemblyCommandlet.h"

std::function<void()> Frame::RunDebugger;
Array<Breakpoint> Frame::Breakpoints;
Array<Frame*> Frame::Callstack;
FrameRunState Frame::RunState = FrameRunState::Running;
Frame* Frame::StepFrame = nullptr;
Expression* Frame::StepExpression = nullptr;
std::string Frame::ExceptionText;
std::unique_ptr<Iterator> Frame::CreatedIterator;

Frame::Frame(UObject* instance, UStruct* func)
{
	Object = instance;
	SetState(func);
}

void Frame::SetState(UStruct* func)
{
	Func = func;
	Variables = std::make_unique<LocalVariables>(func);
}

bool Frame::AddBreakpoint(const NameString& clsName, const NameString& funcName, const NameString& stateName, int statementIndex)
{
	Breakpoint bp;
	bp.Class = clsName;
	bp.Function = funcName;
	bp.State = stateName;

	UClass* cls = engine->packages->FindClass(clsName);
	if (!cls)
		return false;

	if (stateName.IsNone())
	{
		for (UField* child = cls->Children; child; child = child->Next)
		{
			if (child->Name == funcName && UObject::IsType<UFunction>(child))
			{
				UFunction* func = UObject::Cast<UFunction>(child);
				if (statementIndex < 0 || (size_t)statementIndex >= func->Code->Statements.size())
					return false;
				bp.Expr = func->Code->Statements[statementIndex];
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
					if (stateChild->Name == funcName && UObject::IsType<UFunction>(stateChild))
					{
						UFunction* func = UObject::Cast<UFunction>(stateChild);
						if (statementIndex < 0 || (size_t)statementIndex >= func->Code->Statements.size())
							return false;
						bp.Expr = func->Code->Statements[statementIndex];
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

void Frame::StepOut()
{
	StepFrame = Callstack.back();
	RunState = FrameRunState::StepOut;
}

void Frame::ThrowException(const std::string& text)
{
#if defined(_DEBUG) && defined(WIN32)
	DebugBreak();
#endif

	ExceptionText = text;
	Break();
}

std::string debugCallstack;
const char* GetCallStack()
{
	debugCallstack = Frame::GetCallstack();
	return debugCallstack.c_str();
}

std::string Frame::GetName()
{
	std::string name;
	if (Func)
	{
		for (UStruct* s = Func; s != nullptr; s = s->StructParent)
		{
			if (name.empty())
				name = s->Name.ToString();
			else
				name = s->Name.ToString() + "." + name;
		}
	}
	return name;
}

std::string Frame::GetDisassembly(Expression* statement)
{
	std::string result;
	PrintPrettyExpression::Print([&](const std::string& text) { result += text; }, statement);
	return result;
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
		Frame* frame = *it;
		std::string name = frame->GetName();
		if (UStruct* func = frame->Func)
		{
			name += " line " + std::to_string(func->Line);

			if (frame->StatementIndex > 0) // StatementIndex points at the NEXT statement to be executed
			{
				name += ": ";
				name += GetDisassembly(func->Code->Statements[frame->StatementIndex - 1]);
			}
		}
		if (!result.empty()) result += newline;
		result += "at " + name;
	}
	return result;
}

ExpressionValue Frame::Call(UFunction* func, UObject* instance, ArrayView<ExpressionValue> args)
{
	// To do: fix this as it produces a memory allocation
	Array<ExpressionValue> args2;
	args2.reserve(args.size());
	for (auto& arg : args)
		args2.push_back(std::move(arg));
	return Call(func, instance, args2);
}

ExpressionValue Frame::Call(UFunction* func, UObject* instance, Array<ExpressionValue> args)
{
	if (!instance)
	{
		LogMessage("Accessed None when calling " + func->Name.ToString());
		LogMessage(Frame::GetCallstack());
		return ExpressionValue::NothingValue();
	}

	TraceCall(func, instance, args);

	if (!instance->IsEventEnabled(func->Name))
	{
		return ExpressionValue::NothingValue();
	}

	// Trailing optional args may be missing. Add nothing values so the args list matches the function signature.
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
		return CallNative(func, instance, std::move(args));
	}
	else
	{
		return CallScript(func, instance, std::move(args));
	}
}

ExpressionValue Frame::CallScript(UFunction* func, UObject* instance, Array<ExpressionValue> args)
{
	Frame frame(instance, func);

	// Store args in function frame local variables
	int argindex = 0;
	for (UField* field = func->Children; field != nullptr; field = field->Next)
	{
		UProperty* prop = UObject::TryCast<UProperty>(field);
		if (prop)
		{
			ExpressionValue lvalue = ExpressionValue::Variable(frame.Variables->Data, prop);
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

	// Run the function
	ExpressionValue result = frame.Run().Value;

	// Load the result from the frame local result variable
	result.Load();

	// Copy out params from frame local variables
	argindex = 0;
	for (UField* field = func->Children; field != nullptr; field = field->Next)
	{
		UProperty* prop = UObject::TryCast<UProperty>(field);
		if (prop)
		{
			ExpressionValue lvalue = ExpressionValue::Variable(frame.Variables->Data, prop);

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
		}
	}

	return result;
}

ExpressionValue Frame::CallNative(UFunction* func, UObject* instance, Array<ExpressionValue> args)
{
	// Native functions expect the last parameter to be the return value
	bool returnparmfound = false;
	int argindex = 0;
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
			ActiveCallStackFrame activeFrame(&frame);
			try
			{
				callback(instance, args.data());
			}
			catch (const std::exception& e)
			{
				LogMessage(std::string("Script error: ") + e.what());
				return ExpressionValue::NothingValue();
			}
			catch (...)
			{
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
			ActiveCallStackFrame activeFrame(&frame);
			try
			{
				callback(instance, args.data());
			}
			catch (const std::exception& e)
			{
				LogMessage(std::string("Script error: ") + e.what());
				return ExpressionValue::NothingValue();
			}
			catch (...)
			{
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

void Frame::TraceCall(UFunction* func, UObject* instance, const Array<ExpressionValue>& args)
{
#if 0 // To do: create a commandlet that lets us do this
	static NameString TraceActorClass = "CTFGame";
	static NameString TraceActorFunc = "PostBeginPlay";
	if (instance->Class->Name == TraceActorClass && func->Name == TraceActorFunc)
	{
		LogMessage("RemainingBots=" + std::to_string(instance->GetInt("RemainingBots")));
		LogMessage("InitialBots=" + std::to_string(instance->GetInt("InitialBots")));
		std::string traceMessage = "Called " + func->Name.ToString() + "(";
		bool first = true;
		for (auto& arg : args)
		{
			if (first)
				first = false;
			else
				traceMessage += ", ";
			if (arg.GetType() == ExpressionValueType::ValueString)
			{
				traceMessage += "\"";
				traceMessage += arg.ToString();
				traceMessage += "\"";
			}
			else if (arg.GetType() == ExpressionValueType::ValueName)
			{
				traceMessage += "'";
				traceMessage += arg.ToName().ToString();
				traceMessage += "'";
			}
			else if (arg.GetType() == ExpressionValueType::ValueBool)
			{
				traceMessage += "'";
				traceMessage += arg.ToBool() ? "true" : "false";
				traceMessage += "'";
			}
			else if (arg.GetType() == ExpressionValueType::ValueObject)
			{
				traceMessage += arg.ToObject() ? UObject::GetUClassName(arg.ToObject()).ToString() : "null";
			}
			else if (arg.GetType() == ExpressionValueType::Nothing)
			{
				traceMessage += "None";
			}
			else
			{
				traceMessage += "?";
			}
		}
		traceMessage += ")";
		LogMessage(traceMessage);
	}
#endif
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

	ActiveCallStackFrame activeFrame(this);

	if (!Func->Code->Statements.empty())
		StepExpression = Func->Code->Statements[StatementIndex];

	if (RunState == FrameRunState::StepInto)
	{
		// We entered a new function. Break for step into.
		Break();
	}

	const int maxInstructions = 500'000;
	int instructionsRetired = 0;
	while (true)
	{
		if (StatementIndex >= Func->Code->Statements.size())
			ThrowException("Unexpected end of code statements");

		// Note: GotoState may change StatementIndex (jump to a different location) so we have to increment the index before executing the statement
		size_t curStatementIndex = StatementIndex;
		StatementIndex++;

		StepExpression = Func->Code->Statements[curStatementIndex];

		if (instructionsRetired >= maxInstructions)
		{
			LogMessage("Too many VM instructions executed in a single tick");
			Break();
		}
		else if ((RunState == FrameRunState::StepOver || RunState == FrameRunState::StepInto) && StepFrame == this)
		{
			// We are running a new expression. Break on step over, but also step into as there might not been a function to step into.
			Break();
		}
		else if (RunState == FrameRunState::StepOut && StepFrame == nullptr)
		{
			// We found the function exit point. Break the debugger.
			Break();
		}

		Expression* statement = Func->Code->Statements[curStatementIndex];
		ExpressionEvalResult result = ExpressionEvaluator::Eval(statement, Object, Object, Variables->Data);
		//ExpressionEvalResult result = RunExpr(statement, Object, Object, Variables->Data);
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
				if (index != -1)
				{
					StatementIndex = index;
				}
				else
				{
					// State gotos can jump to a parent state block!
					bool found = false;
					for (UClass* cls = Object->Class; cls != nullptr; cls = static_cast<UClass*>(cls->BaseStruct))
					{
						UState* state = cls->GetState(Func->Name);
						if (state)
						{
							int labelIndex = state->Code->FindLabelIndex(result.Label);
							if (labelIndex != -1)
							{
								Func = state;
								StatementIndex = labelIndex;
								found = true;
								break;
							}
						}
					}
					if (!found)
						ThrowException("Could not find label: " + result.Label.ToString());
				}
			}
			break;
		case StatementResult::Stop:
			LatentState = LatentRunState::Stop;
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
						result.Value = ExpressionValue::Variable(Variables->Data, prop);
						result.Value.Load();
						break;
					}
				}
			}

			if (RunState == FrameRunState::StepOut && StepFrame == this)
			{
				// We are exiting the function. Break on next instruction by requesting a break on next instruction.
				StepFrame = nullptr;
			}
			return result;
		case StatementResult::Iterator:
			if (!result.Iter)
				ThrowException("Iterator statement without an iterator in " + Object->Name.ToString() + "." + Func->Name.ToString());
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
				ThrowException("Iterator next statement without an iterator in " + Object->Name.ToString() + "." + Func->Name.ToString());
			if (Iterators.back()->Next())
				StatementIndex = Iterators.back()->StartStatementIndex;
			else
				StatementIndex = Iterators.back()->EndStatementIndex;
			break;
		case StatementResult::IteratorPop:
			if (Iterators.empty())
				ThrowException("Iterator pop statement without an iterator in " + Object->Name.ToString() + "." + Func->Name.ToString());
			Iterators.pop_back();
			break;
		case StatementResult::AccessedNone:
			LogMessage("Accessed None");
			LogMessage(Frame::GetCallstack());
			break;
		}

		if (Object->StateFrame.get() == this && LatentState != LatentRunState::Continue)
		{
			return result;
		}

		instructionsRetired++;
	}
}

void Frame::ProcessSwitch(const ExpressionValue& condition)
{
	SwitchExpression* switchexpr = static_cast<SwitchExpression*>(Func->Code->Statements[StatementIndex - 1]);
	while (true)
	{
		CaseExpression* caseexpr = static_cast<CaseExpression*>(Func->Code->Statements[StatementIndex++]);
		if (caseexpr->Value)
		{
			ExpressionValue casevalue = ExpressionEvaluator::Eval(caseexpr->Value, Object, Object, Variables->Data).Value;
			//ExpressionValue casevalue = RunExpr(caseexpr->Value, Object, Object, Variables->Data).Value;
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

struct VMStackFrame // To do: include local variables in this
{
	Expression* expr[32];
	int pass[32];
	ExpressionValue value[32];
	UObject* context[32];
};

#define PushExpr(e) stack.pass[exprEnd] = 0; stack.expr[exprEnd++] = (e);
#define PopExpr() --exprEnd
#define PushValue(v) stack.value[valueEnd++] = (v)
#define PopValue() stack.value[--valueEnd]
#define PushContext(c) stack.context[contextEnd++] = context; context = (c)
#define PopContext() context = stack.context[--contextEnd]

ExpressionEvalResult Frame::RunExpr(Expression* statementExpr, UObject* self, UObject* context, void* localVariables)
{
	// To do: check that PushExpr + PopValue calls are in correct order
	// To do: check that the function args are in correct order

	auto oldExpr = Frame::StepExpression;

	ExpressionEvalResult result;
	int exprEnd = 0;
	int valueEnd = 0;
	int contextEnd = 0;
	VMStackFrame stack;

	PushExpr(statementExpr);

	while (exprEnd > 0)
	{
		Expression* expr = stack.expr[exprEnd - 1];
		int pass = stack.pass[exprEnd - 1]++;
		Frame::StepExpression = expr;
		switch (expr->Type)
		{
		default:
			break;
		case ExpressionType::LocalVariable:
			PushValue(ExpressionValue::Variable(localVariables, static_cast<LocalVariableExpression*>(expr)->Variable));
			PopExpr();
			break;

		case ExpressionType::InstanceVariable:
			PushValue(ExpressionValue::Variable(context->PropertyData.Data, static_cast<InstanceVariableExpression*>(expr)->Variable));
			PopExpr();
			break;

		case ExpressionType::DefaultVariable:
			if (UObject::TryCast<UClass>(context))
				PushValue(ExpressionValue::Variable(context->PropertyData.Data, static_cast<DefaultVariableExpression*>(expr)->Variable));
			else
				PushValue(ExpressionValue::Variable(context->Class->GetDefaultObject<UObject>()->PropertyData.Data, static_cast<DefaultVariableExpression*>(expr)->Variable));
			PopExpr();
			break;

		case ExpressionType::Return:
			if (pass == 0)
			{
				if (static_cast<ReturnExpression*>(expr)->Value)
				{
					PushExpr(static_cast<ReturnExpression*>(expr)->Value);
				}
				else
				{
					PushValue(ExpressionValue::NothingValue());
				}
			}
			else if (pass == 1)
			{
				result.Result = StatementResult::Return;
				PopExpr();
			}
			break;

		case ExpressionType::Switch:
			if (pass == 0)
			{
				PushExpr(static_cast<SwitchExpression*>(expr)->Condition);
			}
			else
			{
				result.Result = StatementResult::Switch;
				PopExpr();
			}
			break;

		case ExpressionType::Jump:
			result.Result = StatementResult::Jump;
			result.JumpAddress = static_cast<JumpExpression*>(expr)->Offset;
			PushValue(ExpressionValue::NothingValue());
			PopExpr();
			break;

		case ExpressionType::JumpIfNot:
			if (pass == 0)
			{
				PushExpr(static_cast<JumpIfNotExpression*>(expr)->Condition);
			}
			else
			{
				if (!PopValue().ToBool())
				{
					result.Result = StatementResult::Jump;
					result.JumpAddress = static_cast<JumpIfNotExpression*>(expr)->Offset;
				}
				PopExpr();
			}
			break;

		case ExpressionType::Stop:
			result.Result = StatementResult::Stop;
			PushValue(ExpressionValue::NothingValue());
			PopExpr();
			break;

		case ExpressionType::Assert:
			if (pass == 0)
			{
				PushExpr(static_cast<AssertExpression*>(expr)->Condition);
			}
			else
			{
				if (!PopValue().ToBool())
				{
					Frame::ThrowException("Script assert failed for " + self->Name.ToString() + " line " + std::to_string(static_cast<AssertExpression*>(expr)->Line));
				}
				PushValue(ExpressionValue::NothingValue());
				PopExpr();
			}
			break;

		case ExpressionType::Case:
			PushValue(ExpressionValue::NothingValue());
			PopExpr();
			break;

		case ExpressionType::Nothing:
			PushValue(ExpressionValue::NothingValue());
			PopExpr();
			break;

		case ExpressionType::LabelTable:
			// Klingon honor guard has this! (UE 251)
			Frame::ThrowException("Label table expression is not implemented");
			PushValue(ExpressionValue::NothingValue());
			PopExpr();
			break;

		case ExpressionType::GotoLabel:
			if (pass == 0)
			{
				PushExpr(static_cast<GotoLabelExpression*>(expr)->Value);
			}
			else
			{
				result.Result = StatementResult::GotoLabel;
				result.Label = PopValue().ToName();
				PopExpr();
			}
			break;

		case ExpressionType::EatString:
			if (pass == 0)
			{
				PushExpr(static_cast<EatStringExpression*>(expr)->Value);
			}
			else
			{
				PopValue();
				PushValue(ExpressionValue::NothingValue());
				PopExpr();
			}
			break;

		case ExpressionType::Let:
			if (pass == 0)
			{
				PushExpr(static_cast<LetExpression*>(expr)->LeftSide);
				PushExpr(static_cast<LetExpression*>(expr)->RightSide);
			}
			else
			{
				ExpressionValue lvalue = std::move(PopValue());
				ExpressionValue rvalue = std::move(PopValue());
				if (lvalue.GetType() != ExpressionValueType::Nothing)
				{
					lvalue.Store(rvalue);
					PushValue(std::move(lvalue));
				}
				else
				{
					PushValue(std::move(rvalue));
				}
				PopExpr();
			}
			break;

		case ExpressionType::LetBool:
			if (pass == 0)
			{
				PushExpr(static_cast<LetBoolExpression*>(expr)->LeftSide);
				PushExpr(static_cast<LetBoolExpression*>(expr)->RightSide);
			}
			else
			{
				ExpressionValue lvalue = std::move(PopValue());
				ExpressionValue rvalue = std::move(PopValue());
				if (lvalue.GetType() != ExpressionValueType::Nothing)
				{
					lvalue.Store(rvalue);
					PushValue(std::move(lvalue));
				}
				else
				{
					PushValue(std::move(rvalue));
				}
				PopExpr();
			}
			break;

		case ExpressionType::DynArrayElement:
			if (pass == 0)
			{
				PushExpr(static_cast<DynArrayElementExpression*>(expr)->Index);
				PushExpr(static_cast<DynArrayElementExpression*>(expr)->Array);
			}
			else
			{
				int index = PopValue().ToInt();
				ExpressionValue arrayval = std::move(PopValue());
				if (arrayval.IsVariable())
				{
					if (index < 0)
					{
						LogMessage("Negative index used");
						PushValue(ExpressionValue::NothingValue());
					}
					else
					{
						PushValue(arrayval.DynArrayItemAt(index));
					}
				}
				else
				{
					Frame::ThrowException("Array is not a variable in DynArrayElementExpression");
					PushValue(ExpressionValue::NothingValue());
				}
				PopExpr();
			}
			break;

		case ExpressionType::New:
			if (pass == 0)
			{
				PushExpr(static_cast<NewExpression*>(expr)->ParentExpr);
				PushExpr(static_cast<NewExpression*>(expr)->NameExpr);
				PushExpr(static_cast<NewExpression*>(expr)->FlagsExpr);
				PushExpr(static_cast<NewExpression*>(expr)->ClassExpr);
			}
			else
			{
				auto newExpr = static_cast<NewExpression*>(expr);
				ExpressionValue outer = std::move(PopValue());
				ExpressionValue name = std::move(PopValue());
				ExpressionValue flags = std::move(PopValue());
				UClass* cls = UObject::Cast<UClass>(PopValue().ToObject());

				// To do: package needs to be grabbed from outer, or the "transient package" if it is None, a virtual package for runtime objects
				Package* package = engine->packages->GetPackage("Engine");

				UObject* newObj = package->NewObject(
					name.GetType() == ExpressionValueType::Nothing ? NameString() : name.ToName(),
					cls,
					flags.GetType() == ExpressionValueType::Nothing ? ObjectFlags::NoFlags : (ObjectFlags)flags.ToInt(),
					true);

				if (outer.GetType() != ExpressionValueType::Nothing)
					newObj->Outer() = outer.ToObject();

				PushValue(ExpressionValue::ObjectValue(newObj));
				PopExpr();
			}
			break;

		case ExpressionType::ClassContext:
			if (pass == 0)
			{
				PushExpr(static_cast<ClassContextExpression*>(expr)->ObjectExpr);
			}
			else if (pass == 1)
			{
				UClass* cls = UObject::TryCast<UClass>(PopValue().ToObject());
				if (cls)
				{
					PushContext(cls->GetDefaultObject<UObject>());
					PushExpr(static_cast<ClassContextExpression*>(expr)->ContextExpr);
				}
				else
				{
					Frame::ThrowException("Class reference is None");
					PushValue(ExpressionValue::NothingValue());
					PopExpr();
				}
			}
			else
			{
				PopContext();
				PopExpr();
			}
			break;

		case ExpressionType::MetaCast:
			if (pass == 0)
			{
				PushExpr(static_cast<MetaCastExpression*>(expr)->Value);
			}
			else
			{
				auto castExpr = static_cast<MetaCastExpression*>(expr);
				UObject* value = PopValue().ToObject();
				if (value && value != castExpr->Class)
				{
					UClass* cls = UObject::TryCast<UClass>(value);
					while (cls)
					{
						if (cls == castExpr->Class)
							break;
						cls = static_cast<UClass*>(cls->BaseStruct);
					}
					if (!cls)
						value = nullptr;
				}
				PushValue(ExpressionValue::ObjectValue(value));
				PopExpr();
			}
			break;

		case ExpressionType::Unknown0x15:
			// Klingon honor guard has this! (UE 251)
			//Frame::ThrowException("Unknown0x15 expression encountered");
			result.Result = StatementResult::Stop;
			PushValue(ExpressionValue::NothingValue());
			PopExpr();
			break;

		case ExpressionType::Self:
			PushValue(ExpressionValue::ObjectValue(self));
			PopExpr();
			break;

		case ExpressionType::Skip:
			if (pass == 0)
			{
				PushExpr(static_cast<SkipExpression*>(expr)->Value);
			}
			else
			{
				PopExpr();
			}
			break;

		case ExpressionType::Context:
			if (pass == 0)
			{
				PushExpr(static_cast<ContextExpression*>(expr)->ObjectExpr);
			}
			else if (pass == 1)
			{
				UObject* newContext = PopValue().ToObject();
				if (newContext)
				{
					PushContext(newContext);
					PushExpr(static_cast<ContextExpression*>(expr)->ContextExpr);
				}
				else
				{
					result.Result = StatementResult::AccessedNone;
					PushValue(ExpressionValue::NothingValue());
					PopExpr();
				}
			}
			else
			{
				PopContext();
				PopExpr();
			}
			break;

		case ExpressionType::ArrayElement:
			if (pass == 0)
			{
				PushExpr(static_cast<ArrayElementExpression*>(expr)->Index);
				PushExpr(static_cast<ArrayElementExpression*>(expr)->Array);
			}
			else
			{
				int index = PopValue().ToInt();
				ExpressionValue arrayval = std::move(PopValue());
				if (arrayval.IsVariable())
				{
					PushValue(arrayval.ItemAt(index));
				}
				else
				{
					Frame::ThrowException("Array is not a variable in ArrayElementExpression");
					PushValue(ExpressionValue::NothingValue());
				}
				PopExpr();
			}
			break;

		case ExpressionType::IntConst:
			PushValue(ExpressionValue::IntValue(static_cast<IntConstExpression*>(expr)->Value));
			PopExpr();
			break;

		case ExpressionType::FloatConst:
			PushValue(ExpressionValue::FloatValue(static_cast<FloatConstExpression*>(expr)->Value));
			PopExpr();
			break;

		case ExpressionType::StringConst:
			PushValue(ExpressionValue::StringValue(static_cast<StringConstExpression*>(expr)->Value));
			PopExpr();
			break;

		case ExpressionType::ObjectConst:
			PushValue(ExpressionValue::ObjectValue(static_cast<ObjectConstExpression*>(expr)->Object));
			PopExpr();
			break;

		case ExpressionType::NameConst:
			PushValue(ExpressionValue::NameValue(static_cast<NameConstExpression*>(expr)->Value));
			PopExpr();
			break;

		case ExpressionType::RotationConst:
			PushValue(ExpressionValue::RotatorValue({
				static_cast<RotationConstExpression*>(expr)->Pitch,
				static_cast<RotationConstExpression*>(expr)->Yaw,
				static_cast<RotationConstExpression*>(expr)->Roll
				}));
			PopExpr();
			break;

		case ExpressionType::VectorConst:
			PushValue(ExpressionValue::VectorValue({
				static_cast<VectorConstExpression*>(expr)->X,
				static_cast<VectorConstExpression*>(expr)->Y,
				static_cast<VectorConstExpression*>(expr)->Z
				}));
			PopExpr();
			break;

		case ExpressionType::ByteConst:
			PushValue(ExpressionValue::ByteValue(static_cast<ByteConstExpression*>(expr)->Value));
			PopExpr();
			break;

		case ExpressionType::IntZero:
			PushValue(ExpressionValue::IntValue(0));
			PopExpr();
			break;

		case ExpressionType::IntOne:
			PushValue(ExpressionValue::IntValue(1));
			PopExpr();
			break;

		case ExpressionType::True:
			PushValue(ExpressionValue::BoolValue(true));
			PopExpr();
			break;

		case ExpressionType::False:
			PushValue(ExpressionValue::BoolValue(false));
			PopExpr();
			break;

		case ExpressionType::NativeParm:
			Frame::ThrowException("Native parm expression is not implemented");
			PopExpr();
			break;

		case ExpressionType::NoObject:
			PushValue(ExpressionValue::ObjectValue(nullptr));
			PopExpr();
			break;

		case ExpressionType::Unknown0x2b:
			if (pass == 0)
			{
				PushExpr(static_cast<Unknown0x2bExpression*>(expr)->Value);
			}
			else
			{
				// This may have been a truncating instruction from back when strings had a fixed size (package version 61 and earlier)
				PopExpr();
			}
			break;

		case ExpressionType::IntConstByte:
			PushValue(ExpressionValue::ByteValue(static_cast<IntConstByteExpression*>(expr)->Value));
			PopExpr();
			break;

		case ExpressionType::BoolVariable:
			if (pass == 0)
			{
				PushExpr(static_cast<BoolVariableExpression*>(expr)->Variable);
			}
			else
			{
				PopExpr();
			}
			break;

		case ExpressionType::DynamicCast:
			if (pass == 0)
			{
				PushExpr(static_cast<DynamicCastExpression*>(expr)->Value);
			}
			else
			{
				UObject* value = PopValue().ToObject();
				if (value && !value->IsA(static_cast<DynamicCastExpression*>(expr)->Class->Name))
					value = nullptr;
				PushValue(ExpressionValue::ObjectValue(value));
				PopExpr();
			}
			break;

		case ExpressionType::Iterator:
			if (pass == 0)
			{
				PushExpr(static_cast<IteratorExpression*>(expr)->Value);
			}
			else
			{
				result.Result = StatementResult::Iterator;
				result.Iter = std::move(Frame::CreatedIterator);
				result.JumpAddress = static_cast<IteratorExpression*>(expr)->Offset;
				PopExpr();
			}
			break;

		case ExpressionType::IteratorPop:
			result.Result = StatementResult::IteratorPop;
			PushValue(ExpressionValue::NothingValue());
			PopExpr();
			break;

		case ExpressionType::IteratorNext:
			result.Result = StatementResult::IteratorNext;
			PushValue(ExpressionValue::NothingValue());
			PopExpr();
			break;

		case ExpressionType::StructCmpEq:
			if (pass == 0)
			{
				PushExpr(static_cast<StructCmpEqExpression*>(expr)->Value1);
				PushExpr(static_cast<StructCmpEqExpression*>(expr)->Value2);
			}
			else
			{
				ExpressionValue val1 = std::move(PopValue());
				ExpressionValue val2 = std::move(PopValue());
				PushValue(ExpressionValue::BoolValue(val1.IsEqual(val2)));
				PopExpr();
			}
			break;

		case ExpressionType::StructCmpNe:
			if (pass == 0)
			{
				PushExpr(static_cast<StructCmpNeExpression*>(expr)->Value1);
				PushExpr(static_cast<StructCmpNeExpression*>(expr)->Value2);
			}
			else
			{
				ExpressionValue val1 = std::move(PopValue());
				ExpressionValue val2 = std::move(PopValue());
				PushValue(ExpressionValue::BoolValue(!val1.IsEqual(val2)));
				PopExpr();
			}
			break;

		case ExpressionType::StructMember:
			if (pass == 0)
			{
				if (!static_cast<StructMemberExpression*>(expr)->Field)
					Frame::ThrowException("Null field encountered in struct member expression");
				PushExpr(static_cast<StructMemberExpression*>(expr)->Value);
			}
			else
			{
				ExpressionValue val = std::move(PopValue());
				PushValue(val.Member(static_cast<StructMemberExpression*>(expr)->Field));
				PopExpr();
			}
			break;

		case ExpressionType::UnicodeStringConst:
		{
			std::string s;
			s.reserve(static_cast<UnicodeStringConstExpression*>(expr)->Value.size());
			for (wchar_t c : static_cast<UnicodeStringConstExpression*>(expr)->Value)
				s.push_back(c < 128 ? c : '?');
			PushValue(ExpressionValue::StringValue(s));
			PopExpr();
			break;
		}

		case ExpressionType::RotatorToVector:
			if (pass == 0)
			{
				PushExpr(static_cast<RotatorToVectorExpression*>(expr)->Value);
			}
			else
			{
				Rotator rot = PopValue().ToRotator();
				PushValue(ExpressionValue::VectorValue(Coords::Rotation(rot).XAxis));
				PopExpr();
			}
			break;

		case ExpressionType::ByteToInt:
			if (pass == 0)
			{
				PushExpr(static_cast<ByteToIntExpression*>(expr)->Value);
			}
			else
			{
				auto val = PopValue().ToByte();
				PushValue(ExpressionValue::IntValue(val));
				PopExpr();
			}
			break;

		case ExpressionType::ByteToBool:
			if (pass == 0)
			{
				PushExpr(static_cast<ByteToBoolExpression*>(expr)->Value);
			}
			else
			{
				auto val = PopValue().ToByte() != 0;
				PushValue(ExpressionValue::BoolValue(val));
				PopExpr();
			}
			break;

		case ExpressionType::ByteToFloat:
			if (pass == 0)
			{
				PushExpr(static_cast<ByteToFloatExpression*>(expr)->Value);
			}
			else
			{
				auto val = PopValue().ToByte();
				PushValue(ExpressionValue::FloatValue(val));
				PopExpr();
			}
			break;

		case ExpressionType::IntToByte:
			if (pass == 0)
			{
				PushExpr(static_cast<IntToByteExpression*>(expr)->Value);
			}
			else
			{
				auto val = PopValue().ToInt();
				PushValue(ExpressionValue::ByteValue(val));
				PopExpr();
			}
			break;

		case ExpressionType::IntToBool:
			if (pass == 0)
			{
				PushExpr(static_cast<IntToBoolExpression*>(expr)->Value);
			}
			else
			{
				auto val = PopValue().ToInt();
				PushValue(ExpressionValue::BoolValue(val));
				PopExpr();
			}
			break;

		case ExpressionType::IntToFloat:
			if (pass == 0)
			{
				PushExpr(static_cast<IntToFloatExpression*>(expr)->Value);
			}
			else
			{
				auto val = PopValue().ToInt();
				PushValue(ExpressionValue::FloatValue((float)val));
				PopExpr();
			}
			break;

		case ExpressionType::BoolToByte:
			if (pass == 0)
			{
				PushExpr(static_cast<BoolToByteExpression*>(expr)->Value);
			}
			else
			{
				auto val = PopValue().ToBool();
				PushValue(ExpressionValue::ByteValue(val));
				PopExpr();
			}
			break;

		case ExpressionType::BoolToInt:
			if (pass == 0)
			{
				PushExpr(static_cast<BoolToIntExpression*>(expr)->Value);
			}
			else
			{
				auto val = PopValue().ToBool();
				PushValue(ExpressionValue::IntValue(val));
				PopExpr();
			}
			break;

		case ExpressionType::BoolToFloat:
			if (pass == 0)
			{
				PushExpr(static_cast<BoolToFloatExpression*>(expr)->Value);
			}
			else
			{
				auto val = PopValue().ToBool();
				PushValue(ExpressionValue::FloatValue(val));
				PopExpr();
			}
			break;

		case ExpressionType::FloatToByte:
			if (pass == 0)
			{
				PushExpr(static_cast<FloatToByteExpression*>(expr)->Value);
			}
			else
			{
				auto val = PopValue().ToFloat();
				PushValue(ExpressionValue::ByteValue((uint8_t)val));
				PopExpr();
			}
			break;

		case ExpressionType::FloatToInt:
			if (pass == 0)
			{
				PushExpr(static_cast<FloatToIntExpression*>(expr)->Value);
			}
			else
			{
				auto val = PopValue().ToFloat();
				PushValue(ExpressionValue::IntValue((int)val));
				PopExpr();
			}
			break;

		case ExpressionType::FloatToBool:
			if (pass == 0)
			{
				PushExpr(static_cast<FloatToBoolExpression*>(expr)->Value);
			}
			else
			{
				auto val = PopValue().ToFloat();
				PushValue(ExpressionValue::BoolValue(val));
				PopExpr();
			}
			break;

		case ExpressionType::Unknown0x46:
			Frame::ThrowException("Unknown0x46 expression encountered");
			break;

		case ExpressionType::ObjectToBool:
			if (pass == 0)
			{
				PushExpr(static_cast<ObjectToBoolExpression*>(expr)->Value);
			}
			else
			{
				auto val = PopValue().ToObject() != nullptr;
				PushValue(ExpressionValue::BoolValue(val));
				PopExpr();
			}
			break;

		case ExpressionType::NameToBool:
			if (pass == 0)
			{
				PushExpr(static_cast<NameToBoolExpression*>(expr)->Value);
			}
			else
			{
				auto val = PopValue().ToName() != "None";
				PushValue(ExpressionValue::BoolValue(val));
				PopExpr();
			}
			break;

		case ExpressionType::StringToByte:
			if (pass == 0)
			{
				PushExpr(static_cast<StringToByteExpression*>(expr)->Value);
			}
			else
			{
				auto val = std::atoi(PopValue().ToString().c_str());
				PushValue(ExpressionValue::ByteValue(val));
				PopExpr();
			}
			break;

		case ExpressionType::StringToInt:
			if (pass == 0)
			{
				PushExpr(static_cast<StringToIntExpression*>(expr)->Value);
			}
			else
			{
				auto val = std::atoi(PopValue().ToString().c_str());
				PushValue(ExpressionValue::IntValue(val));
				PopExpr();
			}
			break;

		case ExpressionType::StringToBool:
			if (pass == 0)
			{
				PushExpr(static_cast<StringToBoolExpression*>(expr)->Value);
			}
			else
			{
				auto val = std::atoi(PopValue().ToString().c_str()) != 0;
				PushValue(ExpressionValue::BoolValue(val));
				PopExpr();
			}
			break;

		case ExpressionType::StringToFloat:
			if (pass == 0)
			{
				PushExpr(static_cast<StringToFloatExpression*>(expr)->Value);
			}
			else
			{
				auto val = (float)std::atof(PopValue().ToString().c_str());
				PushValue(ExpressionValue::FloatValue(val));
				PopExpr();
			}
			break;

		case ExpressionType::StringToVector:
			if (pass == 0)
			{
				PushExpr(static_cast<StringToVectorExpression*>(expr)->Value);
			}
			else
			{
				std::string v = PopValue().ToString();
				auto pos1 = v.find_first_of(',');
				auto pos2 = v.find_first_of(',', pos1 + 1);
				if (pos1 != std::string::npos && pos2 != std::string::npos)
				{
					PushValue(ExpressionValue::VectorValue({
						(float)std::atof(v.substr(0, pos1).c_str()),
						(float)std::atof(v.substr(pos1 + 1, pos2 - pos1 - 1).c_str()),
						(float)std::atof(v.substr(pos2 + 1).c_str())
						}));
				}
				else
				{
					PushValue(ExpressionValue::VectorValue({ 0.0f }));
				}
				PopExpr();
			}
			break;

		case ExpressionType::StringToRotator:
			if (pass == 0)
			{
				PushExpr(static_cast<StringToRotatorExpression*>(expr)->Value);
			}
			else
			{
				std::string v = PopValue().ToString();
				auto pos1 = v.find_first_of(',');
				auto pos2 = v.find_first_of(',', pos1 + 1);
				if (pos1 != std::string::npos && pos2 != std::string::npos)
				{
					PushValue(ExpressionValue::RotatorValue({
						std::atoi(v.substr(0, pos1).c_str()),
						std::atoi(v.substr(pos1 + 1, pos2 - pos1 - 1).c_str()),
						std::atoi(v.substr(pos2 + 1).c_str())
						}));
				}
				else
				{
					PushValue(ExpressionValue::RotatorValue({ 0, 0, 0 }));
				}
				PopExpr();
				break;
			}
			break;

		case ExpressionType::VectorToBool:
			if (pass == 0)
			{
				PushExpr(static_cast<VectorToBoolExpression*>(expr)->Value);
			}
			else
			{
				vec3 v = PopValue().ToVector();
				PushValue(ExpressionValue::BoolValue(v != vec3(0.0f)));
				PopExpr();
			}
			break;

		case ExpressionType::VectorToRotator:
			if (pass == 0)
			{
				PushExpr(static_cast<VectorToRotatorExpression*>(expr)->Value);
			}
			else
			{
				vec3 v = PopValue().ToVector();
				PushValue(ExpressionValue::RotatorValue(Rotator::FromVector(v)));
				PopExpr();
			}
			break;

		case ExpressionType::RotatorToBool:
			if (pass == 0)
			{
				PushExpr(static_cast<RotatorToBoolExpression*>(expr)->Value);
			}
			else
			{
				bool v = PopValue().ToRotator() != Rotator(0, 0, 0);
				PushValue(ExpressionValue::BoolValue(v));
				PopExpr();
			}
			break;

		case ExpressionType::ByteToString:
			if (pass == 0)
			{
				PushExpr(static_cast<ByteToStringExpression*>(expr)->Value);
			}
			else
			{
				auto v = PopValue().ToByte();
				PushValue(ExpressionValue::StringValue(std::to_string(v)));
				PopExpr();
			}
			break;

		case ExpressionType::IntToString:
			if (pass == 0)
			{
				PushExpr(static_cast<IntToStringExpression*>(expr)->Value);
			}
			else
			{
				auto v = PopValue().ToInt();
				PushValue(ExpressionValue::StringValue(std::to_string(v)));
				PopExpr();
			}
			break;

		case ExpressionType::BoolToString:
			if (pass == 0)
			{
				PushExpr(static_cast<BoolToStringExpression*>(expr)->Value);
			}
			else
			{
				auto v = PopValue().ToBool();
				PushValue(ExpressionValue::StringValue(std::to_string(v)));
				PopExpr();
			}
			break;

		case ExpressionType::FloatToString:
			if (pass == 0)
			{
				PushExpr(static_cast<FloatToStringExpression*>(expr)->Value);
			}
			else
			{
				auto v = PopValue().ToFloat();
				PushValue(ExpressionValue::StringValue(std::to_string(v)));
				PopExpr();
			}
			break;

		case ExpressionType::ObjectToString:
			if (pass == 0)
			{
				PushExpr(static_cast<ObjectToStringExpression*>(expr)->Value);
			}
			else
			{
				UObject* obj = PopValue().ToObject();
				PushValue(ExpressionValue::StringValue(obj ? obj->package->GetPackageName().ToString() + "." + obj->Name.ToString() : "None"));
				PopExpr();
			}
			break;

		case ExpressionType::NameToString:
			if (pass == 0)
			{
				PushExpr(static_cast<NameToStringExpression*>(expr)->Value);
			}
			else
			{
				NameString v = PopValue().ToName();
				PushValue(ExpressionValue::StringValue(v.ToString()));
				PopExpr();
			}
			break;

		case ExpressionType::VectorToString:
			if (pass == 0)
			{
				PushExpr(static_cast<VectorToStringExpression*>(expr)->Value);
			}
			else
			{
				vec3 v = PopValue().ToVector();
				PushValue(ExpressionValue::StringValue(std::to_string(v.x) + "," + std::to_string(v.y) + "," + std::to_string(v.z)));
				PopExpr();
			}
			break;

		case ExpressionType::RotatorToString:
			if (pass == 0)
			{
				PushExpr(static_cast<RotatorToStringExpression*>(expr)->Value);
			}
			else
			{
				Rotator v = PopValue().ToRotator();
				PushValue(ExpressionValue::StringValue(std::to_string(v.Pitch & 0xffff) + "," + std::to_string(v.Yaw & 0xffff) + "," + std::to_string(v.Roll & 0xffff)));
				PopExpr();
			}
			break;

		case ExpressionType::StringToName:
			if (pass == 0)
			{
				PushExpr(static_cast<StringToNameExpression*>(expr)->Value);
			}
			else
			{
				std::string v = PopValue().ToString();
				PushValue(ExpressionValue::NameValue(v));
				PopExpr();
			}
			break;

		case ExpressionType::DynArrayToInt:
			if (pass == 0)
			{
				PushExpr(static_cast<DynArrayToIntExpression*>(expr)->Value);
			}
			else
			{
				size_t count = PopValue().ToArray().GetSize();
				PushValue(ExpressionValue::IntValue((int)count));
				PopExpr();
			}
			break;

		case ExpressionType::VirtualFunction:
			if (pass == 0)
			{
				const auto& args = static_cast<VirtualFunctionExpression*>(expr)->Args;
				for (auto it = args.rbegin(); it != args.rend(); ++it)
				{
					PushExpr(*it);
				}
			}
			else
			{
				auto funcExpr = static_cast<VirtualFunctionExpression*>(expr);
				ArrayView<ExpressionValue> funcArgs(&stack.value[valueEnd - funcExpr->Args.size()], funcExpr->Args.size());
				valueEnd -= (int)funcArgs.size();

				UClass* contextClass = UObject::TryCast<UClass>(context);
				if (!contextClass)
					contextClass = context->Class;

				// Search states first

				NameString stateName = context->GetStateName();
				bool found = false;
				for (UClass* cls = contextClass; cls != nullptr; cls = static_cast<UClass*>(cls->BaseStruct))
				{
					UState* state = cls->GetState(stateName);
					if (state)
					{
						UFunction* func = state->GetFunction(funcExpr->Name);
						if (func)
						{
							PushValue(Frame::Call(func, context, funcArgs));
							PopExpr();
							found = true;
							break;
						}
					}
				}

				if (!found)
				{
					// Search normal member functions next
					for (UClass* cls = contextClass; !found && cls != nullptr; cls = static_cast<UClass*>(cls->BaseStruct))
					{
						for (UField* field = cls->Children; field != nullptr; field = field->Next)
						{
							UFunction* func = UObject::TryCast<UFunction>(field);
							if (func && func->Name == funcExpr->Name)
							{
								PushValue(Frame::Call(func, context, funcArgs));
								PopExpr();
								found = true;
								break;
							}
						}
					}
				}

				if (!found)
				{
					Frame::ThrowException("Script virtual function " + funcExpr->Name.ToString() + " not found!");
					PushValue(ExpressionValue::NothingValue());
				}
			}
			break;

		case ExpressionType::FinalFunction:
			if (pass == 0)
			{
				const auto& args = static_cast<VirtualFunctionExpression*>(expr)->Args;
				for (auto it = args.rbegin(); it != args.rend(); ++it)
				{
					PushExpr(*it);
				}
			}
			else
			{
				auto funcExpr = static_cast<FinalFunctionExpression*>(expr);
				ArrayView<ExpressionValue> funcArgs(&stack.value[valueEnd - funcExpr->Args.size()], funcExpr->Args.size());
				valueEnd -= (int)funcArgs.size();

				PushValue(Frame::Call(funcExpr->Func, context, funcArgs));
				PopExpr();
			}
			break;

		case ExpressionType::GlobalFunction:
			if (pass == 0)
			{
				const auto& args = static_cast<GlobalFunctionExpression*>(expr)->Args;
				for (auto it = args.rbegin(); it != args.rend(); ++it)
				{
					PushExpr(*it);
				}
			}
			else
			{
				auto funcExpr = static_cast<GlobalFunctionExpression*>(expr);
				ArrayView<ExpressionValue> funcArgs(&stack.value[valueEnd - funcExpr->Args.size()], funcExpr->Args.size());
				valueEnd -= (int)funcArgs.size();

				// Global function calls skip the states and only searches normal member functions

				UClass* contextClass = UObject::TryCast<UClass>(context);
				if (!contextClass)
					contextClass = context->Class;

				bool found = false;
				for (UClass* cls = contextClass; cls != nullptr; cls = static_cast<UClass*>(cls->BaseStruct))
				{
					UFunction* func = cls->GetFunction(funcExpr->Name);
					if (func)
					{
						PushValue(Frame::Call(func, context, funcArgs));
						PopExpr();
						found = true;
						break;
					}
				}

				if (!found)
					Frame::ThrowException("Script global function " + static_cast<GlobalFunctionExpression*>(expr)->Name.ToString() + " not found!");
			}
			break;

		case ExpressionType::NativeFunction:
			// To do: conditional operator should probably be handled by the skip expression
			if (static_cast<NativeFunctionExpression*>(expr)->nativeindex == 130) // conditional operator &&
			{
				auto funcExpr = static_cast<NativeFunctionExpression*>(expr);
				if (pass == 0)
				{
					PushExpr(funcExpr->Args[0]);
				}
				else if (pass == 1)
				{
					if (PopValue().ToBool())
					{
						PushExpr(funcExpr->Args[1]);
					}
					else
					{
						PushValue(ExpressionValue::BoolValue(false));
						PopExpr();
					}
				}
				else
				{
					bool value = PopValue().ToBool();
					PushValue(ExpressionValue::BoolValue(value));
					PopExpr();
				}
			}
			else if (static_cast<NativeFunctionExpression*>(expr)->nativeindex == 132) // conditional operator ||
			{
				auto funcExpr = static_cast<NativeFunctionExpression*>(expr);
				if (pass == 0)
				{
					PushExpr(funcExpr->Args[0]);
				}
				else if (pass == 1)
				{
					if (!PopValue().ToBool())
					{
						PushExpr(funcExpr->Args[1]);
					}
					else
					{
						PushValue(ExpressionValue::BoolValue(true));
						PopExpr();
					}
				}
				else
				{
					bool value = PopValue().ToBool();
					PushValue(ExpressionValue::BoolValue(value));
					PopExpr();
				}
			}
			else
			{
				if (pass == 0)
				{
					const auto& args = static_cast<NativeFunctionExpression*>(expr)->Args;
					for (auto it = args.rbegin(); it != args.rend(); ++it)
					{
						PushExpr(*it);
					}
				}
				else
				{
					auto funcExpr = static_cast<NativeFunctionExpression*>(expr);
					ArrayView<ExpressionValue> funcArgs(&stack.value[valueEnd - funcExpr->Args.size()], funcExpr->Args.size());
					valueEnd -= (int)funcArgs.size();

					PushValue(Frame::Call(NativeFunctions::FuncByIndex[funcExpr->nativeindex], context, funcArgs));
					PopExpr();
				}
			}
			break;

		case ExpressionType::Construct:
			Frame::ThrowException("Construct expression not implemented");
			PushValue(ExpressionValue::NothingValue());
			PopExpr();
			break;

		case ExpressionType::FunctionArguments:
			PushValue(ExpressionValue::NothingValue());
			PopExpr();
			break;
		}
	}

	Frame::StepExpression = oldExpr;
	if (valueEnd > 0)
		result.Value = std::move(PopValue());
	return result;
}

/////////////////////////////////////////////////////////////////////////////

LocalVariables::LocalVariables(UStruct* func) : Func(func)
{
	if (func)
	{
		Data = AlignedAlloc(func->StructAlignment, func->StructSize);

		for (UProperty* prop : func->Properties)
		{
			prop->ConstructArray(static_cast<uint8_t*>(Data) + prop->DataOffset.DataOffset);
		}
	}
}

LocalVariables::~LocalVariables()
{
	if (Func && Data)
	{
		for (UProperty* prop : Func->Properties)
		{
			prop->DestructArray(static_cast<uint8_t*>(Data) + prop->DataOffset.DataOffset);
		}
	}

	AlignedFree(Data);
}
