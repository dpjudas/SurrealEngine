#pragma once

#include "VM/ExpressionVisitor.h"

class TextListViewItem;
class UFunction;

class ExpressionItemBuilder : public ExpressionVisitor
{
public:
	static std::unique_ptr<TextListViewItem> createItem(const std::string& name, Expression* expr);

private:
	void Expr(LocalVariableExpression* expr) override;
	void Expr(InstanceVariableExpression* expr) override;
	void Expr(DefaultVariableExpression* expr) override;
	void Expr(ReturnExpression* expr) override;
	void Expr(SwitchExpression* expr) override;
	void Expr(JumpExpression* expr) override;
	void Expr(JumpIfNotExpression* expr) override;
	void Expr(StopExpression* expr) override;
	void Expr(AssertExpression* expr) override;
	void Expr(CaseExpression* expr) override;
	void Expr(NothingExpression* expr) override;
	void Expr(LabelTableExpression* expr) override;
	void Expr(GotoLabelExpression* expr) override;
	void Expr(EatStringExpression* expr) override;
	void Expr(LetExpression* expr) override;
	void Expr(DynArrayElementExpression* expr) override;
	void Expr(NewExpression* expr) override;
	void Expr(ClassContextExpression* expr) override;
	void Expr(MetaCastExpression* expr) override;
	void Expr(LetBoolExpression* expr) override;
	void Expr(Unknown0x15Expression* expr) override;
	void Expr(SelfExpression* expr) override;
	void Expr(SkipExpression* expr) override;
	void Expr(ContextExpression* expr) override;
	void Expr(ArrayElementExpression* expr) override;
	void Expr(IntConstExpression* expr) override;
	void Expr(FloatConstExpression* expr) override;
	void Expr(StringConstExpression* expr) override;
	void Expr(ObjectConstExpression* expr) override;
	void Expr(NameConstExpression* expr) override;
	void Expr(RotationConstExpression* expr) override;
	void Expr(VectorConstExpression* expr) override;
	void Expr(ByteConstExpression* expr) override;
	void Expr(IntZeroExpression* expr) override;
	void Expr(IntOneExpression* expr) override;
	void Expr(TrueExpression* expr) override;
	void Expr(FalseExpression* expr) override;
	void Expr(NativeParmExpression* expr) override;
	void Expr(NoObjectExpression* expr) override;
	void Expr(Unknown0x2bExpression* expr) override;
	void Expr(IntConstByteExpression* expr) override;
	void Expr(BoolVariableExpression* expr) override;
	void Expr(DynamicCastExpression* expr) override;
	void Expr(IteratorExpression* expr) override;
	void Expr(IteratorPopExpression* expr) override;
	void Expr(IteratorNextExpression* expr) override;
	void Expr(StructCmpEqExpression* expr) override;
	void Expr(StructCmpNeExpression* expr) override;
	void Expr(UnicodeStringConstExpression* expr) override;
	void Expr(StructMemberExpression* expr) override;
	void Expr(RotatorToVectorExpression* expr) override;
	void Expr(ByteToIntExpression* expr) override;
	void Expr(ByteToBoolExpression* expr) override;
	void Expr(ByteToFloatExpression* expr) override;
	void Expr(IntToByteExpression* expr) override;
	void Expr(IntToBoolExpression* expr) override;
	void Expr(IntToFloatExpression* expr) override;
	void Expr(BoolToByteExpression* expr) override;
	void Expr(BoolToIntExpression* expr) override;
	void Expr(BoolToFloatExpression* expr) override;
	void Expr(FloatToByteExpression* expr) override;
	void Expr(FloatToIntExpression* expr) override;
	void Expr(FloatToBoolExpression* expr) override;
	void Expr(Unknown0x46Expression* expr) override;
	void Expr(ObjectToBoolExpression* expr) override;
	void Expr(NameToBoolExpression* expr) override;
	void Expr(StringToByteExpression* expr) override;
	void Expr(StringToIntExpression* expr) override;
	void Expr(StringToBoolExpression* expr) override;
	void Expr(StringToFloatExpression* expr) override;
	void Expr(StringToVectorExpression* expr) override;
	void Expr(StringToRotatorExpression* expr) override;
	void Expr(VectorToBoolExpression* expr) override;
	void Expr(VectorToRotatorExpression* expr) override;
	void Expr(RotatorToBoolExpression* expr) override;
	void Expr(ByteToStringExpression* expr) override;
	void Expr(IntToStringExpression* expr) override;
	void Expr(BoolToStringExpression* expr) override;
	void Expr(FloatToStringExpression* expr) override;
	void Expr(ObjectToStringExpression* expr) override;
	void Expr(NameToStringExpression* expr) override;
	void Expr(VectorToStringExpression* expr) override;
	void Expr(RotatorToStringExpression* expr) override;
	void Expr(VirtualFunctionExpression* expr) override;
	void Expr(FinalFunctionExpression* expr) override;
	void Expr(GlobalFunctionExpression* expr) override;
	void Expr(NativeFunctionExpression* expr) override;
	void Expr(FunctionArgumentsExpression* expr) override;

	static std::string GetFullFuncName(UFunction* func);

	std::unique_ptr<TextListViewItem> item;
};
