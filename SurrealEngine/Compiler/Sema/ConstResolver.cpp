
#include "Precomp.h"
#include "ConstResolver.h"
#include "SemanticAnalysis.h"
#include "Compiler/Typesystem/TypeSystem.h"
#include "Compiler/Lex/TextUtil.h"
#include <cmath>

ExpressionConstantValue ConstResolver::convert(const ExpressionResult& src, TypeName* dest)
{
	return convert(src.type, src.constval, dest);
}

ExpressionConstantValue ConstResolver::convert(TypeName* srctype, const ExpressionConstantValue& srcvalue, TypeName* dest)
{
	if (!srcvalue.is_constant)
		throw SemaException("Constant expression expected");

	if (srctype == dest) return srcvalue;

	ExpressionConstantValue val = srcvalue;
	auto& ts = sema.type_system();
	int64_t src = 0;

	if (srctype == ts.sbyte_type) { src = srcvalue.i8; }
	else if (srctype == ts.byte_type) { src = (int64_t)srcvalue.u8; }
	else if (srctype == ts.int16_type) { src = srcvalue.i16; }
	else if (srctype == ts.uint16_type) { src = (int64_t)srcvalue.u16; }
	else if (srctype == ts.int32_type) { src = srcvalue.i32; }
	else if (srctype == ts.uint32_type) { src = (int64_t)srcvalue.u32; }
	else if (srctype == ts.int64_type) { src = srcvalue.i64; }
	else if (srctype == ts.uint64_type) { src = (int64_t)srcvalue.u64; }
	else if (srctype == ts.char_type) { src = srcvalue.character; }
	else if (srctype == ts.single_type && dest != ts.double_type) { src = (int64_t)srcvalue.f32; }
	else if (srctype == ts.double_type && dest != ts.single_type) { src = (int64_t)srcvalue.f64; }

	if (dest == ts.sbyte_type) { val.i8 = (int8_t)src; }
	else if (dest == ts.byte_type) { val.u8 = (uint8_t)src; }
	else if (dest == ts.int16_type) { val.i16 = (int16_t)src; }
	else if (dest == ts.uint16_type) { val.u16 = (uint16_t)src; }
	else if (dest == ts.int32_type) { val.i32 = (int32_t)src; }
	else if (dest == ts.uint32_type) { val.u32 = (uint32_t)src; }
	else if (dest == ts.int64_type) { val.i64 = (int64_t)src; }
	else if (dest == ts.uint64_type) { val.u64 = (uint64_t)src; }
	else if (dest == ts.char_type) { val.character = (uint16_t)src; }
	else if (dest == ts.single_type && srctype == ts.double_type) { val.f32 = (float)srcvalue.f64; }
	else if (dest == ts.double_type && srctype == ts.single_type) { val.f64 = (double)srcvalue.f32; }
	else if (dest == ts.single_type) { val.f32 = (float)src; }
	else if (dest == ts.double_type) { val.f64 = (double)src; }

	return val;
}

ExpressionConstantValue ConstResolver::call_plus_overload(FunctionMember* func, const ExpressionResult& a)
{
	if (!a.constval.is_constant) return {};

	return convert(a, func->parameters[0]->type);
}

ExpressionConstantValue ConstResolver::call_minus_overload(FunctionMember* func, const ExpressionResult& a)
{
	if (!a.constval.is_constant) return {};

	auto& ts = sema.type_system();
	ExpressionConstantValue v = convert(a, func->parameters[0]->type);
	ExpressionConstantValue result;
	result.is_constant = true;
	if (func == ts.unary_operator_int32)
		result.i32 = -v.i32;
	else if (func == ts.unary_operator_int64)
		result.i64 = -v.i64;
	else if (func == ts.unary_operator_single)
		result.f32 = -v.f32;
	else if (func == ts.unary_operator_double)
		result.f64 = -v.f64;
	else
		throw SemaException("Unknown operator overload");
	return result;
}

