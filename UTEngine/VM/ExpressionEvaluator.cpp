
#include "Precomp.h"
#include "ExpressionEvaluator.h"
#include "Expression.h"
#include "Bytecode.h"
#include "Frame.h"

ExpressionValue ExpressionEvaluator::Eval(Expression* expr)
{
	expr->Visit(this);
	return ResultValue;
}

void ExpressionEvaluator::Expr(LocalVariableExpression* expr)
{
}

void ExpressionEvaluator::Expr(InstanceVariableExpression* expr)
{
}

void ExpressionEvaluator::Expr(DefaultVariableExpression* expr)
{
}

void ExpressionEvaluator::Expr(ReturnExpression* expr)
{
}

void ExpressionEvaluator::Expr(SwitchExpression* expr)
{
}

void ExpressionEvaluator::Expr(JumpExpression* expr)
{
}

void ExpressionEvaluator::Expr(JumpIfNotExpression* expr)
{
}

void ExpressionEvaluator::Expr(StopExpression* expr)
{
}

void ExpressionEvaluator::Expr(AssertExpression* expr)
{
}

void ExpressionEvaluator::Expr(CaseExpression* expr)
{
}

void ExpressionEvaluator::Expr(NothingExpression* expr)
{
	ResultValue = ExpressionValue::NothingValue();
}

void ExpressionEvaluator::Expr(LabelTableExpression* expr)
{
}

void ExpressionEvaluator::Expr(GotoLabelExpression* expr)
{
}

void ExpressionEvaluator::Expr(EatStringExpression* expr)
{
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
}

void ExpressionEvaluator::Expr(SelfExpression* expr)
{
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
	ResultValue = ExpressionValue::IntValue(expr->Value);
}

void ExpressionEvaluator::Expr(FloatConstExpression* expr)
{
	ResultValue = ExpressionValue::FloatValue(expr->Value);
}

void ExpressionEvaluator::Expr(StringConstExpression* expr)
{
	ResultValue = ExpressionValue::StringValue(expr->Value);
}

void ExpressionEvaluator::Expr(ObjectConstExpression* expr)
{
	ResultValue = ExpressionValue::ObjectValue(expr->Object);
}

void ExpressionEvaluator::Expr(NameConstExpression* expr)
{
	ResultValue = ExpressionValue::NameValue(expr->Value);
}

void ExpressionEvaluator::Expr(RotationConstExpression* expr)
{
	ResultValue = ExpressionValue::RotatorValue({ expr->Pitch * (360.0f / 65536.0f), expr->Yaw * (360.0f / 65536.0f), expr->Roll * (360.0f / 65536.0f) });
}

void ExpressionEvaluator::Expr(VectorConstExpression* expr)
{
	ResultValue = ExpressionValue::VectorValue({ expr->X, expr->Y, expr->Z });
}

void ExpressionEvaluator::Expr(ByteConstExpression* expr)
{
	ResultValue = ExpressionValue::ByteValue(expr->Value);
}

void ExpressionEvaluator::Expr(IntZeroExpression* expr)
{
	ResultValue = ExpressionValue::IntValue(0);
}

void ExpressionEvaluator::Expr(IntOneExpression* expr)
{
	ResultValue = ExpressionValue::IntValue(1);
}

void ExpressionEvaluator::Expr(TrueExpression* expr)
{
	ResultValue = ExpressionValue::BoolValue(true);
}

void ExpressionEvaluator::Expr(FalseExpression* expr)
{
	ResultValue = ExpressionValue::BoolValue(false);
}

void ExpressionEvaluator::Expr(NativeParmExpression* expr)
{
}

void ExpressionEvaluator::Expr(NoObjectExpression* expr)
{
	ResultValue = ExpressionValue::ObjectValue(nullptr);
}

void ExpressionEvaluator::Expr(Unknown0x2bExpression* expr)
{
	throw std::runtime_error("Unknown0x2b expression encountered");
}

void ExpressionEvaluator::Expr(IntConstByteExpression* expr)
{
	ResultValue = ExpressionValue::ByteValue(expr->Value);
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
}

void ExpressionEvaluator::Expr(ByteToBoolExpression* expr)
{
}

void ExpressionEvaluator::Expr(ByteToFloatExpression* expr)
{
}

void ExpressionEvaluator::Expr(IntToByteExpression* expr)
{
}

void ExpressionEvaluator::Expr(IntToBoolExpression* expr)
{
}

void ExpressionEvaluator::Expr(IntToFloatExpression* expr)
{
}

void ExpressionEvaluator::Expr(BoolToByteExpression* expr)
{
}

void ExpressionEvaluator::Expr(BoolToIntExpression* expr)
{
}

void ExpressionEvaluator::Expr(BoolToFloatExpression* expr)
{
}

void ExpressionEvaluator::Expr(FloatToByteExpression* expr)
{
}

void ExpressionEvaluator::Expr(FloatToIntExpression* expr)
{
}

void ExpressionEvaluator::Expr(FloatToBoolExpression* expr)
{
}

void ExpressionEvaluator::Expr(Unknown0x46Expression* expr)
{
	throw std::runtime_error("Unknown0x46 expression encountered");
}

void ExpressionEvaluator::Expr(ObjectToBoolExpression* expr)
{
}

void ExpressionEvaluator::Expr(NameToBoolExpression* expr)
{
}

void ExpressionEvaluator::Expr(StringToByteExpression* expr)
{
}

void ExpressionEvaluator::Expr(StringToIntExpression* expr)
{
}

void ExpressionEvaluator::Expr(StringToBoolExpression* expr)
{
}

void ExpressionEvaluator::Expr(StringToFloatExpression* expr)
{
}

void ExpressionEvaluator::Expr(StringToVectorExpression* expr)
{
}

void ExpressionEvaluator::Expr(StringToRotatorExpression* expr)
{
}

void ExpressionEvaluator::Expr(VectorToBoolExpression* expr)
{
}

void ExpressionEvaluator::Expr(VectorToRotatorExpression* expr)
{
}

void ExpressionEvaluator::Expr(RotatorToBoolExpression* expr)
{
}

void ExpressionEvaluator::Expr(ByteToStringExpression* expr)
{
}

void ExpressionEvaluator::Expr(IntToStringExpression* expr)
{
}

void ExpressionEvaluator::Expr(BoolToStringExpression* expr)
{
}

void ExpressionEvaluator::Expr(FloatToStringExpression* expr)
{
}

void ExpressionEvaluator::Expr(ObjectToStringExpression* expr)
{
}

void ExpressionEvaluator::Expr(NameToStringExpression* expr)
{
}

void ExpressionEvaluator::Expr(VectorToStringExpression* expr)
{
}

void ExpressionEvaluator::Expr(RotatorToStringExpression* expr)
{
}

void ExpressionEvaluator::Expr(VirtualFunctionExpression* expr)
{
}

void ExpressionEvaluator::Expr(FinalFunctionExpression* expr)
{
}

void ExpressionEvaluator::Expr(GlobalFunctionExpression* expr)
{
}

void ExpressionEvaluator::Expr(NativeFunctionExpression* expr)
{
}
