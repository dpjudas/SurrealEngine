
#include "Precomp.h"
#include "CreateTypeConstExpression.h"
#include "SemanticAnalysis.h"
#include "Compiler/Typesystem/TypeSystem.h"
#include "Compiler/Lex/TextUtil.h"
#include <cmath>

CreateTypeConstExpression::CreateTypeConstExpression(SemanticAnalysis& sema) : sema(sema)
{
}

TypeConstantExpression* CreateTypeConstExpression::exec(AstExpression* expression)
{
	expression->visit(this);
	return constexp;
}

void CreateTypeConstExpression::expression(AstArrayCreationExpression* node)
{
}

void CreateTypeConstExpression::expression(AstLiteral* node)
{
	auto result = sema.type_system().newExpression<TypeLiteral>();
	result->type = dynamic_cast<Type*>(node->result.type);
	result->i32 = node->result.constval.i32;
	result->str = node->result.constval.str;
	constexp = result;
}

void CreateTypeConstExpression::expression(AstSimpleName* node)
{
	if (dynamic_cast<ConstantTypeMember*>(node->result.member))
	{
		auto result = sema.type_system().newExpression<TypeConstant>();
		result->constant = static_cast<ConstantTypeMember*>(node->result.member);
		constexp = result;
	}
	else if (dynamic_cast<EnumValueTypeMember*>(node->result.member))
	{
		auto result = sema.type_system().newExpression<TypeConstant>();
		result->enum_value = static_cast<EnumValueTypeMember*>(node->result.member);
		constexp = result;
	}
	else
	{
		throw SemaException("Name reference in constant expressions must be either a constant or an enum value", node);
	}
}

void CreateTypeConstExpression::expression(AstNamedObject* node)
{
	//
}

void CreateTypeConstExpression::expression(AstMemberAccess* node)
{
	if (dynamic_cast<ConstantTypeMember*>(node->result.member))
	{
		auto result = sema.type_system().newExpression<TypeConstant>();
		result->constant = static_cast<ConstantTypeMember*>(node->result.member);
		constexp = result;
	}
	else if (dynamic_cast<EnumValueTypeMember*>(node->result.member))
	{
		auto result = sema.type_system().newExpression<TypeConstant>();
		result->enum_value = static_cast<EnumValueTypeMember*>(node->result.member);
		constexp = result;
	}
	else
	{
		throw SemaException("Member reference in constant expressions must be either a constant or an enum value", node);
	}
}

void CreateTypeConstExpression::expression(AstInvocationExpression* node)
{
	node->expression->visit(this);
}

void CreateTypeConstExpression::expression(AstElementAccess* node)
{
	node->expression->visit(this);
}

void CreateTypeConstExpression::expression(AstBaseAccess* node)
{
	throw SemaException("base keyword not allowed in constant expressions", node);
}

void CreateTypeConstExpression::expression(AstPostIncrementExpression* node)
{
	node->expression->visit(this);
}

void CreateTypeConstExpression::expression(AstPostDecrementExpression* node)
{
	node->expression->visit(this);
}

void CreateTypeConstExpression::expression(AstNewExpression* node)
{
	for (size_t i = 0; i < node->args.size(); i++)
	{
		node->args[i]->visit(this);
	}
}

void CreateTypeConstExpression::expression(AstTypeofExpression* node)
{
	throw SemaException("typeof keyword not allowed in constant expressions", node);
}

void CreateTypeConstExpression::expression(AstParenthesizedExpression* node)
{
	auto result = sema.type_system().newExpression<TypeParenthesizedExpression>();
	node->expression->visit(this);
	result->expression = constexp;
	constexp = result;
}

void CreateTypeConstExpression::expression(AstSizeofExpression* node)
{
	throw SemaException("unchecked keyword not supported", node);
}

void CreateTypeConstExpression::expression(AstAnonymousMethodExpression* node)
{
	throw SemaException("anonymous method not allowed in constant expressions", node);
}

void CreateTypeConstExpression::expression(AstUnaryPlusExpression* node)
{
	constexp = createUnary<TypeUnaryPlusExpression>(node);
}

void CreateTypeConstExpression::expression(AstUnaryMinusExpression* node)
{
	constexp = createUnary<TypeUnaryMinusExpression>(node);
}

void CreateTypeConstExpression::expression(AstUnaryLogicalNotExpression* node)
{
	constexp = createUnary<TypeUnaryLogicalNotExpression>(node);
}

void CreateTypeConstExpression::expression(AstUnaryBitwiseComplementExpression* node)
{
	constexp = createUnary<TypeUnaryBitwiseComplementExpression>(node);
}

void CreateTypeConstExpression::expression(AstUnaryPreIncrementExpression* node)
{
	constexp = createUnary<TypeUnaryBitwiseComplementExpression>(node);
}

void CreateTypeConstExpression::expression(AstUnaryPreDecrementExpression* node)
{
	constexp = createUnary<TypeUnaryBitwiseComplementExpression>(node);
}

void CreateTypeConstExpression::expression(AstMultiplicationExpression* node)
{
	constexp = createBinary<TypeMultiplicationExpression>(node);
}