ExpressionConstantValue ConstResolver::call_logical_not_overload(FunctionMember* func, const ExpressionResult& a)
{
	if (!a.constval.is_constant) return {};

	auto& ts = sema.type_system();
	ExpressionConstantValue v = convert(a, func->parameters[0]->type);
	ExpressionConstantValue result;
	result.is_constant = true;
	if (func == ts.unary_operator_boolean)
		result.i1 = !v.i1;
	else
		throw SemaException("Unknown operator overload");
	return result;
}

ExpressionConstantValue ConstResolver::call_bitwise_complement_overload(FunctionMember* func, const ExpressionResult& a)
{
	if (!a.constval.is_constant) return {};

	auto& ts = sema.type_system();
	ExpressionConstantValue v = convert(a, func->parameters[0]->type);
	ExpressionConstantValue result;
	result.is_constant = true;
	if (func == ts.unary_operator_int32)
		result.i32 = ~v.i32;
	else if (func == ts.unary_operator_int64)
		result.i64 = ~v.i64;
	else if (func == ts.unary_operator_uint32)
		result.u32 = ~v.u32;
	else if (func == ts.unary_operator_uint64)
		result.u64 = ~v.u64;
	else
		throw SemaException("Unknown operator overload");
	return result;
}

ExpressionConstantValue ConstResolver::call_addition_overload(FunctionMember* func, const ExpressionResult& a, const ExpressionResult& b)
{
	if (!a.constval.is_constant || !b.constval.is_constant) return {};

	auto& ts = sema.type_system();
	ExpressionConstantValue v1 = convert(a, func->parameters[0]->type);
	ExpressionConstantValue v2 = convert(b, func->parameters[1]->type);
	ExpressionConstantValue result;
	result.is_constant = true;
	if (func == ts.binary_operator_int32)
		result.i32 = v1.i32 + v2.i32;
	else if (func == ts.binary_operator_uint32)
		result.u32 = v1.u32 + v2.u32;
	else if (func == ts.binary_operator_int64)
		result.i64 = v1.i64 + v2.i64;
	else if (func == ts.binary_operator_uint64)
		result.u64 = v1.u64 + v2.u64;
	else if (func == ts.binary_operator_single)
		result.f32 = v1.f32 + v2.f32;
	else if (func == ts.binary_operator_double)
		result.f64 = v1.f64 + v2.f64;
	else if (func == ts.binary_operator_string)
		result.str = v1.str + v2.str;
	else if (func == ts.binary_operator_tostring1 || func == ts.binary_operator_tostring2)
		throw SemaException("ToString not allowed in constant expression");
	else
		throw SemaException("Unknown operator overload");
	return result;
}

ExpressionConstantValue ConstResolver::call_subtraction_overload(FunctionMember* func, const ExpressionResult& a, const ExpressionResult& b)
{
	if (!a.constval.is_constant || !b.constval.is_constant) return {};

	auto& ts = sema.type_system();
	ExpressionConstantValue v1 = convert(a, func->parameters[0]->type);
	ExpressionConstantValue v2 = convert(b, func->parameters[1]->type);
	ExpressionConstantValue result;
	result.is_constant = true;
	if (func == ts.binary_operator_int32)
		result.i32 = v1.i32 - v2.i32;
	else if (func == ts.binary_operator_uint32)
		result.u32 = v1.u32 - v2.u32;
	else if (func == ts.binary_operator_int64)
		result.i64 = v1.i64 - v2.i64;
	else if (func == ts.binary_operator_uint64)
		result.u64 = v1.u64 - v2.u64;
	else if (func == ts.binary_operator_single)
		result.f32 = v1.f32 - v2.f32;
	else if (func == ts.binary_operator_double)
		result.f64 = v1.f64 - v2.f64;
	else
		throw SemaException("Unknown operator overload");
	return result;
}

