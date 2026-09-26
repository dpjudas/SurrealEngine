#pragma once

#include "ExpressionVisitor.h"
#include "Package/NameString.h"

class UObject;
class UClass;
class UStruct;
class UFunction;
class UProperty;

enum class ExpressionType : uint8_t
{
	LocalVariable,
	InstanceVariable,
	DefaultVariable,
	Return,
	Switch,
	Jump,
	JumpIfNot,
	Stop,
	Assert,
	Case,
	Nothing,
	LabelTable,
	GotoLabel,
	EatString,
	Let,
	DynArrayElement,
	New,
	ClassContext,
	MetaCast,
	LetBool,
	Unknown0x15,
	Self,
	Skip,
	Context,
	ArrayElement,
	IntConst,
	FloatConst,
	StringConst,
	ObjectConst,
	NameConst,
	RotationConst,
	VectorConst,
	ByteConst,
	IntZero,
	IntOne,
	True,
	False,
	NativeParm,
	NoObject,
	Unknown0x2b,
	IntConstByte,
	BoolVariable,
	DynamicCast,
	Iterator,
	IteratorPop,
	IteratorNext,
	StructCmpEq,
	StructCmpNe,
	UnicodeStringConst,
	StructMember,
	RotatorToVector,
	ByteToInt,
	ByteToBool,
	ByteToFloat,
	IntToByte,
	IntToBool,
	IntToFloat,
	BoolToByte,
	BoolToInt,
	BoolToFloat,
	FloatToByte,
	FloatToInt,
	FloatToBool,
	Unknown0x46,
	ObjectToBool,
	NameToBool,
	StringToByte,
	StringToInt,
	StringToBool,
	StringToFloat,
	StringToVector,
	StringToRotator,
	VectorToBool,
	VectorToRotator,
	RotatorToBool,
	ByteToString,
	IntToString,
	BoolToString,
	FloatToString,
	ObjectToString,
	NameToString,
	VectorToString,
	RotatorToString,
	StringToName,
	DynArrayToInt,
	VirtualFunction,
	FinalFunction,
	GlobalFunction,
	NativeFunction,
	FunctionArguments,
	Construct,
};

class Expression
{
public:
	Expression(ExpressionType type) : Type(type) {}
	virtual ~Expression() = default;
	virtual void Visit(ExpressionVisitor* visitor) = 0;

	ExpressionType Type;
	int StatementIndex = -1;
};

class LocalVariableExpression : public Expression
{
public:
	LocalVariableExpression() : Expression(ExpressionType::LocalVariable) {}
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	UProperty* Variable = nullptr;
};

class InstanceVariableExpression : public Expression
{
public:
	InstanceVariableExpression() : Expression(ExpressionType::InstanceVariable) {}
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	UProperty* Variable = nullptr;
};

class DefaultVariableExpression : public Expression
{
public:
	DefaultVariableExpression() : Expression(ExpressionType::DefaultVariable) {}
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	UProperty* Variable = nullptr;
};

class ReturnExpression : public Expression
{
public:
	ReturnExpression() : Expression(ExpressionType::Return) {}
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* Value = nullptr;
};

class SwitchExpression : public Expression
{
public:
	SwitchExpression() : Expression(ExpressionType::Switch) {}
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	int Size = 0;
	Expression* Condition = nullptr;
};

class JumpExpression : public Expression
{
public:
	JumpExpression() : Expression(ExpressionType::Jump) {}
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	uint16_t Offset = 0;
};

class JumpIfNotExpression : public Expression
{
public:
	JumpIfNotExpression() : Expression(ExpressionType::JumpIfNot) {}
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	uint16_t Offset = 0;
	Expression* Condition = nullptr;
};

class StopExpression : public Expression
{
public:
	StopExpression() : Expression(ExpressionType::Stop) {}
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }
};

class AssertExpression : public Expression
{
public:
	AssertExpression() : Expression(ExpressionType::Assert) {}
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	uint16_t Line = 0;
	Expression* Condition = nullptr;
};

class CaseExpression : public Expression
{
public:
	CaseExpression() : Expression(ExpressionType::Case) {}
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	uint16_t NextOffset = 0;
	Expression* Value = nullptr;
};

class NothingExpression : public Expression
{
public:
	NothingExpression() : Expression(ExpressionType::Nothing) {}
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }
};

struct LabelEntry
{
	NameString Name;
	uint32_t Offset = 0;
};

class LabelTableExpression : public Expression
{
public:
	LabelTableExpression() : Expression(ExpressionType::LabelTable) {}
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Array<LabelEntry> Labels;
};

class GotoLabelExpression : public Expression
{
public:
	GotoLabelExpression() : Expression(ExpressionType::GotoLabel) {}
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* Value = nullptr;
};

