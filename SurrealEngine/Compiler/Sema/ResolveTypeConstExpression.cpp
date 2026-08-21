
#include "Precomp.h"
#include "ResolveTypeConstExpression.h"
#include "SemanticAnalysis.h"
#include "Compiler/Typesystem/TypeSystem.h"
#include "Compiler/Typesystem/TypeExpression.h"
#include "Compiler/Lex/TextUtil.h"
#include <cmath>

void ResolveTypeConstExpression::analyze(TypeConstantExpression* expression)
{
	expression->visit(this);
}

void ResolveTypeConstExpression::expression(TypeLiteral* node)
{
	node->result.type = dynamic_cast<Type*>(node->type);
	node->result.constval.u64 = node->u64;
	node->result.constval.str = node->str;
	node->result.constval.is_constant = true;
}

void ResolveTypeConstExpression::expression(TypeConstant* node)
{
	// To do: resolve the referenced constant
}

void ResolveTypeConstExpression::expression(TypeParenthesizedExpression* node)
{
	node->expression->visit(this);
	node->result.constval = node->expression->result.constval;
}

void ResolveTypeConstExpression::expression(TypeUnaryPlusExpression* node)
{
	node->operand->visit(this);
	node->result.constval = call_plus_overload(node->result.function, node->operand->result);
}

void ResolveTypeConstExpression::expression(TypeUnaryMinusExpression* node)
{
	node->operand->visit(this);
	node->result.constval = call_minus_overload(node->result.function, node->operand->result);
}

void ResolveTypeConstExpression::expression(TypeUnaryLogicalNotExpression* node)
{
	node->operand->visit(this);
	node->result.constval = call_logical_not_overload(node->result.function, node->operand->result);
}

void ResolveTypeConstExpression::expression(TypeUnaryBitwiseComplementExpression* node)
{
	node->operand->visit(this);
	node->result.constval = call_bitwise_complement_overload(node->result.function, node->operand->result);
}

void ResolveTypeConstExpression::expression(TypeMultiplicationExpression* node)
{
	node->operand1->visit(this);
	node->operand2->visit(this);
	node->result.constval = call_multiplication_overload(node->result.function, node->operand1->result, node->operand2->result);
}

void ResolveTypeConstExpression::expression(TypeDivisionExpression* node)
{
	node->operand1->visit(this);
	node->operand2->visit(this);
	node->result.constval = call_division_overload(node->result.function, node->operand1->result, node->operand2->result);
}

void ResolveTypeConstExpression::expression(TypeRemainderExpression* node)
{
	node->operand1->visit(this);
	node->operand2->visit(this);
	node->result.constval = call_remainder_overload(node->result.function, node->operand1->result, node->operand2->result);
}

void ResolveTypeConstExpression::expression(TypeAdditionExpression* node)
{
	node->operand1->visit(this);
	node->operand2->visit(this);
	node->result.constval = call_addition_overload(node->result.function, node->operand1->result, node->operand2->result);
}

void ResolveTypeConstExpression::expression(TypeSubtractionExpression* node)
{
	node->operand1->visit(this);
	node->operand2->visit(this);
	node->result.constval = call_subtraction_overload(node->result.function, node->operand1->result, node->operand2->result);
}

void ResolveTypeConstExpression::expression(TypeShiftLeftExpression* node)
{
	node->operand1->visit(this);
	node->operand2->visit(this);
	node->result.constval = call_shift_left_overload(node->result.function, node->operand1->result, node->operand2->result);
}

void ResolveTypeConstExpression::expression(TypeShiftRightExpression* node)
{
	node->operand1->visit(this);
	node->operand2->visit(this);
	node->result.constval = call_shift_right_overload(node->result.function, node->operand1->result, node->operand2->result);
}

void ResolveTypeConstExpression::expression(TypeLessExpression* node)
{
	node->operand1->visit(this);
	node->operand2->visit(this);
	node->result.constval = call_less_overload(node->result.function, node->operand1->result, node->operand2->result);
}

void ResolveTypeConstExpression::expression(TypeGreaterExpression* node)
{
	node->operand1->visit(this);
	node->operand2->visit(this);
	node->result.constval = call_greater_overload(node->result.function, node->operand1->result, node->operand2->result);
}

void ResolveTypeConstExpression::expression(TypeLessEqualExpression* node)
{
	node->operand1->visit(this);
	node->operand2->visit(this);
	node->result.constval = call_less_equal_overload(node->result.function, node->operand1->result, node->operand2->result);
}

void ResolveTypeConstExpression::expression(TypeGreaterEqualExpression* node)
{
	node->operand1->visit(this);
	node->operand2->visit(this);
	node->result.constval = call_greater_equal_overload(node->result.function, node->operand1->result, node->operand2->result);
}

void ResolveTypeConstExpression::expression(TypeEqualExpression* node)
{
	node->operand1->visit(this);
	node->operand2->visit(this);
	node->result.constval = call_equal_overload(node->result.function, node->operand1->result, node->operand2->result);
}

void ResolveTypeConstExpression::expression(TypeNotEqualExpression* node)
{
	node->operand1->visit(this);
	node->operand2->visit(this);
	node->result.constval = call_not_equal_overload(node->result.function, node->operand1->result, node->operand2->result);
}

void ResolveTypeConstExpression::expression(TypeLogicalAndExpression* node)
{
	node->operand1->visit(this);
	node->operand2->visit(this);
	node->result.constval = call_logical_and_overload(node->result.function, node->operand1->result, node->operand2->result);
}

void ResolveTypeConstExpression::expression(TypeLogicalOrExpression* node)
{
	node->operand1->visit(this);
	node->operand2->visit(this);
	node->result.constval = call_logical_or_overload(node->result.function, node->operand1->result, node->operand2->result);
}

void ResolveTypeConstExpression::expression(TypeLogicalXorExpression* node)
{
	node->operand1->visit(this);
	node->operand2->visit(this);
	node->result.constval = call_logical_xor_overload(node->result.function, node->operand1->result, node->operand2->result);
}

void ResolveTypeConstExpression::expression(TypeConditionalAndExpression* node)
{
	node->operand1->visit(this);
	node->operand2->visit(this);
	node->result.constval = call_conditional_and_overload(node->result.function, node->operand1->result, node->operand2->result);
}

void ResolveTypeConstExpression::expression(TypeConditionalOrExpression* node)
{
	node->operand1->visit(this);
	node->operand2->visit(this);
	node->result.constval = call_conditional_or_overload(node->result.function, node->operand1->result, node->operand2->result);
}

void ResolveTypeConstExpression::expression(TypeTrinaryExpression* node)
{
}
