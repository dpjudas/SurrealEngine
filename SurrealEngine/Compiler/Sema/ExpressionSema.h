
#pragma once

#include "Compiler/Ast/Ast.h"

class SemanticAnalysis;
class TypeScope;
class IndexerTypeMember;
class ConstructorTypeMember;
class ClassType;
class StructType;

class ExpressionSema : AstExpressionVisitor
{
public:
	ExpressionSema(SemanticAnalysis& sema, NameScope& name_scope, TypeScope& type_scope);
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

	FunctionMember* get_increment_overload(const ExpressionResult& a);
	FunctionMember* get_decrement_overload(const ExpressionResult& a);
	FunctionMember* get_plus_overload(const ExpressionResult& a);
	FunctionMember* get_minus_overload(const ExpressionResult& a);
	FunctionMember* get_logical_not_overload(const ExpressionResult& a);
	FunctionMember* get_bitwise_complement_overload(const ExpressionResult& a);

	FunctionMember* get_addition_overload(const ExpressionResult& a, const ExpressionResult& b);
	FunctionMember* get_subtraction_overload(const ExpressionResult& a, const ExpressionResult& b);
	FunctionMember* get_multiplication_overload(const ExpressionResult& a, const ExpressionResult& b);
	FunctionMember* get_division_overload(const ExpressionResult& a, const ExpressionResult& b);
	FunctionMember* get_remainder_overload(const ExpressionResult& a, const ExpressionResult& b);
	FunctionMember* get_logical_and_overload(const ExpressionResult& a, const ExpressionResult& b);
	FunctionMember* get_logical_or_overload(const ExpressionResult& a, const ExpressionResult& b);
	FunctionMember* get_logical_xor_overload(const ExpressionResult& a, const ExpressionResult& b);
	FunctionMember* get_shift_left_overload(const ExpressionResult& a, const ExpressionResult& b);
	FunctionMember* get_shift_right_overload(const ExpressionResult& a, const ExpressionResult& b);
	FunctionMember* get_less_overload(const ExpressionResult& a, const ExpressionResult& b);
	FunctionMember* get_greater_overload(const ExpressionResult& a, const ExpressionResult& b);
	FunctionMember* get_less_equal_overload(const ExpressionResult& a, const ExpressionResult& b);
	FunctionMember* get_greater_equal_overload(const ExpressionResult& a, const ExpressionResult& b);
	FunctionMember* get_equal_overload(const ExpressionResult& a, const ExpressionResult& b);
	FunctionMember* get_not_equal_overload(const ExpressionResult& a, const ExpressionResult& b);
	FunctionMember* get_conditional_and_overload(const ExpressionResult& a, const ExpressionResult& b);
	FunctionMember* get_conditional_or_overload(const ExpressionResult& a, const ExpressionResult& b);

	SemanticAnalysis& sema;
	NameScope& name_scope;
	TypeScope& type_scope;
};
