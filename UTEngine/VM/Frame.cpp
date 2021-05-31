
#include "Precomp.h"
#include "Frame.h"
#include "Bytecode.h"
#include "ExpressionEvaluator.h"
#include "NativeFunc.h"

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
					args.push_back(retval);
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

		return returnparmfound ? args.back() : ExpressionValue::NothingValue();
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
						const ExpressionValue& rvalue = args[argindex];
						lvalue.Store(rvalue);
					}

					argindex++;
				}
			}
		}

		ExpressionValue result = frame.Run().Value;
		result.Load();

		for (UField* field = func->Children; field != nullptr; field = field->Next)
		{
			UProperty* prop = dynamic_cast<UProperty*>(field);
			if (prop)
			{
				ExpressionValue lvalue = ExpressionValue::Variable(frame.Variables.get(), prop);
				lvalue.VariableProperty->Destruct(lvalue.VariablePtr);
			}
		}

		return result;
	}
}

ExpressionEvalResult Frame::Run()
{
	while (true)
	{
		if (StatementIndex >= Code->Statements.size())
			throw std::runtime_error("Unexpected end of code statements");

		ExpressionEvalResult result = ExpressionEvaluator::Eval(Code->Statements[StatementIndex], Object, Variables.get());
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
	for (int i = 0; i < switchexpr->Size; i++)
	{
		CaseExpression* caseexpr = static_cast<CaseExpression*>(Code->Statements[StatementIndex++]);
		if (caseexpr->Value)
		{
			ExpressionValue casevalue = ExpressionEvaluator::Eval(caseexpr->Value, Object, Variables.get()).Value;
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
