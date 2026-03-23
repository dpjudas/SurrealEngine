
#pragma once

class TypeLiteral;
class TypeConstant;
class TypeParenthesizedExpression;
class TypeUnaryPlusExpression;
class TypeUnaryMinusExpression;
class TypeUnaryLogicalNotExpression;
class TypeUnaryBitwiseComplementExpression;
class TypeMultiplicationExpression;
class TypeDivisionExpression;
class TypeRemainderExpression;
class TypeAdditionExpression;
class TypeSubtractionExpression;
class TypeShiftLeftExpression;
class TypeShiftRightExpression;
class TypeLessExpression;
class TypeGreaterExpression;
class TypeLessEqualExpression;
class TypeGreaterEqualExpression;
class TypeEqualExpression;
class TypeNotEqualExpression;
class TypeLogicalAndExpression;
class TypeLogicalOrExpression;
class TypeLogicalXorExpression;
class TypeConditionalAndExpression;
class TypeConditionalOrExpression;
class TypeTrinaryExpression;

class TypeConstantExpressionVisitor
{
public:
	virtual void expression(TypeLiteral *node) = 0;
	virtual void expression(TypeConstant *node) = 0;
	virtual void expression(TypeParenthesizedExpression *node) = 0;
	virtual void expression(TypeUnaryPlusExpression *node) = 0;
	virtual void expression(TypeUnaryMinusExpression *node) = 0;
	virtual void expression(TypeUnaryLogicalNotExpression *node) = 0;
	virtual void expression(TypeUnaryBitwiseComplementExpression *node) = 0;
	virtual void expression(TypeMultiplicationExpression *node) = 0;
	virtual void expression(TypeDivisionExpression *node) = 0;
	virtual void expression(TypeRemainderExpression *node) = 0;
	virtual void expression(TypeAdditionExpression *node) = 0;
	virtual void expression(TypeSubtractionExpression *node) = 0;
	virtual void expression(TypeShiftLeftExpression *node) = 0;
	virtual void expression(TypeShiftRightExpression *node) = 0;
	virtual void expression(TypeLessExpression *node) = 0;
	virtual void expression(TypeGreaterExpression *node) = 0;
	virtual void expression(TypeLessEqualExpression *node) = 0;
	virtual void expression(TypeGreaterEqualExpression *node) = 0;
	virtual void expression(TypeEqualExpression *node) = 0;
	virtual void expression(TypeNotEqualExpression *node) = 0;
	virtual void expression(TypeLogicalAndExpression *node) = 0;
	virtual void expression(TypeLogicalOrExpression *node) = 0;
	virtual void expression(TypeLogicalXorExpression *node) = 0;
	virtual void expression(TypeConditionalAndExpression *node) = 0;
	virtual void expression(TypeConditionalOrExpression *node) = 0;
	virtual void expression(TypeTrinaryExpression *node) = 0;
};
