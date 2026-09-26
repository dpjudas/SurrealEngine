
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

#if 0

ExpressionEvalResult Frame::RunExpr(Expression* statementExpr, UObject* self, UObject* context, void* localVariables)
{
	auto oldExpr = Frame::StepExpression;

	ExpressionEvalResult result;
	Expression* exprStack[64];
	Expression* expr;
	int exprIndex = 0;
	exprStack[exprIndex++] = statementExpr;

	while (exprIndex > 0)
	{
		exprIndex--;
		expr = exprStack[exprIndex];
		Frame::StepExpression = expr;
		switch (expr->Type)
		{
		default:
			break;
		case ExpressionType::LocalVariable:
			result.Value = ExpressionValue::Variable(localVariables, static_cast<LocalVariableExpression*>(expr)->Variable);
			break;

		case ExpressionType::InstanceVariable:
			result.Value = ExpressionValue::Variable(context->PropertyData.Data, static_cast<InstanceVariableExpression*>(expr)->Variable);
			break;

		case ExpressionType::DefaultVariable:
			if (UObject::TryCast<UClass>(context))
				result.Value = ExpressionValue::Variable(context->PropertyData.Data, static_cast<DefaultVariableExpression*>(expr)->Variable);
			else
				result.Value = ExpressionValue::Variable(context->Class->GetDefaultObject<UObject>()->PropertyData.Data, static_cast<DefaultVariableExpression*>(expr)->Variable);
			break;

		case ExpressionType::Return:
			if (static_cast<ReturnExpression*>(expr)->Value)
				result.Value = Eval(static_cast<ReturnExpression*>(expr)->Value).Value;
			else
				result.Value = ExpressionValue::NothingValue();
			result.Result = StatementResult::Return;
			break;

		case ExpressionType::Switch:
			result.Value = Eval(static_cast<SwitchExpression*>(expr)->Condition).Value;
			result.Result = StatementResult::Switch;
			break;

		case ExpressionType::Jump:
			result.Result = StatementResult::Jump;
			result.JumpAddress = static_cast<JumpExpression*>(expr)->Offset;
			break;

		case ExpressionType::JumpIfNot:
			if (!Eval(static_cast<JumpIfNotExpression*>(expr)->Condition).Value.ToBool())
			{
				result.Result = StatementResult::Jump;
				result.JumpAddress = static_cast<JumpIfNotExpression*>(expr)->Offset;
			}
			break;

		case ExpressionType::Stop:
			result.Result = StatementResult::Stop;
			break;

		case ExpressionType::Assert:
			if (!Eval(static_cast<AssertExpression*>(expr)->Condition).Value.ToBool())
			{
				Frame::ThrowException("Script assert failed for " + self->Name.ToString() + " line " + std::to_string(static_cast<AssertExpression*>(expr)->Line));
			}
			break;

		case ExpressionType::Case:
			result.Value = ExpressionValue::NothingValue();
			break;

		case ExpressionType::Nothing:
			result.Value = ExpressionValue::NothingValue();
			break;

		case ExpressionType::LabelTable:
			// Klingon honor guard has this! (UE 251)
			Frame::ThrowException("Label table expression is not implemented");
			break;

		case ExpressionType::GotoLabel:
			result.Result = StatementResult::GotoLabel;
			result.Label = Eval(static_cast<GotoLabelExpression*>(expr)->Value).Value.ToName();
			break;

		case ExpressionType::EatString:
			Eval(static_cast<EatStringExpression*>(expr)->Value);
			result.Value = ExpressionValue::NothingValue();
			break;

		case ExpressionType::Let:
		{
			ExpressionValue lvalue = Eval(static_cast<LetExpression*>(expr)->LeftSide).Value;
			ExpressionValue rvalue = Eval(static_cast<LetExpression*>(expr)->RightSide).Value;
			if (lvalue.GetType() != ExpressionValueType::Nothing)
			{
				lvalue.Store(rvalue);
				result.Value = std::move(lvalue);
			}
			else
			{
				result.Value = std::move(rvalue);
			}
			break;
		}

		case ExpressionType::LetBool:
		{
			ExpressionValue lvalue = Eval(static_cast<LetBoolExpression*>(expr)->LeftSide).Value;
			ExpressionValue rvalue = Eval(static_cast<LetBoolExpression*>(expr)->RightSide).Value;
			if (lvalue.GetType() != ExpressionValueType::Nothing)
			{
				lvalue.Store(rvalue);
				result.Value = std::move(lvalue);
			}
			else
			{
				result.Value = std::move(rvalue);
			}
			break;
		}

		case ExpressionType::DynArrayElement:
		{
			int index = Eval(static_cast<DynArrayElementExpression*>(expr)->Index).Value.ToInt();
			auto arrayval = Eval(static_cast<DynArrayElementExpression*>(expr)->Array).Value;
			if (arrayval.IsVariable())
			{
				if (index < 0)
				{
					LogMessage("Negative index used");
					result.Value = ExpressionValue::NothingValue();
				}
				else
				{
					result.Value = arrayval.DynArrayItemAt(index);
				}
			}
			else
			{
				Frame::ThrowException("Array is not a variable in DynArrayElementExpression");
			}
			break;
		}

		case ExpressionType::New:
		{
			auto newExpr = static_cast<NewExpression*>(expr);
			ExpressionValue outer = Eval(newExpr->ParentExpr).Value;
			ExpressionValue name = Eval(newExpr->NameExpr).Value;
			ExpressionValue flags = Eval(newExpr->FlagsExpr).Value;
			UClass* cls = UObject::Cast<UClass>(Eval(newExpr->ClassExpr).Value.ToObject());

			// To do: package needs to be grabbed from outer, or the "transient package" if it is None, a virtual package for runtime objects
			Package* package = engine->packages->GetPackage("Engine");

			UObject* newObj = package->NewObject(
				name.GetType() == ExpressionValueType::Nothing ? NameString() : name.ToName(),
				cls,
				flags.GetType() == ExpressionValueType::Nothing ? ObjectFlags::NoFlags : (ObjectFlags)flags.ToInt(),
				true);

			if (outer.GetType() != ExpressionValueType::Nothing)
				newObj->Outer() = outer.ToObject();

			result.Value = ExpressionValue::ObjectValue(newObj);
			break;
		}

		case ExpressionType::ClassContext:
		{
			ExpressionValue object = Eval(static_cast<ClassContextExpression*>(expr)->ObjectExpr).Value;
			UClass* cls = UObject::TryCast<UClass>(object.ToObject());
			if (cls)
			{
				result = Eval(static_cast<ClassContextExpression*>(expr)->ContextExpr, self, cls->GetDefaultObject<UObject>(), localVariables);
			}
			else
			{
				Frame::ThrowException("Class reference is None");
			}
			break;
		}

		case ExpressionType::MetaCast:
		{
			auto castExpr = static_cast<MetaCastExpression*>(expr);
			UObject* value = Eval(castExpr->Value).Value.ToObject();
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
			result.Value = ExpressionValue::ObjectValue(value);
			break;
		}

		case ExpressionType::Unknown0x15:
			// Klingon honor guard has this! (UE 251)
			//Frame::ThrowException("Unknown0x15 expression encountered");
			result.Result = StatementResult::Stop;
			break;

		case ExpressionType::Self:
			result.Value = ExpressionValue::ObjectValue(self);
			break;

		case ExpressionType::Skip:
			result = Eval(static_cast<SkipExpression*>(expr)->Value);
			break;

		case ExpressionType::Context:
		{
			auto value = Eval(static_cast<ContextExpression*>(expr)->ObjectExpr).Value;
			UObject* context = value.ToObject();
			if (context)
			{
				result = Eval(static_cast<ContextExpression*>(expr)->ContextExpr, self, context, localVariables);
			}
			else
			{
				result.Result = StatementResult::AccessedNone;
			}
			break;
		}

		case ExpressionType::ArrayElement:
		{
			int index = Eval(static_cast<ArrayElementExpression*>(expr)->Index).Value.ToInt();
			auto arrayval = Eval(static_cast<ArrayElementExpression*>(expr)->Array).Value;
			if (arrayval.IsVariable())
			{
				result.Value = arrayval.ItemAt(index);
			}
			else
			{
				Frame::ThrowException("Array is not a variable in ArrayElementExpression");
			}
			break;
		}

		case ExpressionType::IntConst:
			result.Value = ExpressionValue::IntValue(static_cast<IntConstExpression*>(expr)->Value);
			break;

		case ExpressionType::FloatConst:
			result.Value = ExpressionValue::FloatValue(static_cast<FloatConstExpression*>(expr)->Value);
			break;

		case ExpressionType::StringConst:
			result.Value = ExpressionValue::StringValue(static_cast<StringConstExpression*>(expr)->Value);
			break;

		case ExpressionType::ObjectConst:
			result.Value = ExpressionValue::ObjectValue(static_cast<ObjectConstExpression*>(expr)->Object);
			break;

		case ExpressionType::NameConst:
			result.Value = ExpressionValue::NameValue(static_cast<NameConstExpression*>(expr)->Value);
			break;

		case ExpressionType::RotationConst:
			result.Value = ExpressionValue::RotatorValue({
				static_cast<RotationConstExpression*>(expr)->Pitch,
				static_cast<RotationConstExpression*>(expr)->Yaw,
				static_cast<RotationConstExpression*>(expr)->Roll
				});
			break;

		case ExpressionType::VectorConst:
			result.Value = ExpressionValue::VectorValue({
				static_cast<VectorConstExpression*>(expr)->X,
				static_cast<VectorConstExpression*>(expr)->Y,
				static_cast<VectorConstExpression*>(expr)->Z
				});
			break;

		case ExpressionType::ByteConst:
			result.Value = ExpressionValue::ByteValue(static_cast<ByteConstExpression*>(expr)->Value);
			break;

		case ExpressionType::IntZero:
			result.Value = ExpressionValue::IntValue(0);
			break;

		case ExpressionType::IntOne:
			result.Value = ExpressionValue::IntValue(1);
			break;

		case ExpressionType::True:
			result.Value = ExpressionValue::BoolValue(true);
			break;

		case ExpressionType::False:
			result.Value = ExpressionValue::BoolValue(false);
			break;

		case ExpressionType::NativeParm:
			Frame::ThrowException("Native parm expression is not implemented");
			break;

		case ExpressionType::NoObject:
			result.Value = ExpressionValue::ObjectValue(nullptr);
			break;

		case ExpressionType::Unknown0x2b:
			result = Eval(static_cast<Unknown0x2bExpression*>(expr)->Value); // This may have been a truncating instruction from back when strings had a fixed size (package version 61 and earlier)
			break;

		case ExpressionType::IntConstByte:
			result.Value = ExpressionValue::ByteValue(static_cast<IntConstByteExpression*>(expr)->Value);
			break;

		case ExpressionType::BoolVariable:
			result.Value = Eval(static_cast<BoolVariableExpression*>(expr)->Variable).Value;
			break;

		case ExpressionType::DynamicCast:
		{
			UObject* value = Eval(static_cast<DynamicCastExpression*>(expr)->Value).Value.ToObject();
			if (value && !value->IsA(static_cast<DynamicCastExpression*>(expr)->Class->Name))
				value = nullptr;
			result.Value = ExpressionValue::ObjectValue(value);
		}

		case ExpressionType::Iterator:
		{
			Eval(static_cast<IteratorExpression*>(expr)->Value);
			result.Result = StatementResult::Iterator;
			result.Iter = std::move(Frame::CreatedIterator);
			result.JumpAddress = static_cast<IteratorExpression*>(expr)->Offset;
			break;
		}

		case ExpressionType::IteratorPop:
			result.Result = StatementResult::IteratorPop;
			break;

		case ExpressionType::IteratorNext:
			result.Result = StatementResult::IteratorNext;
			break;

		case ExpressionType::StructCmpEq:
		{
			ExpressionValue val1 = Eval(static_cast<StructCmpEqExpression*>(expr)->Value1).Value;
			ExpressionValue val2 = Eval(static_cast<StructCmpEqExpression*>(expr)->Value2).Value;
			result.Value = ExpressionValue::BoolValue(val1.IsEqual(val2));
			break;
		}

		case ExpressionType::StructCmpNe:
		{
			ExpressionValue val1 = Eval(static_cast<StructCmpNeExpression*>(expr)->Value1).Value;
			ExpressionValue val2 = Eval(static_cast<StructCmpNeExpression*>(expr)->Value2).Value;
			result.Value = ExpressionValue::BoolValue(!val1.IsEqual(val2));
			break;
		}

		case ExpressionType::StructMember:
			if (static_cast<StructMemberExpression*>(expr)->Field)
				result.Value = Eval(static_cast<StructMemberExpression*>(expr)->Value).Value.Member(static_cast<StructMemberExpression*>(expr)->Field);
			else
				Frame::ThrowException("Null field encountered in struct member expression");
			break;

		case ExpressionType::UnicodeStringConst:
		{
			std::string s;
			s.reserve(static_cast<UnicodeStringConstExpression*>(expr)->Value.size());
			for (wchar_t c : static_cast<UnicodeStringConstExpression*>(expr)->Value)
				s.push_back(c < 128 ? c : '?');
			result.Value = ExpressionValue::StringValue(s);
			break;
		}

		case ExpressionType::RotatorToVector:
		{
			Rotator rot = Eval(static_cast<RotatorToVectorExpression*>(expr)->Value).Value.ToRotator();
			result.Value = ExpressionValue::VectorValue(Coords::Rotation(rot).XAxis);
			break;
		}

		case ExpressionType::ByteToInt:
			result.Value = ExpressionValue::IntValue(Eval(static_cast<ByteToIntExpression*>(expr)->Value).Value.ToByte());
			break;

		case ExpressionType::ByteToBool:
			result.Value = ExpressionValue::BoolValue(Eval(static_cast<ByteToBoolExpression*>(expr)->Value).Value.ToByte() != 0);
			break;

		case ExpressionType::ByteToFloat:
			result.Value = ExpressionValue::FloatValue(Eval(static_cast<ByteToFloatExpression*>(expr)->Value).Value.ToByte());
			break;

		case ExpressionType::IntToByte:
			result.Value = ExpressionValue::ByteValue(Eval(static_cast<IntToByteExpression*>(expr)->Value).Value.ToInt());
			break;

		case ExpressionType::IntToBool:
			result.Value = ExpressionValue::BoolValue(Eval(static_cast<IntToBoolExpression*>(expr)->Value).Value.ToInt());
			break;

		case ExpressionType::IntToFloat:
			result.Value = ExpressionValue::FloatValue((float)Eval(static_cast<IntToFloatExpression*>(expr)->Value).Value.ToInt());
			break;

		case ExpressionType::BoolToByte:
			result.Value = ExpressionValue::ByteValue(Eval(static_cast<BoolToByteExpression*>(expr)->Value).Value.ToBool());
			break;

		case ExpressionType::BoolToInt:
			result.Value = ExpressionValue::IntValue(Eval(static_cast<BoolToIntExpression*>(expr)->Value).Value.ToBool());
			break;

		case ExpressionType::BoolToFloat:
			result.Value = ExpressionValue::FloatValue(Eval(static_cast<BoolToFloatExpression*>(expr)->Value).Value.ToBool());
			break;

		case ExpressionType::FloatToByte:
			result.Value = ExpressionValue::ByteValue((int)Eval(static_cast<FloatToByteExpression*>(expr)->Value).Value.ToFloat());
			break;

		case ExpressionType::FloatToInt:
			result.Value = ExpressionValue::IntValue((int)Eval(static_cast<FloatToIntExpression*>(expr)->Value).Value.ToFloat());
			break;

		case ExpressionType::FloatToBool:
			result.Value = ExpressionValue::BoolValue((bool)Eval(static_cast<FloatToBoolExpression*>(expr)->Value).Value.ToFloat());
			break;

		case ExpressionType::Unknown0x46:
			Frame::ThrowException("Unknown0x46 expression encountered");
			break;

		case ExpressionType::ObjectToBool:
			result.Value = ExpressionValue::BoolValue(Eval(static_cast<ObjectToBoolExpression*>(expr)->Value).Value.ToObject() != nullptr);
			break;

		case ExpressionType::NameToBool:
			result.Value = ExpressionValue::BoolValue(Eval(static_cast<NameToBoolExpression*>(expr)->Value).Value.ToName() != "None");
			break;

		case ExpressionType::StringToByte:
			result.Value = ExpressionValue::ByteValue(std::atoi(Eval(static_cast<StringToByteExpression*>(expr)->Value).Value.ToString().c_str()));
			break;

		case ExpressionType::StringToInt:
			result.Value = ExpressionValue::IntValue(std::atoi(Eval(static_cast<StringToIntExpression*>(expr)->Value).Value.ToString().c_str()));
			break;

		case ExpressionType::StringToBool:
			result.Value = ExpressionValue::BoolValue(std::atoi(Eval(static_cast<StringToBoolExpression*>(expr)->Value).Value.ToString().c_str()));
			break;

		case ExpressionType::StringToFloat:
			result.Value = ExpressionValue::FloatValue((float)std::atof(Eval(static_cast<StringToFloatExpression*>(expr)->Value).Value.ToString().c_str()));
			break;

		case ExpressionType::StringToVector:
		{
			std::string v = Eval(static_cast<StringToVectorExpression*>(expr)->Value).Value.ToString();
			auto pos1 = v.find_first_of(',');
			auto pos2 = v.find_first_of(',', pos1 + 1);
			if (pos1 != std::string::npos && pos2 != std::string::npos)
			{
				result.Value = ExpressionValue::VectorValue({ (float)std::atof(v.substr(0, pos1).c_str()), (float)std::atof(v.substr(pos1 + 1, pos2 - pos1 - 1).c_str()), (float)std::atof(v.substr(pos2 + 1).c_str()) });
			}
			else
			{
				result.Value = ExpressionValue::VectorValue({ 0.0f });
			}
			break;
		}

		case ExpressionType::StringToRotator:
		{
			std::string v = Eval(static_cast<StringToRotatorExpression*>(expr)->Value).Value.ToString();
			auto pos1 = v.find_first_of(',');
			auto pos2 = v.find_first_of(',', pos1 + 1);
			if (pos1 != std::string::npos && pos2 != std::string::npos)
			{
				result.Value = ExpressionValue::RotatorValue({ std::atoi(v.substr(0, pos1).c_str()), std::atoi(v.substr(pos1 + 1, pos2 - pos1 - 1).c_str()), std::atoi(v.substr(pos2 + 1).c_str()) });
			}
			else
			{
				result.Value = ExpressionValue::RotatorValue({ 0, 0, 0 });
			}
			break;
		}

		case ExpressionType::VectorToBool:
			result.Value = ExpressionValue::BoolValue(Eval(static_cast<VectorToBoolExpression*>(expr)->Value).Value.ToVector() != vec3(0.0f));
			break;

		case ExpressionType::VectorToRotator:
			result.Value = ExpressionValue::RotatorValue(Rotator::FromVector(Eval(static_cast<VectorToRotatorExpression*>(expr)->Value).Value.ToVector()));
			break;

		case ExpressionType::RotatorToBool:
			result.Value = ExpressionValue::BoolValue(Eval(static_cast<RotatorToBoolExpression*>(expr)->Value).Value.ToRotator() != Rotator(0, 0, 0));
			break;

		case ExpressionType::ByteToString:
			result.Value = ExpressionValue::StringValue(std::to_string(Eval(static_cast<ByteToStringExpression*>(expr)->Value).Value.ToByte()));
			break;

		case ExpressionType::IntToString:
			result.Value = ExpressionValue::StringValue(std::to_string(Eval(static_cast<IntToStringExpression*>(expr)->Value).Value.ToInt()));
			break;

		case ExpressionType::BoolToString:
			result.Value = ExpressionValue::StringValue(std::to_string(Eval(static_cast<BoolToStringExpression*>(expr)->Value).Value.ToBool()));
			break;

		case ExpressionType::FloatToString:
			result.Value = ExpressionValue::StringValue(std::to_string(Eval(static_cast<FloatToStringExpression*>(expr)->Value).Value.ToFloat()));
			break;

		case ExpressionType::ObjectToString:
		{
			UObject* obj = Eval(static_cast<ObjectToStringExpression*>(expr)->Value).Value.ToObject();
			result.Value = ExpressionValue::StringValue(obj ? obj->package->GetPackageName().ToString() + "." + obj->Name.ToString() : "None");
		}

		case ExpressionType::NameToString:
			result.Value = ExpressionValue::StringValue(Eval(static_cast<NameToStringExpression*>(expr)->Value).Value.ToName().ToString());
			break;

		case ExpressionType::VectorToString:
		{
			vec3 v = Eval(static_cast<VectorToStringExpression*>(expr)->Value).Value.ToVector();
			result.Value = ExpressionValue::StringValue(std::to_string(v.x) + "," + std::to_string(v.y) + "," + std::to_string(v.z));
			break;
		}

		case ExpressionType::RotatorToString:
		{
			Rotator v = Eval(static_cast<RotatorToStringExpression*>(expr)->Value).Value.ToRotator();
			result.Value = ExpressionValue::StringValue(std::to_string(v.Pitch & 0xffff) + "," + std::to_string(v.Yaw & 0xffff) + "," + std::to_string(v.Roll & 0xffff));
			break;
		}

		case ExpressionType::StringToName:
		{
			std::string v = Eval(static_cast<StringToNameExpression*>(expr)->Value).Value.ToString();
			result.Value = ExpressionValue::NameValue(v);
			break;
		}

		case ExpressionType::DynArrayToInt:
		{
			size_t count = Eval(static_cast<DynArrayToIntExpression*>(expr)->Value).Value.ToArray().GetSize();
			result.Value = ExpressionValue::IntValue((int)count);
			break;
		}

		case ExpressionType::VirtualFunction:
		{
			UClass* contextClass = UObject::TryCast<UClass>(context);
			if (!contextClass)
				contextClass = context->Class;

			// Search states first

			NameString stateName = context->GetStateName();
			for (UClass* cls = contextClass; cls != nullptr; cls = static_cast<UClass*>(cls->BaseStruct))
			{
				UState* state = cls->GetState(stateName);
				if (state)
				{
					UFunction* func = state->GetFunction(static_cast<VirtualFunctionExpression*>(expr)->Name);
					if (func)
					{
						CallExpr(func, static_cast<VirtualFunctionExpression*>(expr)->Args);
						return;
					}
				}
			}

			// Search normal member functions next

			for (UClass* cls = contextClass; cls != nullptr; cls = static_cast<UClass*>(cls->BaseStruct))
			{
				for (UField* field = cls->Children; field != nullptr; field = field->Next)
				{
					UFunction* func = UObject::TryCast<UFunction>(field);
					if (func && func->Name == static_cast<VirtualFunctionExpression*>(expr)->Name)
					{
						CallExpr(func, static_cast<VirtualFunctionExpression*>(expr)->Args);
						return;
					}
				}
			}

			Frame::ThrowException("Script virtual function " + static_cast<VirtualFunctionExpression*>(expr)->Name.ToString() + " not found!");
			break;
		}

		case ExpressionType::FinalFunction:
			CallExpr(static_cast<FinalFunctionExpression*>(expr)->Func, static_cast<FinalFunctionExpression*>(expr)->Args);
			break;

		case ExpressionType::GlobalFunction:
		{
			// Global function calls skip the states and only searches normal member functions

			UClass* contextClass = UObject::TryCast<UClass>(context);
			if (!contextClass)
				contextClass = context->Class;

			for (UClass* cls = contextClass; cls != nullptr; cls = static_cast<UClass*>(cls->BaseStruct))
			{
				UFunction* func = cls->GetFunction(static_cast<GlobalFunctionExpression*>(expr)->Name);
				if (func)
				{
					CallExpr(func, static_cast<GlobalFunctionExpression*>(expr)->Args);
					return;
				}
			}

			Frame::ThrowException("Script global function " + static_cast<GlobalFunctionExpression*>(expr)->Name.ToString() + " not found!");
			break;
		}

		case ExpressionType::NativeFunction:
			if (static_cast<NativeFunctionExpression*>(expr)->nativeindex == 130) // conditional operator &&
			{
				result.Value = ExpressionValue::BoolValue(
					Eval(static_cast<NativeFunctionExpression*>(expr)->Args[0], self, self, localVariables).Value.ToBool() &&
					Eval(static_cast<NativeFunctionExpression*>(expr)->Args[1], self, self, localVariables).Value.ToBool());
			}
			else if (static_cast<NativeFunctionExpression*>(expr)->nativeindex == 132) // conditional operator ||
			{
				result.Value = ExpressionValue::BoolValue(
					Eval(static_cast<NativeFunctionExpression*>(expr)->Args[0], self, self, localVariables).Value.ToBool() ||
					Eval(static_cast<NativeFunctionExpression*>(expr)->Args[1], self, self, localVariables).Value.ToBool());
			}
			else
			{
				CallExpr(NativeFunctions::FuncByIndex[static_cast<NativeFunctionExpression*>(expr)->nativeindex], static_cast<NativeFunctionExpression*>(expr)->Args);
			}
			break;

		case ExpressionType::Construct:
			Frame::ThrowException("Construct expression not implemented");
			break;

		case ExpressionType::FunctionArguments:
			result.Value = ExpressionValue::NothingValue();
			break;
		}
	}

	Frame::StepExpression = oldExpr;
	return result;
}

ExpressionValue Frame::CallExpr(UFunction* func, const Array<Expression*>& exprArgs)
{
	/*
	Array<ExpressionValue> args;
	args.reserve(exprArgs.size());
	for (Expression* arg : exprArgs)
		args.push_back(Eval(arg, self, self, localVariables).Value);
	return Frame::Call(func, context, std::move(args));
	*/
	return {};
}

#endif

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