ExpressionConstantValue ConstResolver::call_multiplication_overload(FunctionMember* func, const ExpressionResult& a, const ExpressionResult& b)
{
	if (!a.constval.is_constant || !b.constval.is_constant) return {};

	auto& ts = sema.type_system();
	ExpressionConstantValue v1 = convert(a, func->parameters[0]->type);
	ExpressionConstantValue v2 = convert(b, func->parameters[1]->type);
	ExpressionConstantValue result;
	result.is_constant = true;
	if (func == ts.binary_operator_int32)
		result.i32 = v1.i32 * v2.i32;
	else if (func == ts.binary_operator_uint32)
		result.u32 = v1.u32 * v2.u32;
	else if (func == ts.binary_operator_int64)
		result.i64 = v1.i64 * v2.i64;
	else if (func == ts.binary_operator_uint64)
		result.u64 = v1.u64 * v2.u64;
	else if (func == ts.binary_operator_single)
		result.f32 = v1.f32 * v2.f32;
	else if (func == ts.binary_operator_double)
		result.f64 = v1.f64 * v2.f64;
	else
		throw SemaException("Unknown operator overload");
	return result;
}

ExpressionConstantValue ConstResolver::call_division_overload(FunctionMember* func, const ExpressionResult& a, const ExpressionResult& b)
{
	if (!a.constval.is_constant || !b.constval.is_constant) return {};

	auto& ts = sema.type_system();
	ExpressionConstantValue v1 = convert(a, func->parameters[0]->type);
	ExpressionConstantValue v2 = convert(b, func->parameters[1]->type);
	ExpressionConstantValue result;
	result.is_constant = true;
	if (func == ts.binary_operator_int32)
		result.i32 = v1.i32 / v2.i32;
	else if (func == ts.binary_operator_uint32)
		result.u32 = v1.u32 / v2.u32;
	else if (func == ts.binary_operator_int64)
		result.i64 = v1.i64 / v2.i64;
	else if (func == ts.binary_operator_uint64)
		result.u64 = v1.u64 / v2.u64;
	else if (func == ts.binary_operator_single)
		result.f32 = v1.f32 / v2.f32;
	else if (func == ts.binary_operator_double)
		result.f64 = v1.f64 / v2.f64;
	else
		throw SemaException("Unknown operator overload");
	return result;
}

ExpressionConstantValue ConstResolver::call_remainder_overload(FunctionMember* func, const ExpressionResult& a, const ExpressionResult& b)
{
	if (!a.constval.is_constant || !b.constval.is_constant) return {};

	auto& ts = sema.type_system();
	ExpressionConstantValue v1 = convert(a, func->parameters[0]->type);
	ExpressionConstantValue v2 = convert(b, func->parameters[1]->type);
	ExpressionConstantValue result;
	result.is_constant = true;
	if (func == ts.binary_operator_int32)
		result.i32 = v1.i32 % v2.i32;
	else if (func == ts.binary_operator_uint32)
		result.u32 = v1.u32 % v2.u32;
	else if (func == ts.binary_operator_int64)
		result.i64 = v1.i64 % v2.i64;
	else if (func == ts.binary_operator_uint64)
		result.u64 = v1.u64 % v2.u64;
	else if (func == ts.binary_operator_single)
		result.f32 = std::fmod(v1.f32, v2.f32);
	else if (func == ts.binary_operator_double)
		result.f64 = std::fmod(v1.f64, v2.f64);
	else
		throw SemaException("Unknown operator overload");
	return result;
}

ExpressionConstantValue ConstResolver::call_logical_and_overload(FunctionMember* func, const ExpressionResult& a, const ExpressionResult& b)
{
	if (!a.constval.is_constant || !b.constval.is_constant) return {};

	ExpressionConstantValue v1 = convert(a, func->parameters[0]->type);
	ExpressionConstantValue v2 = convert(b, func->parameters[1]->type);
	ExpressionConstantValue result;
	result.is_constant = true;
	auto& ts = sema.type_system();
	if (func == ts.compare_operator_int32)
		result.i32 = v1.i32 & v2.i32;
	else if (func == ts.compare_operator_int64)
		result.i64 = v1.i64 & v2.i64;
	else if (func == ts.compare_operator_uint32)
		result.u32 = v1.u32 & v2.u32;
	else if (func == ts.compare_operator_uint64)
		result.u64 = v1.u64 & v2.u64;
	else if (func == ts.compare_operator_boolean)
		result.i1 = v1.i1 & v2.i1;
	else
		throw SemaException("Unknown operator overload");
	return result;
}

