
#pragma once

#include <map>
#include "AstTypeDeclaration.h"
#include "AstExpression.h"

class AstExpression;
class ConstantTypeMember;

class AstConstantDeclarator : public AstNode
{
public:
	std::string identifier;
	AstExpression *expression = nullptr;
	ConstantTypeMember *sema_type = nullptr;
};

class AstConstantDeclaration : public AstNode
{
public:
	void visit(AstNameVisitor *visitor)
	{
		visitor->name(this);
	}

	AstName *type = nullptr;
	std::vector<AstConstantDeclarator *> declarators;
};

class AstExpression;
class AstArrayInitializer;
class FieldTypeMember;

class AstVariableDeclarator : public AstNode
{
public:
	std::string identifier;
	int array_dimension = 0;
	AstExpression *expression = nullptr;
	AstArrayInitializer *array_initializer = nullptr;
};

class AstFieldDeclaration : public AstNode
{
public:
	void visit(AstNameVisitor *visitor)
	{
		visitor->name(this);
	}

	std::string group;
	bool is_static = false;
	bool is_const = false;
	bool is_localized = false;
	bool is_native = false;
	bool is_travel = false;
	AstAccessType access_type = {};
	AstName *type = nullptr;
	std::vector<AstVariableDeclarator *> declarators;

	FieldTypeMember *sema_type = nullptr;
};

class AstStatement;
class AstBlockStatement;
class AstMethodFixedParameter;

class AstMethodParameter : public AstNode
{
};

class AstMethodFixedParameter : public AstMethodParameter
{
public:
	bool is_ref = false;
	bool is_out = false;
	bool is_optional = false;
	bool is_coerce = false;
	AstName *type = nullptr;
	std::string identifier;
	int array_dimension = 0;
};

class NameScope
{
public:
	std::map<std::string, ExpressionResult> variables;
};

class MethodTypeMember;

class AstMethodDeclaration : public AstNode
{
public:
	void visit(AstNameVisitor *visitor)
	{
		visitor->name(this);
	}

	bool is_static = false;
	bool is_event = false;
	bool is_native = false;
	bool is_singular = false;
	bool is_simulated = false;
	bool is_final = false;
	int native_index = 0;
	AstAccessType access_type = {};
	AstName *return_type = nullptr;
	std::string identifier;
	std::vector<AstMethodParameter *> parameters;
	AstBlockStatement *block = nullptr;

	MethodTypeMember *sema_type = nullptr;
	NameScope name_scope;
};

class OperatorTypeMember;

class AstOperatorDeclaration : public AstNode
{
public:
	void visit(AstNameVisitor *visitor)
	{
		visitor->name(this);
	}

	bool is_public = false;
	bool is_static = false;
	bool is_extern = false;
	std::string operator_type;
	std::string conversion_type;
	AstMethodFixedParameter *param1 = nullptr;
	AstMethodFixedParameter *param2 = nullptr;

	AstBlockStatement *block = nullptr;

	OperatorTypeMember *sema_type = nullptr;
	NameScope name_scope;
};
