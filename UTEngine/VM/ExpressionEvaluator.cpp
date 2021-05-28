
#include "Precomp.h"
#include "ExpressionEvaluator.h"
#include "Expression.h"
#include "Bytecode.h"
#include "Frame.h"
#include "NativeFunc.h"

ExpressionEvalResult ExpressionEvaluator::Eval(Expression* expr, UObject* self, void* localVariables)
{
	ExpressionEvaluator evaluator;
	evaluator.Self = self;
	evaluator.LocalVariables = localVariables;
	expr->Visit(&evaluator);
	return evaluator.Result;
}

void ExpressionEvaluator::Expr(LocalVariableExpression* expr)
{
	Result.Value = ExpressionValue::Variable(nullptr, expr->Variable);
}

void ExpressionEvaluator::Expr(InstanceVariableExpression* expr)
{
	Result.Value = ExpressionValue::Variable(Self->PropertyData.Data, expr->Variable);
}

void ExpressionEvaluator::Expr(DefaultVariableExpression* expr)
{
	// Result.Value = ExpressionValue::Variable(Self->DefaultObject, expr->Variable);
}

void ExpressionEvaluator::Expr(ReturnExpression* expr)
{
	Result.Value = Eval(expr->Value).Value;
	Result.Result = StatementResult::Return;
}

void ExpressionEvaluator::Expr(SwitchExpression* expr)
{
}

void ExpressionEvaluator::Expr(JumpExpression* expr)
{
	Result.Result = StatementResult::Jump;
	Result.JumpAddress = expr->Offset;
}

void ExpressionEvaluator::Expr(JumpIfNotExpression* expr)
{
	if (!Eval(expr->Condition).Value.ToBool())
	{
		Result.Result = StatementResult::Jump;
		Result.JumpAddress = expr->Offset;
	}
}

void ExpressionEvaluator::Expr(StopExpression* expr)
{
	Result.Result = StatementResult::Stop;
}

void ExpressionEvaluator::Expr(AssertExpression* expr)
{
	if (!Eval(expr->Condition).Value.ToBool())
	{
		throw std::runtime_error("Script assert failed for " + Self->Name + " line " + std::to_string(expr->Line));
	}
}

void ExpressionEvaluator::Expr(CaseExpression* expr)
{
}

void ExpressionEvaluator::Expr(NothingExpression* expr)
{
	Result.Value = ExpressionValue::NothingValue();
}

void ExpressionEvaluator::Expr(LabelTableExpression* expr)
{
}

void ExpressionEvaluator::Expr(GotoLabelExpression* expr)
{
	Result.Result = StatementResult::GotoLabel;
	Result.Label = Eval(expr->Value).Value.ToName();
}

void ExpressionEvaluator::Expr(EatStringExpression* expr)
{
	Eval(expr->Value);
	Result.Value = ExpressionValue::NothingValue();
}

