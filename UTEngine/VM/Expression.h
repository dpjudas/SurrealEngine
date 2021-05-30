#pragma once

#include "ExpressionVisitor.h"

class UObject;
class UClass;
class UFunction;
class UProperty;

class Expression
{
public:
	virtual ~Expression() = default;
	virtual void Visit(ExpressionVisitor* visitor) = 0;

	int StatementIndex = -1;
};

class LocalVariableExpression : public Expression
{
public:
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	UProperty* Variable = nullptr;
};

class InstanceVariableExpression : public Expression
{
public:
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	UProperty* Variable = nullptr;
};

class DefaultVariableExpression : public Expression
{
public:
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	UProperty* Variable = nullptr;
};

class ReturnExpression : public Expression
{
public:
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* Value = nullptr;
};

class SwitchExpression : public Expression
{
public:
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	uint8_t Size = 0;
	Expression* Condition = nullptr;
};

class JumpExpression : public Expression
{
public:
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	uint16_t Offset = 0;
};

class JumpIfNotExpression : public Expression
{
public:
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	uint16_t Offset = 0;
	Expression* Condition = nullptr;
};

class StopExpression : public Expression
{
public:
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }
};

class AssertExpression : public Expression
{
public:
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	uint16_t Line = 0;
	Expression* Condition = nullptr;
};

class CaseExpression : public Expression
{
public:
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	uint16_t NextOffset = 0;
	Expression* Value = nullptr;
};

class NothingExpression : public Expression
{
public:
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }
};

struct LabelEntry
{
	std::string Name;
	uint32_t Offset = 0;
};

class LabelTableExpression : public Expression
{
public:
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	std::vector<LabelEntry> Labels;
};

class GotoLabelExpression : public Expression
{
public:
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* Value = nullptr;
};

class EatStringExpression : public Expression
{
public:
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* Value = nullptr;
};

class LetExpression : public Expression
{
public:
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* LeftSide = nullptr;
	Expression* RightSide = nullptr;
};

class DynArrayElementExpression : public Expression
{
public:
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* Index = nullptr;
	Expression* Array = nullptr;
};

class NewExpression : public Expression
{
public:
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* ParentExpr = nullptr;
	Expression* NameExpr = nullptr;
	Expression* FlagsExpr = nullptr;
	Expression* ClassExpr = nullptr;
};

class ClassContextExpression : public Expression
{
public:
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* ObjectExpr = nullptr;
	uint16_t NullExprCodeOffset = 0;
	uint8_t ZeroFillSize = 0;
	Expression* ContextExpr = nullptr;
};

class MetaCastExpression : public Expression
{
public:
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	UClass* Class = nullptr;
	Expression* Value = nullptr;
};

class LetBoolExpression : public Expression
{
public:
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* LeftSide = nullptr;
	Expression* RightSide = nullptr;
};

class Unknown0x15Expression : public Expression
{
public:
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* Value = nullptr;
};

class SelfExpression : public Expression
{
public:
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }
};

class SkipExpression : public Expression
{
public:
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	uint16_t Skip = 0;
	Expression* Value = nullptr;
};

class ContextExpression : public Expression
{
public:
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* ObjectExpr = nullptr;
	uint16_t NullExprCodeOffset = 0;
	uint8_t ZeroFillSize = 0;
	Expression* ContextExpr = nullptr;
};

class ArrayElementExpression : public Expression
{
public:
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* Index = nullptr;
	Expression* Array = nullptr;
};

class IntConstExpression : public Expression
{
public:
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	uint32_t Value = 0;
};

class FloatConstExpression : public Expression
{
public:
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	float Value = 0.0f;
};

class StringConstExpression : public Expression
{
public:
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	std::string Value;
};

class ObjectConstExpression : public Expression
{
public:
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	UObject* Object = nullptr;
};

class NameConstExpression : public Expression
{
public:
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	std::string Value;
};

class RotationConstExpression : public Expression
{
public:
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	int32_t Pitch = 0;
	int32_t Yaw = 0;
	int32_t Roll = 0;
};

class VectorConstExpression : public Expression
{
public:
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	float X = 0.0f;
	float Y = 0.0f;
	float Z = 0.0f;
};

class ByteConstExpression : public Expression
{
public:
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	uint8_t Value = 0;
};

class IntZeroExpression : public Expression
{
public:
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }
};

class IntOneExpression : public Expression
{
public:
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }
};

class TrueExpression : public Expression
{
public:
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }
};

class FalseExpression : public Expression
{
public:
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }
};

class NativeParmExpression : public Expression
{
public:
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	UObject* Object = nullptr;
};

class NoObjectExpression : public Expression
{
public:
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }
};

