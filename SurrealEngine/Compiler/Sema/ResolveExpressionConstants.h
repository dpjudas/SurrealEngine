
#pragma once

#include "Compiler/Ast/Ast.h"
#include "ConstResolver.h"

class SemanticAnalysis;

class ResolveExpressionConstants : public ConstResolver, AstExpressionVisitor
{
public:
	ResolveExpressionConstants(SemanticAnalysis& sema) : ConstResolver(sema) {}
	void analyze(AstExpression* expression);

private:
	void expression(AstArrayCreationExpression* node) override;
	void expression(AstLiteral* node) override;
	void expression(AstSimpleName* node) override;
	void expression(AstNamedObject* node) override;
	void expression(AstInvocationExpression* node) override;
	void expression(AstElementAccess* node) override;
	void expression(AstBaseAccess* node) override;
	void expression(AstMemberAccess* node) override;
	void expression(AstPostIncrementExpression* node) override;
	void expression(AstPostDecrementExpression* node) override;
	void expression(AstNewExpression* node) override;
	void expression(AstTypeofExpression* node) override;
	void expression(AstParenthesizedExpression* node) override;
	void expression(AstSizeofExpression* node) override;
	void expression(AstAnonymousMethodExpression* node) override;
	void expression(AstUnaryPlusExpression* node) override;
	void expression(AstUnaryMinusExpression* node) override;
	void expression(AstUnaryLogicalNotExpression* node) override;
	void expression(AstUnaryBitwiseComplementExpression* node) override;
	void expression(AstUnaryPreIncrementExpression* node) override;
	void expression(AstUnaryPreDecrementExpression* node) override;
	void expression(AstMultiplicationExpression* node) override;
	void expression(AstExponentiationExpression* node) override;
	void expression(AstDivisionExpression* node) override;
	void expression(AstRemainderExpression* node) override;
	void expression(AstAdditionExpression* node) override;
	void expression(AstSubtractionExpression* node) override;
	void expression(AstStringConcatExpression* node) override;
	void expression(AstStringSpaceConcatExpression* node) override;
	void expression(AstDotProductExpression* node) override;
	void expression(AstCrossProductExpression* node) override;
	void expression(AstShiftLeftExpression* node) override;
	void expression(AstShiftRightExpression* node) override;
	void expression(AstUnsignedShiftRightExpression* node) override;
	void expression(AstLessExpression* node) override;
	void expression(AstGreaterExpression* node) override;
	void expression(AstLessEqualExpression* node) override;
	void expression(AstGreaterEqualExpression* node) override;
	void expression(AstIsExpression* node) override;
	void expression(AstAsExpression* node) override;
	void expression(AstEqualExpression* node) override;
	void expression(AstNotEqualExpression* node) override;
	void expression(AstCaseInsensitiveEqualExpression* node) override;
	void expression(AstLogicalAndExpression* node) override;
	void expression(AstLogicalOrExpression* node) override;
	void expression(AstLogicalXorExpression* node) override;
	void expression(AstLogicalXorXorExpression* node) override;
	void expression(AstConditionalAndExpression* node) override;
	void expression(AstConditionalOrExpression* node) override;
	void expression(AstAssignmentExpression* node) override;
	void expression(AstTrinaryExpression* node) override;
};
