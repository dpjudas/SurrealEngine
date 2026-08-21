
#include "Precomp.h"
#include "ResolveExpressionConstants.h"
#include "SemanticAnalysis.h"
#include "Compiler/Typesystem/TypeSystem.h"
#include "Compiler/Lex/TextUtil.h"
#include <cmath>

void ResolveExpressionConstants::analyze(AstExpression* expression)
{
	expression->visit(this);
}

void ResolveExpressionConstants::expression(AstArrayCreationExpression* node)
{
}

void ResolveExpressionConstants::expression(AstLiteral* node)
{
}

void ResolveExpressionConstants::expression(AstSimpleName* node)
{
}

void ResolveExpressionConstants::expression(AstNamedObject* node)
{
}

void ResolveExpressionConstants::expression(AstMemberAccess* node)
{
	if (node->expression)
		node->expression->visit(this);
}

void ResolveExpressionConstants::expression(AstInvocationExpression* node)
{
	node->expression->visit(this);
}

void ResolveExpressionConstants::expression(AstElementAccess* node)
{
	node->expression->visit(this);
}

void ResolveExpressionConstants::expression(AstBaseAccess* node)
{
}

void ResolveExpressionConstants::expression(AstPostIncrementExpression* node)
{
	node->expression->visit(this);
}

void ResolveExpressionConstants::expression(AstPostDecrementExpression* node)
{
	node->expression->visit(this);
}

void ResolveExpressionConstants::expression(AstNewExpression* node)
{
	for (size_t i = 0; i < node->args.size(); i++)
	{
		node->args[i]->visit(this);
	}
}

void ResolveExpressionConstants::expression(AstTypeofExpression* node)
{
}

void ResolveExpressionConstants::expression(AstParenthesizedExpression* node)
{
	node->expression->visit(this);
	node->result.constval = node->expression->result.constval;
}

void ResolveExpressionConstants::expression(AstSizeofExpression* node)
{
}

void ResolveExpressionConstants::expression(AstAnonymousMethodExpression* node)
{
}

void ResolveExpressionConstants::expression(AstUnaryPlusExpression* node)
{
	node->operand->visit(this);
	node->result.constval = call_plus_overload(node->result.function, node->operand->result);
}

void ResolveExpressionConstants::expression(AstUnaryMinusExpression* node)
{
	node->operand->visit(this);
	node->result.constval = call_minus_overload(node->result.function, node->operand->result);
}

void ResolveExpressionConstants::expression(AstUnaryLogicalNotExpression* node)
{
	node->operand->visit(this);
	node->result.constval = call_logical_not_overload(node->result.function, node->operand->result);
}

void ResolveExpressionConstants::expression(AstUnaryBitwiseComplementExpression* node)
{
	node->operand->visit(this);
	node->result.constval = call_bitwise_complement_overload(node->result.function, node->operand->result);
}

void ResolveExpressionConstants::expression(AstUnaryPreIncrementExpression* node)
{
	node->operand->visit(this);
}

void ResolveExpressionConstants::expression(AstUnaryPreDecrementExpression* node)
{
	node->operand->visit(this);
}

void ResolveExpressionConstants::expression(AstMultiplicationExpression* node)
{
	node->operand1->visit(this);
	node->operand2->visit(this);
	node->result.constval = call_multiplication_overload(node->result.function, node->operand1->result, node->operand2->result);
}

void ResolveExpressionConstants::expression(AstExponentiationExpression* node)
{
}

void ResolveExpressionConstants::expression(AstDivisionExpression* node)
{
	node->operand1->visit(this);
	node->operand2->visit(this);
	node->result.constval = call_division_overload(node->result.function, node->operand1->result, node->operand2->result);
}

void ResolveExpressionConstants::expression(AstRemainderExpression* node)
{
	node->operand1->visit(this);
	node->operand2->visit(this);
	node->result.constval = call_remainder_overload(node->result.function, node->operand1->result, node->operand2->result);
}

void ResolveExpressionConstants::expression(AstAdditionExpression* node)
{
	node->operand1->visit(this);
	node->operand2->visit(this);
	node->result.constval = call_addition_overload(node->result.function, node->operand1->result, node->operand2->result);
}

void ResolveExpressionConstants::expression(AstSubtractionExpression* node)
{
	node->operand1->visit(this);
	node->operand2->visit(this);
	node->result.constval = call_subtraction_overload(node->result.function, node->operand1->result, node->operand2->result);
}