class Unknown0x2bExpression : public Expression
{
public:
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	uint8_t Unknown = 0;
	Expression* Value = nullptr;
};

class IntConstByteExpression : public Expression
{
public:
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	uint8_t Value = 0;
};

class BoolVariableExpression : public Expression
{
public:
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* Value = nullptr;
};

class DynamicCastExpression : public Expression
{
public:
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	UClass* Class = nullptr;
	Expression* Value = nullptr;
};

class IteratorExpression : public Expression
{
public:
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* Value = nullptr;
	uint16_t Offset = 0;
};

class IteratorPopExpression : public Expression
{
public:
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }
};

class IteratorNextExpression : public Expression
{
public:
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }
};

class StructCmpEqExpression : public Expression
{
public:
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	UObject* Struct = nullptr;
	Expression* Value1 = nullptr;
	Expression* Value2 = nullptr;
};

class StructCmpNeExpression : public Expression
{
public:
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	UObject* Struct = nullptr;
	Expression* Value1 = nullptr;
	Expression* Value2 = nullptr;
};

class UnicodeStringConstExpression : public Expression
{
public:
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	std::wstring Value;
};

class StructMemberExpression : public Expression
{
public:
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	UObject* Object = nullptr;
	Expression* Value = nullptr;
};

class RotatorToVectorExpression : public Expression
{
public:
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* Value = nullptr;
};

class ByteToIntExpression : public Expression
{
public:
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* Value = nullptr;
};

class ByteToBoolExpression : public Expression
{
public:
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* Value = nullptr;
};

class ByteToFloatExpression : public Expression
{
public:
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* Value = nullptr;
};

class IntToByteExpression : public Expression
{
public:
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* Value = nullptr;
};

class IntToBoolExpression : public Expression
{
public:
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* Value = nullptr;
};

class IntToFloatExpression : public Expression
{
public:
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* Value = nullptr;
};

class BoolToByteExpression : public Expression
{
public:
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* Value = nullptr;
};

class BoolToIntExpression : public Expression
{
public:
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* Value = nullptr;
};

class BoolToFloatExpression : public Expression
{
public:
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* Value = nullptr;
};

class FloatToByteExpression : public Expression
{
public:
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* Value = nullptr;
};

class FloatToIntExpression : public Expression
{
public:
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* Value = nullptr;
};

class FloatToBoolExpression : public Expression
{
public:
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* Value = nullptr;
};

class Unknown0x46Expression : public Expression
{
public:
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* Value = nullptr;
};

class ObjectToBoolExpression : public Expression
{
public:
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* Value = nullptr;
};

class NameToBoolExpression : public Expression
{
public:
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* Value = nullptr;
};

class StringToByteExpression : public Expression
{
public:
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* Value = nullptr;
};

class StringToIntExpression : public Expression
{
public:
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* Value = nullptr;
};

class StringToBoolExpression : public Expression
{
public:
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* Value = nullptr;
};

class StringToFloatExpression : public Expression
{
public:
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* Value = nullptr;
};

class StringToVectorExpression : public Expression
{
public:
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* Value = nullptr;
};

class StringToRotatorExpression : public Expression
{
public:
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* Value = nullptr;
};

class VectorToBoolExpression : public Expression
{
public:
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* Value = nullptr;
};

class VectorToRotatorExpression : public Expression
{
public:
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* Value = nullptr;
};

class RotatorToBoolExpression : public Expression
{
public:
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* Value = nullptr;
};

class ByteToStringExpression : public Expression
{
public:
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* Value = nullptr;
};

class IntToStringExpression : public Expression
{
public:
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* Value = nullptr;
};

class BoolToStringExpression : public Expression
{
public:
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* Value = nullptr;
};

class FloatToStringExpression : public Expression
{
public:
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* Value = nullptr;
};

class ObjectToStringExpression : public Expression
{
public:
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* Value = nullptr;
};

class NameToStringExpression : public Expression
{
public:
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* Value = nullptr;
};

class VectorToStringExpression : public Expression
{
public:
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* Value = nullptr;
};

class RotatorToStringExpression : public Expression
{
public:
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* Value = nullptr;
};

class VirtualFunctionExpression : public Expression
{
public:
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	std::string Name;
	std::vector<Expression*> Args;
};

class FinalFunctionExpression : public Expression
{
public:
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	UFunction* Func = nullptr;
	std::vector<Expression*> Args;
};

class GlobalFunctionExpression : public Expression
{
public:
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	std::string Name;
	std::vector<Expression*> Args;
};

class NativeFunctionExpression : public Expression
{
public:
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	int nativeindex;
	std::vector<Expression*> Args;
};
