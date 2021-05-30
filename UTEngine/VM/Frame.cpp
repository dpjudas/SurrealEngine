
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
					ExpressionValue retval = ExpressionValue::NothingValue();
					if (dynamic_cast<UByteProperty*>(prop)) retval.Type = ExpressionValueType::ValueByte;
					else if (dynamic_cast<UIntProperty*>(prop)) retval.Type = ExpressionValueType::ValueInt;
					else if (dynamic_cast<UBoolProperty*>(prop)) retval.Type = ExpressionValueType::ValueBool;
					else if (dynamic_cast<UFloatProperty*>(prop)) retval.Type = ExpressionValueType::ValueFloat;
					else if (dynamic_cast<UObjectProperty*>(prop)) retval.Type = ExpressionValueType::ValueObject;
					else if (dynamic_cast<UStructProperty*>(prop) && static_cast<UStructProperty*>(prop)->Struct->Name == "Vector") retval.Type = ExpressionValueType::ValueVector;
					else if (dynamic_cast<UStructProperty*>(prop) && static_cast<UStructProperty*>(prop)->Struct->Name == "Rotator") retval.Type = ExpressionValueType::ValueRotator;
					else if (dynamic_cast<UStringProperty*>(prop)) retval.Type = ExpressionValueType::ValueString;
					else if (dynamic_cast<UStrProperty*>(prop)) retval.Type = ExpressionValueType::ValueString;
					else if (dynamic_cast<UNameProperty*>(prop)) retval.Type = ExpressionValueType::ValueName;
					else if (dynamic_cast<UStructProperty*>(prop) && static_cast<UStructProperty*>(prop)->Struct->Name == "Color") retval.Type = ExpressionValueType::ValueColor;
					else throw std::runtime_error("Unknown return parm property type");
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
				if (AllFlags(prop->PropFlags, PropertyFlags::Parm))
				{
					if (argindex < args.size())
					{
						const ExpressionValue& rvalue = args[argindex];
						switch (rvalue.Type)
						{
						default:
						case ExpressionValueType::Nothing: break;
						case ExpressionValueType::Variable: lvalue.VariableProperty->CopyConstruct(lvalue.VariablePtr, rvalue.VariablePtr); break;
						case ExpressionValueType::ValueByte: *static_cast<uint8_t*>(lvalue.VariablePtr) = rvalue.Value.Byte; break;
						case ExpressionValueType::ValueInt: *static_cast<int32_t*>(lvalue.VariablePtr) = rvalue.Value.Int; break;
						case ExpressionValueType::ValueBool: *static_cast<bool*>(lvalue.VariablePtr) = rvalue.Value.Bool; break;
						case ExpressionValueType::ValueFloat: *static_cast<float*>(lvalue.VariablePtr) = rvalue.Value.Float; break;
						case ExpressionValueType::ValueObject: *static_cast<UObject**>(lvalue.VariablePtr) = rvalue.Value.Object; break;
						case ExpressionValueType::ValueVector: *static_cast<vec3*>(lvalue.VariablePtr) = rvalue.Value.Vector; break;
						case ExpressionValueType::ValueRotator: *static_cast<Rotator*>(lvalue.VariablePtr) = rvalue.Value.Rotator; break;
						case ExpressionValueType::ValueString: *static_cast<std::string*>(lvalue.VariablePtr) = rvalue.ValueString; break;
						case ExpressionValueType::ValueName: *static_cast<std::string*>(lvalue.VariablePtr) = rvalue.ValueString; break;
						case ExpressionValueType::ValueColor: *static_cast<Color*>(lvalue.VariablePtr) = rvalue.Value.Color; break;
						}
					}
					else
					{
						prop->Construct(reinterpret_cast<uint8_t*>(frame.Variables.get()) + prop->DataOffset);
					}

					argindex++;
				}
				else
				{
					lvalue.VariableProperty->Construct(lvalue.VariablePtr);
				}
			}
		}

		ExpressionValue result = frame.Run().Value;

		if (result.Type == ExpressionValueType::Variable)
		{
			auto prop = result.VariableProperty;
			if (dynamic_cast<UByteProperty*>(prop)) result = ExpressionValue::ByteValue(result.ToByte());
			else if (dynamic_cast<UIntProperty*>(prop)) result = ExpressionValue::IntValue(result.ToInt());
			else if (dynamic_cast<UBoolProperty*>(prop)) result = ExpressionValue::BoolValue(result.ToBool());
			else if (dynamic_cast<UFloatProperty*>(prop)) result = ExpressionValue::FloatValue(result.ToFloat());
			else if (dynamic_cast<UObjectProperty*>(prop)) result = ExpressionValue::ObjectValue(result.ToObject());
			else if (dynamic_cast<UStructProperty*>(prop) && static_cast<UStructProperty*>(prop)->Struct->Name == "Vector") result = ExpressionValue::VectorValue(result.ToVector());
			else if (dynamic_cast<UStructProperty*>(prop) && static_cast<UStructProperty*>(prop)->Struct->Name == "Rotator") result = ExpressionValue::RotatorValue(result.ToRotator());
			else if (dynamic_cast<UStringProperty*>(prop)) result = ExpressionValue::StringValue(result.ToString());
			else if (dynamic_cast<UStrProperty*>(prop)) result = ExpressionValue::StringValue(result.ToString());
			else if (dynamic_cast<UNameProperty*>(prop)) result = ExpressionValue::NameValue(result.ToName());
			else if (dynamic_cast<UStructProperty*>(prop) && static_cast<UStructProperty*>(prop)->Struct->Name == "Color") result = ExpressionValue::ColorValue(result.ToColor());
			else throw std::runtime_error("Unknown return variable type");
		}

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

			bool matched = false;
			if (condition.Type == ExpressionValueType::Variable)
			{
				auto prop = condition.VariableProperty;
				if (dynamic_cast<UByteProperty*>(prop)) matched = condition.ToByte() == casevalue.ToByte();
				else if (dynamic_cast<UIntProperty*>(prop)) matched = condition.ToInt() == casevalue.ToInt();
				else if (dynamic_cast<UBoolProperty*>(prop)) matched = condition.ToBool() == casevalue.ToBool();
				else if (dynamic_cast<UFloatProperty*>(prop)) matched = condition.ToFloat() == casevalue.ToFloat();
				else if (dynamic_cast<UObjectProperty*>(prop)) matched = condition.ToObject() == casevalue.ToObject();
				else if (dynamic_cast<UStructProperty*>(prop) && static_cast<UStructProperty*>(prop)->Struct->Name == "Vector") matched = condition.ToVector() == casevalue.ToVector();
				else if (dynamic_cast<UStructProperty*>(prop) && static_cast<UStructProperty*>(prop)->Struct->Name == "Rotator") matched = condition.ToRotator() == casevalue.ToRotator();
				else if (dynamic_cast<UStringProperty*>(prop)) matched = condition.ToString() == casevalue.ToString();
				else if (dynamic_cast<UStrProperty*>(prop)) matched = condition.ToString() == casevalue.ToString();
				else if (dynamic_cast<UNameProperty*>(prop)) matched = condition.ToName() == casevalue.ToName();
				else if (dynamic_cast<UStructProperty*>(prop) && static_cast<UStructProperty*>(prop)->Struct->Name == "Color") matched = condition.ToColor() == casevalue.ToColor();
			}
			else
			{
				switch (condition.Type)
				{
				default:
				case ExpressionValueType::Nothing: break;
				case ExpressionValueType::ValueByte: matched = condition.ToByte() == casevalue.ToByte(); break;
				case ExpressionValueType::ValueInt: matched = condition.ToInt() == casevalue.ToInt(); break;
				case ExpressionValueType::ValueBool: matched = condition.ToBool() == casevalue.ToBool(); break;
				case ExpressionValueType::ValueFloat: matched = condition.ToFloat() == casevalue.ToFloat(); break;
				case ExpressionValueType::ValueObject: matched = condition.ToObject() == casevalue.ToObject(); break;
				case ExpressionValueType::ValueVector: matched = condition.ToVector() == casevalue.ToVector(); break;
				case ExpressionValueType::ValueRotator: matched = condition.ToRotator() == casevalue.ToRotator(); break;
				case ExpressionValueType::ValueString: matched = condition.ToString() == casevalue.ToString(); break;
				case ExpressionValueType::ValueName: matched = condition.ToName() == casevalue.ToName(); break;
				case ExpressionValueType::ValueColor: matched = condition.ToColor() == casevalue.ToColor(); break;
				}
			}

			if (matched)
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
