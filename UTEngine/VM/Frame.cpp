
#include "Precomp.h"
#include "Frame.h"
#include "Bytecode.h"
#include "ExpressionEvaluator.h"
#include "NativeFunc.h"
#include "UObject/UTextBuffer.h"

#ifdef _DEBUG
std::vector<std::string> callstack;
std::string Frame::GetCallstack()
{
	std::string result;
	for (auto it = callstack.rbegin(); it != callstack.rend(); ++it)
	{
		const std::string& callstackName = *it;
#ifdef WIN32
		if (!result.empty()) result += "\r\n";
#else
		if (!result.empty()) result += "\n";
#endif
		result += callstackName;
	}
	return result;
}
#else
std::vector<UFunction*> callstack;
std::string Frame::GetCallstack()
{
	std::string result;

#ifdef WIN32
	std::string newline = "\r\n";
#else
	std::string newline = "\n";
#endif

	for (auto it = callstack.rbegin(); it != callstack.rend(); ++it)
	{
		UFunction* func = *it;
		std::string callstackName;
		for (UStruct* s = func; s != nullptr; s = s->StructParent)
		{
			if (callstackName.empty())
				callstackName = s->Name;
			else
				callstackName = s->Name + "." + callstackName;
		}
		if (func)
			callstackName += " line " + std::to_string(func->Line);
		if (!result.empty()) result += newline;
		result += callstackName;
	}
	return result;
}
#endif

ExpressionValue Frame::Call(UFunction* func, UObject* instance, std::vector<ExpressionValue> args)
{
#ifdef _DEBUG
	std::string callstackName;
	for (UStruct* s = func; s != nullptr; s = s->StructParent)
	{
		if (callstackName.empty())
			callstackName = s->Name;
		else
			callstackName = s->Name + "." + callstackName;
	}
	callstack.push_back(callstackName);
#else
	callstack.push_back(func);
#endif

#if 0
	if (callstackName == "WindowConsole.UWindow.KeyEvent")
	{
		//std::string properties = instance->GetUObject("MouseWindow")->PrintProperties();
		std::string properties = instance->PrintProperties();
		properties.push_back(' ');
	}
#endif

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

		if (func->NativeFuncIndex != 0)
		{
			NativeFunctions::NativeByIndex[func->NativeFuncIndex](instance, args.data());
		}
		else
		{
			NativeFunctions::NativeByName[{ func->Name, func->NativeStruct->Name }](instance, args.data());
		}

		callstack.pop_back();
		return returnparmfound ? std::move(args.back()) : ExpressionValue::NothingValue();
	}
	else
	{
		Frame frame;
		frame.Variables.reset(new uint64_t[(func->StructSize + 7) / 8]);
		frame.Object = instance;
		frame.Code = func->Code.get();

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

		callstack.pop_back();
		return result;
	}
}

ExpressionEvalResult Frame::Run()
{
	while (true)
	{
		if (StatementIndex >= Code->Statements.size())
			throw std::runtime_error("Unexpected end of code statements");

		ExpressionEvalResult result = ExpressionEvaluator::Eval(Code->Statements[StatementIndex], Object, Object, Variables.get());
		switch (result.Result)
		{
		case StatementResult::Next:
			StatementIndex++;
			break;
		case StatementResult::Jump:
			StatementIndex = Code->FindStatementIndex(result.JumpAddress);
			break;
		case StatementResult::Switch:
			ProcessSwitch(result.Value);
			break;
		case StatementResult::LatentWait:
		case StatementResult::Return:
		case StatementResult::Stop:
		case StatementResult::GotoLabel:
			return result;
		}
	}
}

void Frame::ProcessSwitch(const ExpressionValue& condition)
{
	SwitchExpression* switchexpr = static_cast<SwitchExpression*>(Code->Statements[StatementIndex++]);
	while (true)
	{
		CaseExpression* caseexpr = static_cast<CaseExpression*>(Code->Statements[StatementIndex++]);
		if (caseexpr->Value)
		{
			ExpressionValue casevalue = ExpressionEvaluator::Eval(caseexpr->Value, Object, Object, Variables.get()).Value;
			if (condition.IsEqual(casevalue))
				break;
			else
				StatementIndex = Code->FindStatementIndex(caseexpr->NextOffset);
		}
		else
		{
			break;
		}
	}
}