void ExpressionEvaluator::Expr(LetExpression* expr)
{
	ExpressionValue lvalue = Eval(expr->LeftSide).Value;
	ExpressionValue rvalue = Eval(expr->RightSide).Value;

	switch (rvalue.Type)
	{
	default:
	case ExpressionValueType::Nothing: throw std::runtime_error("Unexpected rvalue type for let expression"); break;
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

void ExpressionEvaluator::Expr(DynArrayElementExpression* expr)
{
}

void ExpressionEvaluator::Expr(NewExpression* expr)
{
}

void ExpressionEvaluator::Expr(ClassContextExpression* expr)
{
}

void ExpressionEvaluator::Expr(MetaCastExpression* expr)
{
}

void ExpressionEvaluator::Expr(LetBoolExpression* expr)
{
}

void ExpressionEvaluator::Expr(Unknown0x15Expression* expr)
{
	throw std::runtime_error("Unknown0x15 expression encountered");
}

void ExpressionEvaluator::Expr(SelfExpression* expr)
{
	Result.Value = ExpressionValue::ObjectValue(Self);
}

void ExpressionEvaluator::Expr(SkipExpression* expr)
{
}

void ExpressionEvaluator::Expr(ContextExpression* expr)
{
}

void ExpressionEvaluator::Expr(ArrayElementExpression* expr)
{
}

void ExpressionEvaluator::Expr(IntConstExpression* expr)
{
	Result.Value = ExpressionValue::IntValue(expr->Value);
}

void ExpressionEvaluator::Expr(FloatConstExpression* expr)
{
	Result.Value = ExpressionValue::FloatValue(expr->Value);
}

void ExpressionEvaluator::Expr(StringConstExpression* expr)
{
	Result.Value = ExpressionValue::StringValue(expr->Value);
}

void ExpressionEvaluator::Expr(ObjectConstExpression* expr)
{
	Result.Value = ExpressionValue::ObjectValue(expr->Object);
}

void ExpressionEvaluator::Expr(NameConstExpression* expr)
{
	Result.Value = ExpressionValue::NameValue(expr->Value);
}

void ExpressionEvaluator::Expr(RotationConstExpression* expr)
{
	Result.Value = ExpressionValue::RotatorValue({ expr->Pitch * (360.0f / 65536.0f), expr->Yaw * (360.0f / 65536.0f), expr->Roll * (360.0f / 65536.0f) });
}

void ExpressionEvaluator::Expr(VectorConstExpression* expr)
{
	Result.Value = ExpressionValue::VectorValue({ expr->X, expr->Y, expr->Z });
}

void ExpressionEvaluator::Expr(ByteConstExpression* expr)
{
	Result.Value = ExpressionValue::ByteValue(expr->Value);
}

void ExpressionEvaluator::Expr(IntZeroExpression* expr)
{
	Result.Value = ExpressionValue::IntValue(0);
}

void ExpressionEvaluator::Expr(IntOneExpression* expr)
{
	Result.Value = ExpressionValue::IntValue(1);
}

void ExpressionEvaluator::Expr(TrueExpression* expr)
{
	Result.Value = ExpressionValue::BoolValue(true);
}

void ExpressionEvaluator::Expr(FalseExpression* expr)
{
	Result.Value = ExpressionValue::BoolValue(false);
}

void ExpressionEvaluator::Expr(NativeParmExpression* expr)
{
}

void ExpressionEvaluator::Expr(NoObjectExpression* expr)
{
	Result.Value = ExpressionValue::ObjectValue(nullptr);
}

void ExpressionEvaluator::Expr(Unknown0x2bExpression* expr)
{
	throw std::runtime_error("Unknown0x2b expression encountered");
}

void ExpressionEvaluator::Expr(IntConstByteExpression* expr)
{
	Result.Value = ExpressionValue::ByteValue(expr->Value);
}

void ExpressionEvaluator::Expr(BoolVariableExpression* expr)
{
}

void ExpressionEvaluator::Expr(DynamicCastExpression* expr)
{
}

void ExpressionEvaluator::Expr(IteratorExpression* expr)
{
}

void ExpressionEvaluator::Expr(IteratorPopExpression* expr)
{
}

void ExpressionEvaluator::Expr(IteratorNextExpression* expr)
{
}

void ExpressionEvaluator::Expr(StructCmpEqExpression* expr)
{
}

void ExpressionEvaluator::Expr(StructCmpNeExpression* expr)
{
}

void ExpressionEvaluator::Expr(UnicodeStringConstExpression* expr)
{
}

void ExpressionEvaluator::Expr(StructMemberExpression* expr)
{
}

void ExpressionEvaluator::Expr(RotatorToVectorExpression* expr)
{
}

void ExpressionEvaluator::Expr(ByteToIntExpression* expr)
{
	Result.Value = ExpressionValue::IntValue(Eval(expr->Value).Value.ToByte());
}

void ExpressionEvaluator::Expr(ByteToBoolExpression* expr)
{
	Result.Value = ExpressionValue::BoolValue(Eval(expr->Value).Value.ToByte() != 0);
}

void ExpressionEvaluator::Expr(ByteToFloatExpression* expr)
{
	Result.Value = ExpressionValue::FloatValue(Eval(expr->Value).Value.ToByte());
}

void ExpressionEvaluator::Expr(IntToByteExpression* expr)
{
	Result.Value = ExpressionValue::ByteValue(Eval(expr->Value).Value.ToInt());
}

void ExpressionEvaluator::Expr(IntToBoolExpression* expr)
{
	Result.Value = ExpressionValue::BoolValue(Eval(expr->Value).Value.ToInt());
}

void ExpressionEvaluator::Expr(IntToFloatExpression* expr)
{
	Result.Value = ExpressionValue::FloatValue((float)Eval(expr->Value).Value.ToInt());
}

void ExpressionEvaluator::Expr(BoolToByteExpression* expr)
{
	Result.Value = ExpressionValue::ByteValue(Eval(expr->Value).Value.ToBool());
}

void ExpressionEvaluator::Expr(BoolToIntExpression* expr)
{
	Result.Value = ExpressionValue::IntValue(Eval(expr->Value).Value.ToBool());
}

void ExpressionEvaluator::Expr(BoolToFloatExpression* expr)
{
	Result.Value = ExpressionValue::FloatValue(Eval(expr->Value).Value.ToBool());
}

void ExpressionEvaluator::Expr(FloatToByteExpression* expr)
{
	Result.Value = ExpressionValue::ByteValue((int)Eval(expr->Value).Value.ToFloat());
}

void ExpressionEvaluator::Expr(FloatToIntExpression* expr)
{
	Result.Value = ExpressionValue::IntValue((int)Eval(expr->Value).Value.ToFloat());
}

void ExpressionEvaluator::Expr(FloatToBoolExpression* expr)
{
	Result.Value = ExpressionValue::BoolValue((bool)Eval(expr->Value).Value.ToFloat());
}

void ExpressionEvaluator::Expr(Unknown0x46Expression* expr)
{
	throw std::runtime_error("Unknown0x46 expression encountered");
}

void ExpressionEvaluator::Expr(ObjectToBoolExpression* expr)
{
	Result.Value = ExpressionValue::BoolValue(Eval(expr->Value).Value.ToObject() != nullptr);
}

void ExpressionEvaluator::Expr(NameToBoolExpression* expr)
{
	Result.Value = ExpressionValue::BoolValue(Eval(expr->Value).Value.ToName() != "None");
}

void ExpressionEvaluator::Expr(StringToByteExpression* expr)
{
	Result.Value = ExpressionValue::ByteValue(std::atoi(Eval(expr->Value).Value.ToString().c_str()));
}

void ExpressionEvaluator::Expr(StringToIntExpression* expr)
{
	Result.Value = ExpressionValue::IntValue(std::atoi(Eval(expr->Value).Value.ToString().c_str()));
}

void ExpressionEvaluator::Expr(StringToBoolExpression* expr)
{
	Result.Value = ExpressionValue::BoolValue(std::atoi(Eval(expr->Value).Value.ToString().c_str()));
}

void ExpressionEvaluator::Expr(StringToFloatExpression* expr)
{
	Result.Value = ExpressionValue::FloatValue((float)std::atof(Eval(expr->Value).Value.ToString().c_str()));
}

void ExpressionEvaluator::Expr(StringToVectorExpression* expr)
{
}

void ExpressionEvaluator::Expr(StringToRotatorExpression* expr)
{
}

void ExpressionEvaluator::Expr(VectorToBoolExpression* expr)
{
	Result.Value = ExpressionValue::BoolValue(Eval(expr->Value).Value.ToVector() != vec3(0.0f));
}

void ExpressionEvaluator::Expr(VectorToRotatorExpression* expr)
{
	vec3 v = Eval(expr->Value).Value.ToVector();
	Result.Value = ExpressionValue::RotatorValue({ v.x, v.y, v.z });
}

void ExpressionEvaluator::Expr(RotatorToBoolExpression* expr)
{
	Result.Value = ExpressionValue::BoolValue(Eval(expr->Value).Value.ToRotator() != Rotator(0.0f, 0.0f, 0.0f));
}

void ExpressionEvaluator::Expr(ByteToStringExpression* expr)
{
	Result.Value = ExpressionValue::StringValue(std::to_string(Eval(expr->Value).Value.ToByte()));
}

void ExpressionEvaluator::Expr(IntToStringExpression* expr)
{
	Result.Value = ExpressionValue::StringValue(std::to_string(Eval(expr->Value).Value.ToInt()));
}

void ExpressionEvaluator::Expr(BoolToStringExpression* expr)
{
	Result.Value = ExpressionValue::StringValue(std::to_string(Eval(expr->Value).Value.ToBool()));
}

void ExpressionEvaluator::Expr(FloatToStringExpression* expr)
{
	Result.Value = ExpressionValue::StringValue(std::to_string(Eval(expr->Value).Value.ToFloat()));
}

void ExpressionEvaluator::Expr(ObjectToStringExpression* expr)
{
}

void ExpressionEvaluator::Expr(NameToStringExpression* expr)
{
	Result.Value = ExpressionValue::StringValue(Eval(expr->Value).Value.ToName());
}

void ExpressionEvaluator::Expr(VectorToStringExpression* expr)
{
}

void ExpressionEvaluator::Expr(RotatorToStringExpression* expr)
{
}

void ExpressionEvaluator::Expr(VirtualFunctionExpression* expr)
{
	for (UClass* cls = Self->Base; cls != nullptr; cls = cls->Base)
	{
		for (UField* field = cls->Children; field != nullptr; field = field->Next)
		{
			UFunction* func = UObject::TryCast<UFunction>(field);
			if (func && func->Name == expr->Name)
			{
				Call(func, expr->Args);
				return;
			}
		}
	}
	throw std::runtime_error("Script virtual function " + expr->Name + " not found!");
}

void ExpressionEvaluator::Expr(FinalFunctionExpression* expr)
{
	Call(expr->Func, expr->Args);
}

void ExpressionEvaluator::Expr(GlobalFunctionExpression* expr)
{
	// Non-final static functions are VirtualFunctionExpression and final static functions are FinalFunctionExpression, so what is a global function?
	throw std::runtime_error("Script global function expression not implemented");
}

void ExpressionEvaluator::Expr(NativeFunctionExpression* expr)
{
	std::vector<ExpressionValue> args;
	args.reserve(expr->Args.size());
	for (Expression* arg : expr->Args) args.push_back(Eval(arg).Value);

	// To do: we need to know the UFunction so that we can handle optional and return parms

	NativeFunctions::NativeByIndex[expr->nativeindex](Self, args.data());
	Result.Value = ExpressionValue::NothingValue();
}

void ExpressionEvaluator::Call(UFunction* func, const std::vector<Expression*>& exprArgs)
{
	std::vector<ExpressionValue> args;
	args.reserve(exprArgs.size());
	for (Expression* arg : exprArgs) args.push_back(Eval(arg).Value);

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
		int argindex = 0;
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
			NativeFunctions::NativeByIndex[func->NativeFuncIndex](Self, args.data());
		}
		else
		{
			NativeFunctions::NativeByName[{ func->Name, func->NativeStruct->Name }](Self, args.data());
		}

		Result.Value = returnparmfound ? args.back() : ExpressionValue::NothingValue();
	}
	else
	{
		Frame frame;
		frame.Object = Self;
		frame.Code = func->Code.get();
		Result = frame.Run();
	}
}
