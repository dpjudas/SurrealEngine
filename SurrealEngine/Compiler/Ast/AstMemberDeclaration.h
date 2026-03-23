
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

	AstAccessType access_type = {};
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

	bool is_static = false;
	bool is_const = false;
	AstAccessType access_type = {};
	AstName *type = nullptr;
	std::vector<AstVariableDeclarator *> declarators;

	FieldTypeMember *sema_type = nullptr;
};

class AstStatement;
class AstBlockStatement;
class AstMethodFixedParameter;
class AstMethodParameterArray;

class AstMethodParameter : public AstNode
{
};

class AstMethodFixedParameter : public AstMethodParameter
{
public:
	bool is_ref = false;
	bool is_out = false;
	AstName *type = nullptr;
	std::string identifier;
};

class AstMethodParameterArray : public AstMethodParameter
{
public:
	AstName *array_type = nullptr;
	std::string identifier;
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
