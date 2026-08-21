
#pragma once

#include "Compiler/Typesystem/TypeExpression.h"

class SemanticAnalysis;

class ConstResolver
{
public:
	ConstResolver(SemanticAnalysis& sema) : sema(sema) {}

	ExpressionConstantValue convert(TypeName* srctype, const ExpressionConstantValue& srcvalue, TypeName* dest);
	ExpressionConstantValue convert(const ExpressionResult& src, TypeName* dest);

protected:
	ExpressionConstantValue call_plus_overload(FunctionMember* func, const ExpressionResult& a);
	ExpressionConstantValue call_minus_overload(FunctionMember* func, const ExpressionResult& a);
	ExpressionConstantValue call_logical_not_overload(FunctionMember* func, const ExpressionResult& a);
	ExpressionConstantValue call_bitwise_complement_overload(FunctionMember* func, const ExpressionResult& a);

	ExpressionConstantValue call_addition_overload(FunctionMember* func, const ExpressionResult& a, const ExpressionResult& b);
	ExpressionConstantValue call_subtraction_overload(FunctionMember* func, const ExpressionResult& a, const ExpressionResult& b);
	ExpressionConstantValue call_multiplication_overload(FunctionMember* func, const ExpressionResult& a, const ExpressionResult& b);
	ExpressionConstantValue call_division_overload(FunctionMember* func, const ExpressionResult& a, const ExpressionResult& b);
	ExpressionConstantValue call_remainder_overload(FunctionMember* func, const ExpressionResult& a, const ExpressionResult& b);
	ExpressionConstantValue call_logical_and_overload(FunctionMember* func, const ExpressionResult& a, const ExpressionResult& b);
	ExpressionConstantValue call_logical_or_overload(FunctionMember* func, const ExpressionResult& a, const ExpressionResult& b);
	ExpressionConstantValue call_logical_xor_overload(FunctionMember* func, const ExpressionResult& a, const ExpressionResult& b);
	ExpressionConstantValue call_shift_left_overload(FunctionMember* func, const ExpressionResult& a, const ExpressionResult& b);
	ExpressionConstantValue call_shift_right_overload(FunctionMember* func, const ExpressionResult& a, const ExpressionResult& b);
	ExpressionConstantValue call_less_overload(FunctionMember* func, const ExpressionResult& a, const ExpressionResult& b);
	ExpressionConstantValue call_greater_overload(FunctionMember* func, const ExpressionResult& a, const ExpressionResult& b);
	ExpressionConstantValue call_less_equal_overload(FunctionMember* func, const ExpressionResult& a, const ExpressionResult& b);
	ExpressionConstantValue call_greater_equal_overload(FunctionMember* func, const ExpressionResult& a, const ExpressionResult& b);
	ExpressionConstantValue call_equal_overload(FunctionMember* func, const ExpressionResult& a, const ExpressionResult& b);
	ExpressionConstantValue call_not_equal_overload(FunctionMember* func, const ExpressionResult& a, const ExpressionResult& b);
	ExpressionConstantValue call_conditional_and_overload(FunctionMember* func, const ExpressionResult& a, const ExpressionResult& b);
	ExpressionConstantValue call_conditional_or_overload(FunctionMember* func, const ExpressionResult& a, const ExpressionResult& b);

	SemanticAnalysis& sema;
};
