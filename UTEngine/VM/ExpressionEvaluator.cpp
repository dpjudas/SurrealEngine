
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

	NativeFunctions::NativeByIndex[expr->nativeindex](Self, args.data());
}

void ExpressionEvaluator::Call(UFunction* func, const std::vector<Expression*>& exprArgs)
{
	std::vector<ExpressionValue> args;
	args.reserve(exprArgs.size());
	for (Expression* arg : exprArgs) args.push_back(Eval(arg).Value);

	if (AllFlags(func->FuncFlags, FunctionFlags::Native))
	{
		if (func->NativeFuncIndex != 0)
		{
			NativeFunctions::NativeByIndex[func->NativeFuncIndex](Self, args.data());
		}
		else
		{
			NativeFunctions::NativeByName[{ func->Name, func->BaseField->Name }](Self, args.data());
		}
	}
	else
	{
		// Call script function
	}
}
