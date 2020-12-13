#pragma once

class UObject;
class UClass;
class UFunction;
class UProperty;

class Expression
{
public:
	virtual ~Expression() = default;
};

class LocalVariableExpression : public Expression
{
public:
	UProperty* Variable = nullptr;
};

class InstanceVariableExpression : public Expression
{
public:
	UProperty* Variable = nullptr;
};

class DefaultVariableExpression : public Expression
{
public:
	UProperty* Variable = nullptr;
};

class ReturnExpression : public Expression
{
public:
	Expression* Value = nullptr;
};

class SwitchExpression : public Expression
{
public:
	uint8_t Size = 0;
	Expression* Condition = nullptr;
};

class JumpExpression : public Expression
{
public:
	uint16_t Offset = 0;
};

class JumpIfNotExpression : public Expression
{
public:
	uint16_t Offset = 0;
	Expression* Condition = nullptr;
};

class StopExpression : public Expression
{
public:
};

class AssertExpression : public Expression
{
public:
	uint16_t Line = 0;
	Expression* Condition = nullptr;
};

class CaseExpression : public Expression
{
public:
	uint16_t NextOffset = 0;
	Expression* Value = nullptr;
};

class NothingExpression : public Expression
{
public:
};

struct LabelEntry
{
	std::string Name;
	uint32_t Offset = 0;
};

class LabelTableExpression : public Expression
{
public:
	std::vector<LabelEntry> Labels;
};

class GotoLabelExpression : public Expression
{
public:
	Expression* Value = nullptr;
};

class EatStringExpression : public Expression
{
public:
	Expression* Value = nullptr;
};

class LetExpression : public Expression
{
public:
	Expression* LeftSide = nullptr;
	Expression* RightSide = nullptr;
};

class DynArrayElementExpression : public Expression
{
public:
	Expression* Index = nullptr;
	Expression* Array = nullptr;
};

class NewExpression : public Expression
{
public:
	Expression* ParentExpr = nullptr;
	Expression* NameExpr = nullptr;
	Expression* FlagsExpr = nullptr;
	Expression* ClassExpr = nullptr;
};

class ClassContextExpression : public Expression
{
public:
	Expression* ObjectExpr = nullptr;
	uint16_t NullExprCodeOffset = 0;
	uint8_t ZeroFillSize = 0;
	Expression* ContextExpr = nullptr;
};

class MetaCastExpression : public Expression
{
public:
	UClass* Class = nullptr;
	Expression* Value = nullptr;
};

class LetBoolExpression : public Expression
{
public:
	Expression* LeftSide = nullptr;
	Expression* RightSide = nullptr;
};

class Unknown0x15Expression : public Expression
{
public:
	Expression* Value = nullptr;
};

class SelfExpression : public Expression
{
public:
};

class SkipExpression : public Expression
{
public:
	uint16_t Skip = 0;
	Expression* Value = nullptr;
};

class ContextExpression : public Expression
{
public:
	Expression* ObjectExpr = nullptr;
	uint16_t NullExprCodeOffset = 0;
	uint8_t ZeroFillSize = 0;
	Expression* ContextExpr = nullptr;
};

class ArrayElementExpression : public Expression
{
public:
	Expression* Index = nullptr;
	Expression* Array = nullptr;
};

class IntConstExpression : public Expression
{
public:
	uint32_t Value = 0;
};

class FloatConstExpression : public Expression
{
public:
	float Value = 0.0f;
};

class StringConstExpression : public Expression
{
public:
	std::string Value;
};

class ObjectConstExpression : public Expression
{
public:
	UObject* Object = nullptr;
};

class NameConstExpression : public Expression
{
public:
	std::string Value;
};

class RotationConstExpression : public Expression
{
public:
	uint32_t Pitch = 0;
	uint32_t Yaw = 0;
	uint32_t Roll = 0;
};

class VectorConstExpression : public Expression
{
public:
	float X = 0.0f;
	float Y = 0.0f;
	float Z = 0.0f;
};

class ByteConstExpression : public Expression
{
public:
	uint8_t Value = 0;
};

class IntZeroExpression : public Expression
{
public:
};

class IntOneExpression : public Expression
{
public:
};

class TrueExpression : public Expression
{
public:
};

class FalseExpression : public Expression
{
public:
};

class NativeParmExpression : public Expression
{
public:
	UObject* Object = nullptr;
};

class NoObjectExpression : public Expression
{
public:
};

class Unknown0x2bExpression : public Expression
{
public:
	uint8_t Unknown = 0;
	Expression* Value = nullptr;
};

class IntConstByteExpression : public Expression
{
public:
	uint8_t Value = 0;
};