class EatStringExpression : public Expression
{
public:
	EatStringExpression() : Expression(ExpressionType::EatString) {}
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* Value = nullptr;
};

class LetExpression : public Expression
{
public:
	LetExpression() : Expression(ExpressionType::Let) {}
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* LeftSide = nullptr;
	Expression* RightSide = nullptr;
};

class DynArrayElementExpression : public Expression
{
public:
	DynArrayElementExpression() : Expression(ExpressionType::DynArrayElement) {}
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* Index = nullptr;
	Expression* Array = nullptr;
};

class NewExpression : public Expression
{
public:
	NewExpression() : Expression(ExpressionType::New) {}
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* ParentExpr = nullptr;
	Expression* NameExpr = nullptr;
	Expression* FlagsExpr = nullptr;
	Expression* ClassExpr = nullptr;
};

class ClassContextExpression : public Expression
{
public:
	ClassContextExpression() : Expression(ExpressionType::ClassContext) {}
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* ObjectExpr = nullptr;
	uint16_t NullExprCodeOffset = 0;
	uint8_t ZeroFillSize = 0;
	Expression* ContextExpr = nullptr;
};

class MetaCastExpression : public Expression
{
public:
	MetaCastExpression() : Expression(ExpressionType::MetaCast) {}
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	UClass* Class = nullptr;
	Expression* Value = nullptr;
};

class LetBoolExpression : public Expression
{
public:
	LetBoolExpression() : Expression(ExpressionType::LetBool) {}
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* LeftSide = nullptr;
	Expression* RightSide = nullptr;
};

class Unknown0x15Expression : public Expression
{
public:
	Unknown0x15Expression() : Expression(ExpressionType::Unknown0x15) {}
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* Value = nullptr;
};

class SelfExpression : public Expression
{
public:
	SelfExpression() : Expression(ExpressionType::Self) {}
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }
};

class SkipExpression : public Expression
{
public:
	SkipExpression() : Expression(ExpressionType::Skip) {}
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	uint16_t Skip = 0;
	Expression* Value = nullptr;
};

class ContextExpression : public Expression
{
public:
	ContextExpression() : Expression(ExpressionType::Context) {}
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* ObjectExpr = nullptr;
	uint16_t NullExprCodeOffset = 0;
	uint8_t ZeroFillSize = 0;
	Expression* ContextExpr = nullptr;
};

class ArrayElementExpression : public Expression
{
public:
	ArrayElementExpression() : Expression(ExpressionType::ArrayElement) {}
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* Index = nullptr;
	Expression* Array = nullptr;
};

class IntConstExpression : public Expression
{
public:
	IntConstExpression() : Expression(ExpressionType::IntConst) {}
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	uint32_t Value = 0;
};

class FloatConstExpression : public Expression
{
public:
	FloatConstExpression() : Expression(ExpressionType::FloatConst) {}
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	float Value = 0.0f;
};

class StringConstExpression : public Expression
{
public:
	StringConstExpression() : Expression(ExpressionType::StringConst) {}
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	std::string Value;
};

class ObjectConstExpression : public Expression
{
public:
	ObjectConstExpression() : Expression(ExpressionType::ObjectConst) {}
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	UObject* Object = nullptr;
};

class NameConstExpression : public Expression
{
public:
	NameConstExpression() : Expression(ExpressionType::NameConst) {}
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	NameString Value;
};

class RotationConstExpression : public Expression
{
public:
	RotationConstExpression() : Expression(ExpressionType::RotationConst) {}
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	int32_t Pitch = 0;
	int32_t Yaw = 0;
	int32_t Roll = 0;
};

class VectorConstExpression : public Expression
{
public:
	VectorConstExpression() : Expression(ExpressionType::VectorConst) {}
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	float X = 0.0f;
	float Y = 0.0f;
	float Z = 0.0f;
};

class ByteConstExpression : public Expression
{
public:
	ByteConstExpression() : Expression(ExpressionType::ByteConst) {}
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	uint8_t Value = 0;
};

class IntZeroExpression : public Expression
{
public:
	IntZeroExpression() : Expression(ExpressionType::IntZero) {}
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }
};

class IntOneExpression : public Expression
{
public:
	IntOneExpression() : Expression(ExpressionType::IntOne) {}
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }
};

class TrueExpression : public Expression
{
public:
	TrueExpression() : Expression(ExpressionType::True) {}
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }
};

class FalseExpression : public Expression
{
public:
	FalseExpression() : Expression(ExpressionType::False) {}
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }
};

class NativeParmExpression : public Expression
{
public:
	NativeParmExpression() : Expression(ExpressionType::NativeParm) {}
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	UObject* Object = nullptr;
};

class NoObjectExpression : public Expression
{
public:
	NoObjectExpression() : Expression(ExpressionType::NoObject) {}
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }
};