ExpressionConstantValue ConstResolver::call_logical_or_overload(FunctionMember* func, const ExpressionResult& a, const ExpressionResult& b)
{
	if (!a.constval.is_constant || !b.constval.is_constant) return {};

	ExpressionConstantValue v1 = convert(a, func->parameters[0]->type);
	ExpressionConstantValue v2 = convert(b, func->parameters[1]->type);
	ExpressionConstantValue result;
	result.is_constant = true;
	auto& ts = sema.type_system();
	if (func == ts.compare_operator_int32)
		result.i32 = v1.i32 | v2.i32;
	else if (func == ts.compare_operator_int64)
		result.i64 = v1.i64 | v2.i64;
	else if (func == ts.compare_operator_uint32)
		result.u32 = v1.u32 | v2.u32;
	else if (func == ts.compare_operator_uint64)
		result.u64 = v1.u64 | v2.u64;
	else if (func == ts.compare_operator_boolean)
		result.i1 = v1.i1 | v2.i1;
	else
		throw SemaException("Unknown operator overload");
	return result;
}

ExpressionConstantValue ConstResolver::call_logical_xor_overload(FunctionMember* func, const ExpressionResult& a, const ExpressionResult& b)
{
	if (!a.constval.is_constant || !b.constval.is_constant) return {};

	ExpressionConstantValue v1 = convert(a, func->parameters[0]->type);
	ExpressionConstantValue v2 = convert(b, func->parameters[1]->type);
	ExpressionConstantValue result;
	result.is_constant = true;
	auto& ts = sema.type_system();
	if (func == ts.compare_operator_int32)
		result.i32 = v1.i32 ^ v2.i32;
	else if (func == ts.compare_operator_int64)
		result.i64 = v1.i64 ^ v2.i64;
	else if (func == ts.compare_operator_uint32)
		result.u32 = v1.u32 ^ v2.u32;
	else if (func == ts.compare_operator_uint64)
		result.u64 = v1.u64 ^ v2.u64;
	else if (func == ts.compare_operator_boolean)
		result.i1 = v1.i1 ^ v2.i1;
	else
		throw SemaException("Unknown operator overload");
	return result;
}

ExpressionConstantValue ConstResolver::call_shift_left_overload(FunctionMember* func, const ExpressionResult& a, const ExpressionResult& b)
{
	if (!a.constval.is_constant || !b.constval.is_constant) return {};

	ExpressionConstantValue v1 = convert(a, func->parameters[0]->type);
	ExpressionConstantValue v2 = convert(b, func->parameters[1]->type);
	ExpressionConstantValue result;
	result.is_constant = true;
	auto& ts = sema.type_system();
	if (func == ts.binary_operator_int32)
		result.i32 = v1.i32 << v2.i32;
	else if (func == ts.binary_operator_uint32_int32)
		result.i64 = v1.i64 << v2.i32;
	else if (func == ts.binary_operator_int64_int32)
		result.u32 = v1.u32 << v2.i32;
	else if (func == ts.binary_operator_uint64_int32)
		result.u64 = v1.u64 << v2.i32;
	else
		throw SemaException("Unknown operator overload");
	return result;
}

ExpressionConstantValue ConstResolver::call_shift_right_overload(FunctionMember* func, const ExpressionResult& a, const ExpressionResult& b)
{
	if (!a.constval.is_constant || !b.constval.is_constant) return {};

	ExpressionConstantValue v1 = convert(a, func->parameters[0]->type);
	ExpressionConstantValue v2 = convert(b, func->parameters[1]->type);
	ExpressionConstantValue result;
	result.is_constant = true;
	auto& ts = sema.type_system();
	if (func == ts.binary_operator_int32)
		result.i32 = v1.i32 >> v2.i32;
	else if (func == ts.binary_operator_uint32_int32)
		result.i64 = v1.i64 >> v2.i32;
	else if (func == ts.binary_operator_int64_int32)
		result.u32 = v1.u32 >> v2.i32;
	else if (func == ts.binary_operator_uint64_int32)
		result.u64 = v1.u64 >> v2.i32;
	else
		throw SemaException("Unknown operator overload");
	return result;
}