class BoolVariableExpression : public Expression
{
public:
	Expression* Value = nullptr;
};

class DynamicCastExpression : public Expression
{
public:
	UClass* Class = nullptr;
	Expression* Value = nullptr;
};

class IteratorExpression : public Expression
{
public:
	Expression* Value = nullptr;
	uint16_t Offset = 0;
};

class IteratorPopExpression : public Expression
{
public:
};

class IteratorNextExpression : public Expression
{
public:
};

class StructCmpEqExpression : public Expression
{
public:
	UObject* Struct = nullptr;
	Expression* Value1 = nullptr;
	Expression* Value2 = nullptr;
};

class StructCmpNeExpression : public Expression
{
public:
	UObject* Struct = nullptr;
	Expression* Value1 = nullptr;
	Expression* Value2 = nullptr;
};

class UnicodeStringConstExpression : public Expression
{
public:
	std::wstring Value;
};

class StructMemberExpression : public Expression
{
public:
	UObject* Object = nullptr;
	Expression* Value = nullptr;
};

class RotatorToVectorExpression : public Expression
{
public:
	Expression* Value = nullptr;
};

class ByteToIntExpression : public Expression
{
public:
	Expression* Value = nullptr;
};

class ByteToBoolExpression : public Expression
{
public:
	Expression* Value = nullptr;
};

class ByteToFloatExpression : public Expression
{
public:
	Expression* Value = nullptr;
};

class IntToByteExpression : public Expression
{
public:
	Expression* Value = nullptr;
};

class IntToBoolExpression : public Expression
{
public:
	Expression* Value = nullptr;
};

class IntToFloatExpression : public Expression
{
public:
	Expression* Value = nullptr;
};

class BoolToByteExpression : public Expression
{
public:
	Expression* Value = nullptr;
};

class BoolToIntExpression : public Expression
{
public:
	Expression* Value = nullptr;
};

class BoolToFloatExpression : public Expression
{
public:
	Expression* Value = nullptr;
};

class FloatToByteExpression : public Expression
{
public:
	Expression* Value = nullptr;
};

class FloatToIntExpression : public Expression
{
public:
	Expression* Value = nullptr;
};

class FloatToBoolExpression : public Expression
{
public:
	Expression* Value = nullptr;
};

class Unknown0x46Expression : public Expression
{
public:
	Expression* Value = nullptr;
};

class ObjectToBoolExpression : public Expression
{
public:
	Expression* Value = nullptr;
};

class NameToBoolExpression : public Expression
{
public:
	Expression* Value = nullptr;
};

class StringToByteExpression : public Expression
{
public:
	Expression* Value = nullptr;
};

class StringToIntExpression : public Expression
{
public:
	Expression* Value = nullptr;
};

class StringToBoolExpression : public Expression
{
public:
	Expression* Value = nullptr;
};

class StringToFloatExpression : public Expression
{
public:
	Expression* Value = nullptr;
};

class StringToVectorExpression : public Expression
{
public:
	Expression* Value = nullptr;
};

class StringToRotatorExpression : public Expression
{
public:
	Expression* Value = nullptr;
};

class VectorToBoolExpression : public Expression
{
public:
	Expression* Value = nullptr;
};

class VectorToRotatorExpression : public Expression
{
public:
	Expression* Value = nullptr;
};

class RotatorToBoolExpression : public Expression
{
public:
	Expression* Value = nullptr;
};

class ByteToStringExpression : public Expression
{
public:
	Expression* Value = nullptr;
};

class IntToStringExpression : public Expression
{
public:
	Expression* Value = nullptr;
};

class BoolToStringExpression : public Expression
{
public:
	Expression* Value = nullptr;
};

class FloatToStringExpression : public Expression
{
public:
	Expression* Value = nullptr;
};

class ObjectToStringExpression : public Expression
{
public:
	Expression* Value = nullptr;
};

class NameToStringExpression : public Expression
{
public:
	Expression* Value = nullptr;
};

class VectorToStringExpression : public Expression
{
public:
	Expression* Value = nullptr;
};

class RotatorToStringExpression : public Expression
{
public:
	Expression* Value = nullptr;
};

class VirtualFunctionExpression : public Expression
{
public:
	std::string Name;
	std::vector<Expression*> Args;
};

class FinalFunctionExpression : public Expression
{
public:
	UFunction* Func = nullptr;
	std::vector<Expression*> Args;
};

class GlobalFunctionExpression : public Expression
{
public:
	std::string Name;
	std::vector<Expression*> Args;
};

class NativeFunctionExpression : public Expression
{
public:
	int nativeindex;
	std::vector<Expression*> Args;
};