class Unknown0x2bExpression : public Expression
{
public:
	Unknown0x2bExpression() : Expression(ExpressionType::Unknown0x2b) {}
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	uint8_t Unknown = 0;
	Expression* Value = nullptr;
};

class IntConstByteExpression : public Expression
{
public:
	IntConstByteExpression() : Expression(ExpressionType::IntConstByte) {}
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	uint8_t Value = 0;
};

class BoolVariableExpression : public Expression
{
public:
	BoolVariableExpression() : Expression(ExpressionType::BoolVariable) {}
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* Variable = nullptr;
};

class DynamicCastExpression : public Expression
{
public:
	DynamicCastExpression() : Expression(ExpressionType::DynamicCast) {}
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	UClass* Class = nullptr;
	Expression* Value = nullptr;
};

class IteratorExpression : public Expression
{
public:
	IteratorExpression() : Expression(ExpressionType::Iterator) {}
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* Value = nullptr;
	uint16_t Offset = 0;
};

class IteratorPopExpression : public Expression
{
public:
	IteratorPopExpression() : Expression(ExpressionType::IteratorPop) {}
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }
};

class IteratorNextExpression : public Expression
{
public:
	IteratorNextExpression() : Expression(ExpressionType::IteratorNext) {}
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }
};

class StructCmpEqExpression : public Expression
{
public:
	StructCmpEqExpression() : Expression(ExpressionType::StructCmpEq) {}
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	UObject* Struct = nullptr;
	Expression* Value1 = nullptr;
	Expression* Value2 = nullptr;
};

class StructCmpNeExpression : public Expression
{
public:
	StructCmpNeExpression() : Expression(ExpressionType::StructCmpNe) {}
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	UObject* Struct = nullptr;
	Expression* Value1 = nullptr;
	Expression* Value2 = nullptr;
};

class UnicodeStringConstExpression : public Expression
{
public:
	UnicodeStringConstExpression() : Expression(ExpressionType::UnicodeStringConst) {}
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	std::wstring Value;
};

class StructMemberExpression : public Expression
{
public:
	StructMemberExpression() : Expression(ExpressionType::StructMember) {}
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	UProperty* Field = nullptr;
	Expression* Value = nullptr;
};

class RotatorToVectorExpression : public Expression
{
public:
	RotatorToVectorExpression() : Expression(ExpressionType::RotatorToVector) {}
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* Value = nullptr;
};

class ByteToIntExpression : public Expression
{
public:
	ByteToIntExpression() : Expression(ExpressionType::ByteToInt) {}
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* Value = nullptr;
};

class ByteToBoolExpression : public Expression
{
public:
	ByteToBoolExpression() : Expression(ExpressionType::ByteToBool) {}
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* Value = nullptr;
};

class ByteToFloatExpression : public Expression
{
public:
	ByteToFloatExpression() : Expression(ExpressionType::ByteToFloat) {}
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* Value = nullptr;
};

class IntToByteExpression : public Expression
{
public:
	IntToByteExpression() : Expression(ExpressionType::IntToByte) {}
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* Value = nullptr;
};

class IntToBoolExpression : public Expression
{
public:
	IntToBoolExpression() : Expression(ExpressionType::IntToBool) {}
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* Value = nullptr;
};

class IntToFloatExpression : public Expression
{
public:
	IntToFloatExpression() : Expression(ExpressionType::IntToFloat) {}
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* Value = nullptr;
};

class BoolToByteExpression : public Expression
{
public:
	BoolToByteExpression() : Expression(ExpressionType::BoolToByte) {}
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* Value = nullptr;
};

class BoolToIntExpression : public Expression
{
public:
	BoolToIntExpression() : Expression(ExpressionType::BoolToInt) {}
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* Value = nullptr;
};

class BoolToFloatExpression : public Expression
{
public:
	BoolToFloatExpression() : Expression(ExpressionType::BoolToFloat) {}
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* Value = nullptr;
};

class FloatToByteExpression : public Expression
{
public:
	FloatToByteExpression() : Expression(ExpressionType::FloatToByte) {}
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* Value = nullptr;
};

class FloatToIntExpression : public Expression
{
public:
	FloatToIntExpression() : Expression(ExpressionType::FloatToInt) {}
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* Value = nullptr;
};

class FloatToBoolExpression : public Expression
{
public:
	FloatToBoolExpression() : Expression(ExpressionType::FloatToBool) {}
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* Value = nullptr;
};

class Unknown0x46Expression : public Expression
{
public:
	Unknown0x46Expression() : Expression(ExpressionType::Unknown0x46) {}
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* Value = nullptr;
};

class ObjectToBoolExpression : public Expression
{
public:
	ObjectToBoolExpression() : Expression(ExpressionType::ObjectToBool) {}
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* Value = nullptr;
};

class NameToBoolExpression : public Expression
{
public:
	NameToBoolExpression() : Expression(ExpressionType::NameToBool) {}
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* Value = nullptr;
};

