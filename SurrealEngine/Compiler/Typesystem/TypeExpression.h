
#pragma once

#include "Type.h"
#include "TypeExpressionVisitor.h"
#include "Math/vec.h"
#include "Math/rotator.h"

class ConstantTypeMember;
class EnumValueTypeMember;
class FunctionMember;
class IRBuilder;
class IRValue;
class IRFunction;

class ExpressionConstantValue
{
public:
	bool is_constant = false;
	union
	{
		bool i1;
		int32_t i32;
		uint8_t u8;
		float f32;
	};
	std::string str;
	vec3 vec;
	Rotator rot;
};

enum class ExpressionClass
{
	value,
	variable,
	type,
	indexer,
	method_group,
	nothing
};

class ExpressionResult
{
public:
	ExpressionResult() = default;
	ExpressionResult(TypeName* type, ExpressionClass variant) : type(type), variant(variant) { }
	ExpressionResult(TypeName* type, ExpressionClass variant, FunctionMember* func) : type(type), variant(variant), function(func) { }

	TypeName* type = nullptr;
	ExpressionClass variant = ExpressionClass::value;
	std::vector<FunctionMember*> method_group;

	TypeName* member = nullptr;
	FunctionMember* function = nullptr;

	IRValue* value = nullptr;
	IRFunction* getter = nullptr;
	IRFunction* setter = nullptr;
	IRValue* instance_expression = nullptr;
	std::vector<IRValue*> indexer_args;

	ExpressionConstantValue constval;
};

class TypeConstantExpression
{
public:
	virtual ~TypeConstantExpression() = default;
	virtual void visit(TypeConstantExpressionVisitor* visitor) = 0;

	ExpressionResult result;
};

class TypeLiteral : public TypeConstantExpression
{
public:
	void visit(TypeConstantExpressionVisitor* visitor) override { visitor->expression(this); }

	Type* type = nullptr;
	union
	{
		bool i1;
		int32_t i32;
		uint8_t u8;
		float f32;
	};
	std::string str;
	vec3 vec;
	Rotator rot;
};

class TypeConstant : public TypeConstantExpression
{
public:
	void visit(TypeConstantExpressionVisitor* visitor) override { visitor->expression(this); }

	ConstantTypeMember* constant = nullptr;
	EnumValueTypeMember* enum_value = nullptr;
};

class TypeParenthesizedExpression : public TypeConstantExpression
{
public:
	void visit(TypeConstantExpressionVisitor* visitor) override { visitor->expression(this); }

	TypeConstantExpression* expression = nullptr;
};

class TypeUnaryExpression : public TypeConstantExpression
{
public:
	TypeConstantExpression* operand = nullptr;
	FunctionMember* function = nullptr;
};

class TypeBinaryExpression : public TypeConstantExpression
{
public:
	TypeConstantExpression* operand1 = nullptr;
	TypeConstantExpression* operand2 = nullptr;
	FunctionMember* function = nullptr;
};

class TypeTrinaryExpression : public TypeConstantExpression
{
public:
	TypeConstantExpression* operand1 = nullptr;
	TypeConstantExpression* operand2 = nullptr;
	TypeConstantExpression* operand3 = nullptr;
	FunctionMember* function = nullptr;
};

class TypeUnaryPlusExpression : public TypeUnaryExpression
{
public:
	void visit(TypeConstantExpressionVisitor* visitor) override { visitor->expression(this); }
};

class TypeUnaryMinusExpression : public TypeUnaryExpression
{
public:
	void visit(TypeConstantExpressionVisitor* visitor) override { visitor->expression(this); }
};

class TypeUnaryLogicalNotExpression : public TypeUnaryExpression
{
public:
	void visit(TypeConstantExpressionVisitor* visitor) override { visitor->expression(this); }
};

class TypeUnaryBitwiseComplementExpression : public TypeUnaryExpression
{
public:
	void visit(TypeConstantExpressionVisitor* visitor) override { visitor->expression(this); }
};

class TypeMultiplicationExpression : public TypeBinaryExpression
{
public:
	void visit(TypeConstantExpressionVisitor* visitor) override { visitor->expression(this); }
};

class TypeExponentiationExpression : public TypeBinaryExpression
{
public:
	void visit(TypeConstantExpressionVisitor* visitor) override { visitor->expression(this); }
};

