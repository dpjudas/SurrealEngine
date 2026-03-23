
#pragma once

#include <memory>
#include "Type.h"
#include "ValueType.h"
#include "ReferenceType.h"
#include "TypeMember.h"
#include "TypeExpression.h"

class AstName;
class AstIdentifierName;
class FunctionMember;
class ExpressionResult;

class TypeSystem
{
public:
	TypeSystem();
	~TypeSystem();

	TypePackage* root_package = nullptr;
	TypePackage *core_package = nullptr;

	NativeHandleType *native_handle_type = nullptr;

	NullType *null_type = nullptr;

	VoidType *void_type = nullptr;
	SByteType *sbyte_type = nullptr;
	ByteType *byte_type = nullptr;
	Int16Type *int16_type = nullptr;
	UInt16Type *uint16_type = nullptr;
	Int32Type *int32_type = nullptr;
	UInt32Type *uint32_type = nullptr;
	Int64Type *int64_type = nullptr;
	UInt64Type *uint64_type = nullptr;
	CharType *char_type = nullptr;
	SingleType *single_type = nullptr;
	DoubleType *double_type = nullptr;
	BooleanType *boolean_type = nullptr;

	ClassType *core_object = nullptr;
	ClassType *core_string = nullptr;
	ClassType *core_value_type = nullptr;
	ClassType *core_enum = nullptr;

	FunctionMember *unary_operator_sbyte = nullptr;
	FunctionMember *unary_operator_byte = nullptr;
	FunctionMember *unary_operator_int16 = nullptr;
	FunctionMember *unary_operator_uint16 = nullptr;
	FunctionMember *unary_operator_int32 = nullptr;
	FunctionMember *unary_operator_uint32 = nullptr;
	FunctionMember *unary_operator_int64 = nullptr;
	FunctionMember *unary_operator_uint64 = nullptr;
	FunctionMember *unary_operator_char = nullptr;
	FunctionMember *unary_operator_single = nullptr;
	FunctionMember *unary_operator_double = nullptr;
	FunctionMember *unary_operator_boolean = nullptr;
	FunctionMember *unary_operator_error = nullptr; // generates compile time error

	FunctionMember *binary_operator_int32 = nullptr;
	FunctionMember *binary_operator_uint32 = nullptr;
	FunctionMember *binary_operator_uint32_int32 = nullptr;
	FunctionMember *binary_operator_int64 = nullptr;
	FunctionMember *binary_operator_int64_int32 = nullptr;
	FunctionMember *binary_operator_uint64 = nullptr;
	FunctionMember *binary_operator_uint64_int32 = nullptr;
	FunctionMember *binary_operator_single = nullptr;
	FunctionMember *binary_operator_double = nullptr;
	FunctionMember *binary_operator_boolean = nullptr;
	FunctionMember *binary_operator_string = nullptr;
	FunctionMember *binary_operator_tostring1 = nullptr; // Calls ToString on the object
	FunctionMember *binary_operator_tostring2 = nullptr; // Calls ToString on the object
	FunctionMember *binary_operator_error1 = nullptr; // generates compile time error
	FunctionMember *binary_operator_error2 = nullptr; // generates compile time error

	FunctionMember *compare_operator_int32 = nullptr;
	FunctionMember *compare_operator_uint32 = nullptr;
	FunctionMember *compare_operator_int64 = nullptr;
	FunctionMember *compare_operator_uint64 = nullptr;
	FunctionMember *compare_operator_single = nullptr;
	FunctionMember *compare_operator_double = nullptr;
	FunctionMember *compare_operator_decimal = nullptr;
	FunctionMember *compare_operator_boolean = nullptr;
	FunctionMember *compare_operator_string = nullptr;
	FunctionMember *compare_operator_error1 = nullptr; // generates compile time error
	FunctionMember *compare_operator_error2 = nullptr; // generates compile time error

	FunctionMember *find_best_function(const std::vector<FunctionMember*> &candidates, const std::vector<ExpressionResult> &args);

	bool implicit_convert_allowed(TypeName *src, TypeName *dest);
	bool explicit_convert_allowed(TypeName *src, TypeName *dest);

	template<typename T, typename... Types>
	T* newType(Types&&... args)
	{
		allocatedTypes.push_back(std::make_unique<T>(std::forward<Types>(args)...));
		T* type = static_cast<T*>(allocatedTypes.back().get());
		return type;
	}

	template<typename T, typename... Types>
	T* newExpression(Types&&... args)
	{
		allocatedExpressions.push_back(std::make_unique<T>(std::forward<Types>(args)...));
		T* expression = static_cast<T*>(allocatedExpressions.back().get());
		return expression;
	}

	template<typename T, typename... Types>
	T* newFixedParameter(Types&&... args)
	{
		allocatedFixedParameters.push_back(std::make_unique<T>(std::forward<Types>(args)...));
		T* parameter = static_cast<T*>(allocatedFixedParameters.back().get());
		return parameter;
	}

	template<typename T, typename... Types>
	T* newParameterArray(Types&&... args)
	{
		allocatedParameterArrays.push_back(std::make_unique<T>(std::forward<Types>(args)...));
		T* arr = static_cast<T*>(allocatedParameterArrays.back().get());
		return arr;
	}

	std::vector<std::unique_ptr<TypeName>> allocatedTypes;
	std::vector<std::unique_ptr<TypeConstantExpression>> allocatedExpressions;
	std::vector<std::unique_ptr<MethodFixedParameter>> allocatedFixedParameters;
	std::vector<std::unique_ptr<MethodParameterArray>> allocatedParameterArrays;

private:
	int compare_conversion(const ExpressionResult &src, MethodFixedParameter *t1, MethodFixedParameter *t2);
};