ExpressionConstantValue ConstResolver::call_less_overload(FunctionMember* func, const ExpressionResult& a, const ExpressionResult& b)
{
	if (!a.constval.is_constant || !b.constval.is_constant) return {};

	ExpressionConstantValue v1 = convert(a, func->parameters[0]->type);
	ExpressionConstantValue v2 = convert(b, func->parameters[1]->type);
	ExpressionConstantValue result;
	result.is_constant = true;
	auto& ts = sema.type_system();
	if (func == ts.compare_operator_int32)
		result.i1 = v1.i32 < v2.i32;
	else if (func == ts.compare_operator_int64)
		result.i1 = v1.i64 < v2.i64;
	else if (func == ts.compare_operator_uint32)
		result.i1 = v1.u32 < v2.u32;
	else if (func == ts.compare_operator_uint64)
		result.i1 = v1.u64 < v2.u64;
	else if (func == ts.compare_operator_single)
		result.i1 = v1.f32 < v2.f32;
	else if (func == ts.compare_operator_double)
		result.i1 = v1.f64 < v2.f64;
	else
		throw SemaException("Unknown operator overload");
	return result;
}

ExpressionConstantValue ConstResolver::call_greater_overload(FunctionMember* func, const ExpressionResult& a, const ExpressionResult& b)
{
	if (!a.constval.is_constant || !b.constval.is_constant) return {};

	ExpressionConstantValue v1 = convert(a, func->parameters[0]->type);
	ExpressionConstantValue v2 = convert(b, func->parameters[1]->type);
	ExpressionConstantValue result;
	result.is_constant = true;
	auto& ts = sema.type_system();
	if (func == ts.compare_operator_int32)
		result.i1 = v1.i32 > v2.i32;
	else if (func == ts.compare_operator_int64)
		result.i1 = v1.i64 > v2.i64;
	else if (func == ts.compare_operator_uint32)
		result.i1 = v1.u32 > v2.u32;
	else if (func == ts.compare_operator_uint64)
		result.i1 = v1.u64 > v2.u64;
	else if (func == ts.compare_operator_single)
		result.i1 = v1.f32 > v2.f32;
	else if (func == ts.compare_operator_double)
		result.i1 = v1.f64 > v2.f64;
	else
		throw SemaException("Unknown operator overload");
	return result;
}

ExpressionConstantValue ConstResolver::call_less_equal_overload(FunctionMember* func, const ExpressionResult& a, const ExpressionResult& b)
{
	if (!a.constval.is_constant || !b.constval.is_constant) return {};

	ExpressionConstantValue v1 = convert(a, func->parameters[0]->type);
	ExpressionConstantValue v2 = convert(b, func->parameters[1]->type);
	ExpressionConstantValue result;
	result.is_constant = true;
	auto& ts = sema.type_system();
	if (func == ts.compare_operator_int32)
		result.i1 = v1.i32 <= v2.i32;
	else if (func == ts.compare_operator_int64)
		result.i1 = v1.i64 <= v2.i64;
	else if (func == ts.compare_operator_uint32)
		result.i1 = v1.u32 <= v2.u32;
	else if (func == ts.compare_operator_uint64)
		result.i1 = v1.u64 <= v2.u64;
	else if (func == ts.compare_operator_single)
		result.i1 = v1.f32 <= v2.f32;
	else if (func == ts.compare_operator_double)
		result.i1 = v1.f64 <= v2.f64;
	else
		throw SemaException("Unknown operator overload");
	return result;
}

ExpressionConstantValue ConstResolver::call_greater_equal_overload(FunctionMember* func, const ExpressionResult& a, const ExpressionResult& b)
{
	if (!a.constval.is_constant || !b.constval.is_constant) return {};

	ExpressionConstantValue v1 = convert(a, func->parameters[0]->type);
	ExpressionConstantValue v2 = convert(b, func->parameters[1]->type);
	ExpressionConstantValue result;
	result.is_constant = true;
	auto& ts = sema.type_system();
	if (func == ts.compare_operator_int32)
		result.i1 = v1.i32 >= v2.i32;
	else if (func == ts.compare_operator_int64)
		result.i1 = v1.i64 >= v2.i64;
	else if (func == ts.compare_operator_uint32)
		result.i1 = v1.u32 >= v2.u32;
	else if (func == ts.compare_operator_uint64)
		result.i1 = v1.u64 >= v2.u64;
	else if (func == ts.compare_operator_single)
		result.i1 = v1.f32 >= v2.f32;
	else if (func == ts.compare_operator_double)
		result.i1 = v1.f64 >= v2.f64;
	else
		throw SemaException("Unknown operator overload");
	return result;
}