void ResolveExpressionConstants::expression(AstStringConcatExpression* node)
{
}

void ResolveExpressionConstants::expression(AstStringSpaceConcatExpression* node)
{
}

void ResolveExpressionConstants::expression(AstDotProductExpression* node)
{
}

void ResolveExpressionConstants::expression(AstCrossProductExpression* node)
{
}

void ResolveExpressionConstants::expression(AstShiftLeftExpression* node)
{
	node->operand1->visit(this);
	node->operand2->visit(this);
	node->result.constval = call_shift_left_overload(node->result.function, node->operand1->result, node->operand2->result);
}

void ResolveExpressionConstants::expression(AstShiftRightExpression* node)
{
	node->operand1->visit(this);
	node->operand2->visit(this);
	node->result.constval = call_shift_right_overload(node->result.function, node->operand1->result, node->operand2->result);
}

void ResolveExpressionConstants::expression(AstUnsignedShiftRightExpression* node)
{
}

void ResolveExpressionConstants::expression(AstLessExpression* node)
{
	node->operand1->visit(this);
	node->operand2->visit(this);
	node->result.constval = call_less_overload(node->result.function, node->operand1->result, node->operand2->result);
}

void ResolveExpressionConstants::expression(AstGreaterExpression* node)
{
	node->operand1->visit(this);
	node->operand2->visit(this);
	node->result.constval = call_greater_overload(node->result.function, node->operand1->result, node->operand2->result);
}

void ResolveExpressionConstants::expression(AstLessEqualExpression* node)
{
	node->operand1->visit(this);
	node->operand2->visit(this);
	node->result.constval = call_less_equal_overload(node->result.function, node->operand1->result, node->operand2->result);
}

void ResolveExpressionConstants::expression(AstGreaterEqualExpression* node)
{
	node->operand1->visit(this);
	node->operand2->visit(this);
	node->result.constval = call_greater_equal_overload(node->result.function, node->operand1->result, node->operand2->result);
}

void ResolveExpressionConstants::expression(AstIsExpression* node)
{
}

void ResolveExpressionConstants::expression(AstAsExpression* node)
{
}

void ResolveExpressionConstants::expression(AstEqualExpression* node)
{
	node->operand1->visit(this);
	node->operand2->visit(this);
	node->result.constval = call_equal_overload(node->result.function, node->operand1->result, node->operand2->result);
}

void ResolveExpressionConstants::expression(AstNotEqualExpression* node)
{
	node->operand1->visit(this);
	node->operand2->visit(this);
	node->result.constval = call_not_equal_overload(node->result.function, node->operand1->result, node->operand2->result);
}

void ResolveExpressionConstants::expression(AstCaseInsensitiveEqualExpression* node)
{
}

void ResolveExpressionConstants::expression(AstLogicalAndExpression* node)
{
	node->operand1->visit(this);
	node->operand2->visit(this);
	node->result.constval = call_logical_and_overload(node->result.function, node->operand1->result, node->operand2->result);
}

void ResolveExpressionConstants::expression(AstLogicalOrExpression* node)
{
	node->operand1->visit(this);
	node->operand2->visit(this);
	node->result.constval = call_logical_or_overload(node->result.function, node->operand1->result, node->operand2->result);
}

void ResolveExpressionConstants::expression(AstLogicalXorExpression* node)
{
	node->operand1->visit(this);
	node->operand2->visit(this);
	node->result.constval = call_logical_xor_overload(node->result.function, node->operand1->result, node->operand2->result);
}

void ResolveExpressionConstants::expression(AstLogicalXorXorExpression* node)
{
}

void ResolveExpressionConstants::expression(AstConditionalAndExpression* node)
{
	node->operand1->visit(this);
	node->operand2->visit(this);
	node->result.constval = call_conditional_and_overload(node->result.function, node->operand1->result, node->operand2->result);
}

void ResolveExpressionConstants::expression(AstConditionalOrExpression* node)
{
	node->operand1->visit(this);
	node->operand2->visit(this);
	node->result.constval = call_conditional_or_overload(node->result.function, node->operand1->result, node->operand2->result);
}

void ResolveExpressionConstants::expression(AstAssignmentExpression* node)
{
	node->operand1->visit(this);
	node->operand2->visit(this);
}

void ResolveExpressionConstants::expression(AstTrinaryExpression* node)
{
}