void CreateTypeConstExpression::expression(AstExponentiationExpression* node)
{
	constexp = createBinary<TypeExponentiationExpression>(node);
}

void CreateTypeConstExpression::expression(AstDivisionExpression* node)
{
	constexp = createBinary<TypeDivisionExpression>(node);
}

void CreateTypeConstExpression::expression(AstRemainderExpression* node)
{
	constexp = createBinary<TypeRemainderExpression>(node);
}

void CreateTypeConstExpression::expression(AstAdditionExpression* node)
{
	constexp = createBinary<TypeAdditionExpression>(node);
}

void CreateTypeConstExpression::expression(AstSubtractionExpression* node)
{
	constexp = createBinary<TypeSubtractionExpression>(node);
}

void CreateTypeConstExpression::expression(AstStringConcatExpression* node)
{
	constexp = createBinary<TypeStringConcatExpression>(node);
}

void CreateTypeConstExpression::expression(AstStringSpaceConcatExpression* node)
{
	constexp = createBinary<TypeStringSpaceConcatExpression>(node);
}

void CreateTypeConstExpression::expression(AstDotProductExpression* node)
{
	constexp = createBinary<TypeDotProductExpression>(node);
}

void CreateTypeConstExpression::expression(AstCrossProductExpression* node)
{
	constexp = createBinary<TypeCrossProductExpression>(node);
}

void CreateTypeConstExpression::expression(AstShiftLeftExpression* node)
{
	constexp = createBinary<TypeShiftLeftExpression>(node);
}

void CreateTypeConstExpression::expression(AstShiftRightExpression* node)
{
	constexp = createBinary<TypeShiftRightExpression>(node);
}

void CreateTypeConstExpression::expression(AstUnsignedShiftRightExpression* node)
{
	constexp = createBinary<TypeUnsignedShiftRightExpression>(node);
}

void CreateTypeConstExpression::expression(AstLessExpression* node)
{
	constexp = createBinary<TypeLessExpression>(node);
}

void CreateTypeConstExpression::expression(AstGreaterExpression* node)
{
	constexp = createBinary<TypeGreaterExpression>(node);
}

void CreateTypeConstExpression::expression(AstLessEqualExpression* node)
{
	constexp = createBinary<TypeLessEqualExpression>(node);
}

void CreateTypeConstExpression::expression(AstGreaterEqualExpression* node)
{
	constexp = createBinary<TypeGreaterEqualExpression>(node);
}

void CreateTypeConstExpression::expression(AstIsExpression* node)
{
	throw SemaException("is keyword method not allowed in constant expressions", node);
}

void CreateTypeConstExpression::expression(AstAsExpression* node)
{
	throw SemaException("as keyword method not allowed in constant expressions", node);
}

void CreateTypeConstExpression::expression(AstEqualExpression* node)
{
	constexp = createBinary<TypeEqualExpression>(node);
}

void CreateTypeConstExpression::expression(AstNotEqualExpression* node)
{
	constexp = createBinary<TypeNotEqualExpression>(node);
}

void CreateTypeConstExpression::expression(AstCaseInsensitiveEqualExpression* node)
{
	constexp = createBinary<TypeCaseInsensitiveEqualExpression>(node);
}

void CreateTypeConstExpression::expression(AstLogicalAndExpression* node)
{
	constexp = createBinary<TypeLogicalAndExpression>(node);
}

void CreateTypeConstExpression::expression(AstLogicalOrExpression* node)
{
	constexp = createBinary<TypeLogicalOrExpression>(node);
}

void CreateTypeConstExpression::expression(AstLogicalXorExpression* node)
{
	constexp = createBinary<TypeLogicalXorExpression>(node);
}

void CreateTypeConstExpression::expression(AstLogicalXorXorExpression* node)
{
	constexp = createBinary<TypeLogicalXorXorExpression>(node);
}

void CreateTypeConstExpression::expression(AstConditionalAndExpression* node)
{
	constexp = createBinary<TypeConditionalAndExpression>(node);
}

void CreateTypeConstExpression::expression(AstConditionalOrExpression* node)
{
	constexp = createBinary<TypeConditionalOrExpression>(node);
}

void CreateTypeConstExpression::expression(AstAssignmentExpression* node)
{
	throw SemaException("assignments not allowed in constant expressions", node);
}

void CreateTypeConstExpression::expression(AstTrinaryExpression* node)
{
}

template<typename T, typename NodeType>
T* CreateTypeConstExpression::createUnary(NodeType* node)
{
	auto result = sema.type_system().newExpression<T>();
	node->operand->visit(this);
	result->operand = constexp;
	result->function = node->result.function;
	return result;
}

template<typename T, typename NodeType>
T* CreateTypeConstExpression::createBinary(NodeType* node)
{
	auto result = sema.type_system().newExpression<T>();
	node->operand1->visit(this);
	result->operand1 = constexp;
	node->operand2->visit(this);
	result->operand2 = constexp;
	result->function = node->result.function;
	return result;
}
