
#pragma once

#include "Compiler/Typesystem/TypeExpressionVisitor.h"
#include "ResolveExpressionConstants.h"

class SemanticAnalysis;

class ResolveTypeConstExpression : public ConstResolver, TypeConstantExpressionVisitor
{
public:
	ResolveTypeConstExpression(SemanticAnalysis& sema) : ConstResolver(sema) {}
	void analyze(TypeConstantExpression* expression);

private:
	void expression(TypeLiteral* node);
	void expression(TypeConstant* node);
	void expression(TypeParenthesizedExpression* node);
	void expression(TypeUnaryPlusExpression* node);
	void expression(TypeUnaryMinusExpression* node);
	void expression(TypeUnaryLogicalNotExpression* node);
	void expression(TypeUnaryBitwiseComplementExpression* node);
	void expression(TypeMultiplicationExpression* node);
	void expression(TypeDivisionExpression* node);
	void expression(TypeRemainderExpression* node);
	void expression(TypeAdditionExpression* node);
	void expression(TypeSubtractionExpression* node);
	void expression(TypeShiftLeftExpression* node);
	void expression(TypeShiftRightExpression* node);
	void expression(TypeLessExpression* node);
	void expression(TypeGreaterExpression* node);
	void expression(TypeLessEqualExpression* node);
	void expression(TypeGreaterEqualExpression* node);
	void expression(TypeEqualExpression* node);
	void expression(TypeNotEqualExpression* node);
	void expression(TypeLogicalAndExpression* node);
	void expression(TypeLogicalOrExpression* node);
	void expression(TypeLogicalXorExpression* node);
	void expression(TypeConditionalAndExpression* node);
	void expression(TypeConditionalOrExpression* node);
	void expression(TypeTrinaryExpression* node);
};