class StringToByteExpression : public Expression
{
public:
	StringToByteExpression() : Expression(ExpressionType::StringToByte) {}
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* Value = nullptr;
};

class StringToIntExpression : public Expression
{
public:
	StringToIntExpression() : Expression(ExpressionType::StringToInt) {}
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* Value = nullptr;
};

class StringToBoolExpression : public Expression
{
public:
	StringToBoolExpression() : Expression(ExpressionType::StringToBool) {}
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* Value = nullptr;
};

class StringToFloatExpression : public Expression
{
public:
	StringToFloatExpression() : Expression(ExpressionType::StringToFloat) {}
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* Value = nullptr;
};

class StringToVectorExpression : public Expression
{
public:
	StringToVectorExpression() : Expression(ExpressionType::StringToVector) {}
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* Value = nullptr;
};

class StringToRotatorExpression : public Expression
{
public:
	StringToRotatorExpression() : Expression(ExpressionType::StringToRotator) {}
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* Value = nullptr;
};

class VectorToBoolExpression : public Expression
{
public:
	VectorToBoolExpression() : Expression(ExpressionType::VectorToBool) {}
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* Value = nullptr;
};

class VectorToRotatorExpression : public Expression
{
public:
	VectorToRotatorExpression() : Expression(ExpressionType::VectorToRotator) {}
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* Value = nullptr;
};

class RotatorToBoolExpression : public Expression
{
public:
	RotatorToBoolExpression() : Expression(ExpressionType::RotatorToBool) {}
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* Value = nullptr;
};

class ByteToStringExpression : public Expression
{
public:
	ByteToStringExpression() : Expression(ExpressionType::ByteToString) {}
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* Value = nullptr;
};

class IntToStringExpression : public Expression
{
public:
	IntToStringExpression() : Expression(ExpressionType::IntToString) {}
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* Value = nullptr;
};

class BoolToStringExpression : public Expression
{
public:
	BoolToStringExpression() : Expression(ExpressionType::BoolToString) {}
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* Value = nullptr;
};

class FloatToStringExpression : public Expression
{
public:
	FloatToStringExpression() : Expression(ExpressionType::FloatToString) {}
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* Value = nullptr;
};

class ObjectToStringExpression : public Expression
{
public:
	ObjectToStringExpression() : Expression(ExpressionType::ObjectToString) {}
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* Value = nullptr;
};

class NameToStringExpression : public Expression
{
public:
	NameToStringExpression() : Expression(ExpressionType::NameToString) {}
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* Value = nullptr;
};

class VectorToStringExpression : public Expression
{
public:
	VectorToStringExpression() : Expression(ExpressionType::VectorToString) {}
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* Value = nullptr;
};

class RotatorToStringExpression : public Expression
{
public:
	RotatorToStringExpression() : Expression(ExpressionType::RotatorToString) {}
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* Value = nullptr;
};

class StringToNameExpression : public Expression
{
public:
	StringToNameExpression() : Expression(ExpressionType::StringToName) {}
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* Value = nullptr;
};

class DynArrayToIntExpression : public Expression
{
public:
	DynArrayToIntExpression() : Expression(ExpressionType::DynArrayToInt) {}
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Expression* Value = nullptr;
};

class VirtualFunctionExpression : public Expression
{
public:
	VirtualFunctionExpression() : Expression(ExpressionType::VirtualFunction) {}
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	NameString Name;
	Array<Expression*> Args;
};

class FinalFunctionExpression : public Expression
{
public:
	FinalFunctionExpression() : Expression(ExpressionType::FinalFunction) {}
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	UFunction* Func = nullptr;
	Array<Expression*> Args;
};

class GlobalFunctionExpression : public Expression
{
public:
	GlobalFunctionExpression() : Expression(ExpressionType::GlobalFunction) {}
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	NameString Name;
	Array<Expression*> Args;
};

class NativeFunctionExpression : public Expression
{
public:
	NativeFunctionExpression() : Expression(ExpressionType::NativeFunction) {}
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	int nativeindex = 0;
	Array<Expression*> Args;
};

struct FunctionArgInfo
{
	int size = 0;
	int flags = 0; // 1 = out parameter
};

class FunctionArgumentsExpression : public Expression
{
public:
	FunctionArgumentsExpression() : Expression(ExpressionType::FunctionArguments) {}
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	Array<FunctionArgInfo> args;
};

struct ConstructArgument
{
	UProperty* Name = nullptr;
	Expression* Value = nullptr;
};

class ConstructExpression : public Expression
{
public:
	ConstructExpression() : Expression(ExpressionType::Construct) {}
	void Visit(ExpressionVisitor* visitor) override { visitor->Expr(this); }

	UStruct* Struct = nullptr;
	Array<ConstructArgument> Args;
};
