
#pragma once

#include <memory>
#include <set>
#include "Type.h"
#include "ValueType.h"
#include "ReferenceType.h"
#include "TypeMember.h"
#include "TypeExpression.h"

class AstName;
class AstKeywordType;
class AstIdentifierName;
class AstNameDeclaration;
class FunctionMember;
class ExpressionResult;

class TypeSystem
{
public:
	TypeSystem();
	~TypeSystem();

	std::vector<Type*> types;

	NullType *null_type = nullptr;
	PointerType* pointer_type = nullptr;
	VoidType *void_type = nullptr;
	ByteType *byte_type = nullptr;
	IntType *int_type = nullptr;
	SingleType *single_type = nullptr;
	BooleanType *boolean_type = nullptr;
	StringType* string_type = nullptr;

	ClassType *object = nullptr;

	FunctionMember *unary_operator_byte = nullptr;
	FunctionMember *unary_operator_int = nullptr;
	FunctionMember *unary_operator_single = nullptr;
	FunctionMember *unary_operator_boolean = nullptr;

	FunctionMember *binary_operator_int = nullptr;
	FunctionMember *binary_operator_single = nullptr;
	FunctionMember *binary_operator_boolean = nullptr;
	FunctionMember *binary_operator_string = nullptr;

	FunctionMember *compare_operator_int = nullptr;
	FunctionMember *compare_operator_single = nullptr;
	FunctionMember *compare_operator_boolean = nullptr;
	FunctionMember *compare_operator_string = nullptr;

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

	std::vector<std::unique_ptr<TypeName>> allocatedTypes;
	std::vector<std::unique_ptr<TypeConstantExpression>> allocatedExpressions;
	std::vector<std::unique_ptr<MethodFixedParameter>> allocatedFixedParameters;

private:
	int compare_conversion(const ExpressionResult &src, MethodFixedParameter *t1, MethodFixedParameter *t2);
};

class TypeScope
{
public:
	TypeScope(TypeSystem& ts) : ts(ts) {}

	void push_scope(AstNameDeclaration* ast_name_declaration) {}
	void pop_scope() {}

	Type* lookup_type(AstName* name) { return nullptr; }
	Type* lookup_keyword(AstKeywordType* type) { return nullptr; }

	std::vector<TypeName*> scopes;

private:
	TypeSystem& ts;
};

class MemberLookup
{
public:
	MemberLookup(TypeSystem& type_system) : type_system(type_system) {}

	void lookup(TypeName* type, const std::string& name) {}

	std::set<TypeName*> members;

private:
	TypeSystem& type_system;
};