ExpressionConstantValue ConstResolver::call_equal_overload(FunctionMember* func, const ExpressionResult& a, const ExpressionResult& b)
{
	if (!a.constval.is_constant || !b.constval.is_constant) return {};

	ExpressionConstantValue v1 = convert(a, func->parameters[0]->type);
	ExpressionConstantValue v2 = convert(b, func->parameters[1]->type);
	ExpressionConstantValue result;
	result.is_constant = true;
	auto& ts = sema.type_system();
	if (func == ts.compare_operator_int32)
		result.i1 = v1.i32 == v2.i32;
	else if (func == ts.compare_operator_int64)
		result.i1 = v1.i64 == v2.i64;
	else if (func == ts.compare_operator_uint32)
		result.i1 = v1.u32 == v2.u32;
	else if (func == ts.compare_operator_uint64)
		result.i1 = v1.u64 == v2.u64;
	else if (func == ts.compare_operator_single)
		result.i1 = v1.f32 == v2.f32;
	else if (func == ts.compare_operator_double)
		result.i1 = v1.f64 == v2.f64;
	else if (func == ts.compare_operator_string)
		result.i1 = v1.str == v2.str;
	else
		throw SemaException("Unknown operator overload");
	return result;
}

ExpressionConstantValue ConstResolver::call_not_equal_overload(FunctionMember* func, const ExpressionResult& a, const ExpressionResult& b)
{
	if (!a.constval.is_constant || !b.constval.is_constant) return {};

	ExpressionConstantValue v1 = convert(a, func->parameters[0]->type);
	ExpressionConstantValue v2 = convert(b, func->parameters[1]->type);
	ExpressionConstantValue result;
	result.is_constant = true;
	auto& ts = sema.type_system();
	if (func == ts.compare_operator_int32)
		result.i1 = v1.i32 != v2.i32;
	else if (func == ts.compare_operator_int64)
		result.i1 = v1.i64 != v2.i64;
	else if (func == ts.compare_operator_uint32)
		result.i1 = v1.u32 != v2.u32;
	else if (func == ts.compare_operator_uint64)
		result.i1 = v1.u64 != v2.u64;
	else if (func == ts.compare_operator_single)
		result.i1 = v1.f32 != v2.f32;
	else if (func == ts.compare_operator_double)
		result.i1 = v1.f64 != v2.f64;
	else if (func == ts.compare_operator_string)
		result.i1 = v1.str != v2.str;
	else
		throw SemaException("Unknown operator overload");
	return result;
}

ExpressionConstantValue ConstResolver::call_conditional_and_overload(FunctionMember* func, const ExpressionResult& a, const ExpressionResult& b)
{
	if (!a.constval.is_constant || !b.constval.is_constant) return {};

	ExpressionConstantValue v1 = convert(a, func->parameters[0]->type);
	ExpressionConstantValue v2 = convert(b, func->parameters[1]->type);
	ExpressionConstantValue result;
	result.is_constant = true;
	auto& ts = sema.type_system();
	if (func == ts.binary_operator_boolean)
		result.i1 = v1.i1 && v2.i1;
	else
		throw SemaException("Unknown operator overload");
	return result;
}

ExpressionConstantValue ConstResolver::call_conditional_or_overload(FunctionMember* func, const ExpressionResult& a, const ExpressionResult& b)
{
	if (!a.constval.is_constant || !b.constval.is_constant) return {};

	ExpressionConstantValue v1 = convert(a, func->parameters[0]->type);
	ExpressionConstantValue v2 = convert(b, func->parameters[1]->type);
	ExpressionConstantValue result;
	result.is_constant = true;
	auto& ts = sema.type_system();
	if (func == ts.binary_operator_boolean)
		result.i1 = v1.i1 || v2.i1;
	else
		throw SemaException("Unknown operator overload");
	return result;
}