class TypeDivisionExpression : public TypeBinaryExpression
{
public:
	void visit(TypeConstantExpressionVisitor* visitor) override { visitor->expression(this); }
};

class TypeRemainderExpression : public TypeBinaryExpression
{
public:
	void visit(TypeConstantExpressionVisitor* visitor) override { visitor->expression(this); }
};

class TypeAdditionExpression : public TypeBinaryExpression
{
public:
	void visit(TypeConstantExpressionVisitor* visitor) override { visitor->expression(this); }
};

class TypeSubtractionExpression : public TypeBinaryExpression
{
public:
	void visit(TypeConstantExpressionVisitor* visitor) override { visitor->expression(this); }
};

class TypeStringConcatExpression : public TypeBinaryExpression
{
public:
	void visit(TypeConstantExpressionVisitor* visitor) override { visitor->expression(this); }
};

class TypeStringSpaceConcatExpression : public TypeBinaryExpression
{
public:
	void visit(TypeConstantExpressionVisitor* visitor) override { visitor->expression(this); }
};

class TypeDotProductExpression : public TypeBinaryExpression
{
public:
	void visit(TypeConstantExpressionVisitor* visitor) override { visitor->expression(this); }
};

class TypeCrossProductExpression : public TypeBinaryExpression
{
public:
	void visit(TypeConstantExpressionVisitor* visitor) override { visitor->expression(this); }
};

class TypeShiftLeftExpression : public TypeBinaryExpression
{
public:
	void visit(TypeConstantExpressionVisitor* visitor) override { visitor->expression(this); }
};

class TypeShiftRightExpression : public TypeBinaryExpression
{
public:
	void visit(TypeConstantExpressionVisitor* visitor) override { visitor->expression(this); }
};

class TypeUnsignedShiftRightExpression : public TypeBinaryExpression
{
public:
	void visit(TypeConstantExpressionVisitor* visitor) override { visitor->expression(this); }
};

class TypeLessExpression : public TypeBinaryExpression
{
public:
	void visit(TypeConstantExpressionVisitor* visitor) override { visitor->expression(this); }
};

class TypeGreaterExpression : public TypeBinaryExpression
{
public:
	void visit(TypeConstantExpressionVisitor* visitor) override { visitor->expression(this); }
};

class TypeLessEqualExpression : public TypeBinaryExpression
{
public:
	void visit(TypeConstantExpressionVisitor* visitor) override { visitor->expression(this); }
};

class TypeGreaterEqualExpression : public TypeBinaryExpression
{
public:
	void visit(TypeConstantExpressionVisitor* visitor) override { visitor->expression(this); }
};

class TypeEqualExpression : public TypeBinaryExpression
{
public:
	void visit(TypeConstantExpressionVisitor* visitor) override { visitor->expression(this); }
};

class TypeNotEqualExpression : public TypeBinaryExpression
{
public:
	void visit(TypeConstantExpressionVisitor* visitor) override { visitor->expression(this); }
};

class TypeCaseInsensitiveEqualExpression : public TypeBinaryExpression
{
public:
	void visit(TypeConstantExpressionVisitor* visitor) override { visitor->expression(this); }
};

class TypeLogicalAndExpression : public TypeBinaryExpression
{
public:
	void visit(TypeConstantExpressionVisitor* visitor) override { visitor->expression(this); }
};

class TypeLogicalOrExpression : public TypeBinaryExpression
{
public:
	void visit(TypeConstantExpressionVisitor* visitor) override { visitor->expression(this); }
};

class TypeLogicalXorExpression : public TypeBinaryExpression
{
public:
	void visit(TypeConstantExpressionVisitor* visitor) override { visitor->expression(this); }
};

class TypeLogicalXorXorExpression : public TypeBinaryExpression
{
public:
	void visit(TypeConstantExpressionVisitor* visitor) override { visitor->expression(this); }
};

class TypeConditionalAndExpression : public TypeBinaryExpression
{
public:
	void visit(TypeConstantExpressionVisitor* visitor) override { visitor->expression(this); }
};

class TypeConditionalOrExpression : public TypeBinaryExpression
{
public:
	void visit(TypeConstantExpressionVisitor* visitor) override { visitor->expression(this); }
};
