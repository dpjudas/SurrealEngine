
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
	Result.Value = ExpressionValue::Variable(LocalVariables, expr->Variable);
}

void ExpressionEvaluator::Expr(InstanceVariableExpression* expr)
{
	Result.Value = ExpressionValue::Variable(Self->PropertyData.Data, expr->Variable);
}

void ExpressionEvaluator::Expr(DefaultVariableExpression* expr)
{
	Result.Value = ExpressionValue::Variable(Self->Base->PropertyData.Data, expr->Variable);
}

void ExpressionEvaluator::Expr(ReturnExpression* expr)
{
	Result.Value = Eval(expr->Value).Value;
	Result.Result = StatementResult::Return;
}

void ExpressionEvaluator::Expr(SwitchExpression* expr)
{
	throw std::runtime_error("Switch expression is not implemented");
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
	throw std::runtime_error("Case expression is not implemented");
}

void ExpressionEvaluator::Expr(NothingExpression* expr)
{
	Result.Value = ExpressionValue::NothingValue();
}

void ExpressionEvaluator::Expr(LabelTableExpression* expr)
{
	throw std::runtime_error("Label table expression is not implemented");
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
	case ExpressionValueType::Nothing: break;
	case ExpressionValueType::Variable: lvalue.VariableProperty->CopyValue(lvalue.VariablePtr, rvalue.VariablePtr); break;
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
	Result.Value = lvalue;
}

void ExpressionEvaluator::Expr(LetBoolExpression* expr)
{
	ExpressionValue lvalue = Eval(expr->LeftSide).Value;
	ExpressionValue rvalue = Eval(expr->RightSide).Value;
	if (rvalue.Type == ExpressionValueType::Variable)
		lvalue.VariableProperty->CopyValue(lvalue.VariablePtr, rvalue.VariablePtr);
	else
		*static_cast<bool*>(lvalue.VariablePtr) = rvalue.Value.Bool;
	Result.Value = lvalue;
}

void ExpressionEvaluator::Expr(DynArrayElementExpression* expr)
{
	throw std::runtime_error("Dynamic array element expression is not implemented");
}

void ExpressionEvaluator::Expr(NewExpression* expr)
{
	throw std::runtime_error("New expression is not implemented");
}

void ExpressionEvaluator::Expr(ClassContextExpression* expr)
{
	throw std::runtime_error("Class context expression is not implemented");
}

void ExpressionEvaluator::Expr(MetaCastExpression* expr)
{
	throw std::runtime_error("Meta cast expression is not implemented");
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
	throw std::runtime_error("Skip expression is not implemented");
}

void ExpressionEvaluator::Expr(ContextExpression* expr)
{
	throw std::runtime_error("Context expression is not implemented");
}

void ExpressionEvaluator::Expr(ArrayElementExpression* expr)
{
	throw std::runtime_error("Array element expression is not implemented");
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
	throw std::runtime_error("Native parm expression is not implemented");
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
	throw std::runtime_error("Bool variable expression is not implemented");
}

void ExpressionEvaluator::Expr(DynamicCastExpression* expr)
{
	throw std::runtime_error("Dynamic cast expression is not implemented");
}

void ExpressionEvaluator::Expr(IteratorExpression* expr)
{
	throw std::runtime_error("Iterator expression is not implemented");
}

void ExpressionEvaluator::Expr(IteratorPopExpression* expr)
{
	throw std::runtime_error("Iterator pop expression is not implemented");
}

void ExpressionEvaluator::Expr(IteratorNextExpression* expr)
{
	throw std::runtime_error("Iterator next expression is not implemented");
}

void ExpressionEvaluator::Expr(StructCmpEqExpression* expr)
{
	throw std::runtime_error("Struct cmpeq expression is not implemented");
}

void ExpressionEvaluator::Expr(StructCmpNeExpression* expr)
{
	throw std::runtime_error("Struct cmpne expression is not implemented");
}

void ExpressionEvaluator::Expr(UnicodeStringConstExpression* expr)
{
	throw std::runtime_error("Unicode string expression is not implemented");
}

void ExpressionEvaluator::Expr(StructMemberExpression* expr)
{
	throw std::runtime_error("Struct member expression is not implemented");
}

void ExpressionEvaluator::Expr(RotatorToVectorExpression* expr)
{
	throw std::runtime_error("Rotator to vector expression is not implemented");
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
	throw std::runtime_error("String to vector expression is not implemented");
}

void ExpressionEvaluator::Expr(StringToRotatorExpression* expr)
{
	throw std::runtime_error("String to rotator expression is not implemented");
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
	throw std::runtime_error("Object to string expression is not implemented");
}

void ExpressionEvaluator::Expr(NameToStringExpression* expr)
{
	Result.Value = ExpressionValue::StringValue(Eval(expr->Value).Value.ToName());
}

void ExpressionEvaluator::Expr(VectorToStringExpression* expr)
{
	throw std::runtime_error("Vector to string expression is not implemented");
}

void ExpressionEvaluator::Expr(RotatorToStringExpression* expr)
{
	throw std::runtime_error("Rotator to string expression is not implemented");
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
	for (Expression* arg : exprArgs)
		args.push_back(Eval(arg).Value);
	Result.Value = Frame::Call(func, Self, args);
}
