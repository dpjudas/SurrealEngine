
#pragma once

#include "AstNode.h"
#include "AstName.h"
#include "AstAccessType.h"
#include <string>
#include <vector>

class Type;
class AstTypeParameterList;

class AstNameDeclaration : public AstNode
{
public:
	Type *type = nullptr;
};

class AstClassDeclaration : public AstNameDeclaration
{
public:
	void visit(AstNameVisitor *visitor)
	{
		visitor->name(this);
	}

	void visit_children(AstNameVisitor *visitor)
	{
		for (size_t i = 0; i < members.size(); i++)
		{
			members[i]->visit(visitor);
		}
	}

	bool is_abstract = false;
	bool is_native = false;
	bool no_export = false;

	std::string identifier;
	AstIdentifierName* base_type = nullptr;

	// allowed members types:
	// AstConstantDeclaration, AstFieldDeclaration, AstMethodDeclaration, AstOperatorDeclaration
	// AstNameDeclaration, AstStructDeclaration
	std::vector<AstNode *> members;
};

class AstStructDeclaration : public AstNameDeclaration
{
public:
	void visit(AstNameVisitor *visitor)
	{
		visitor->name(this);
	}

	void visit_children(AstNameVisitor *visitor)
	{
		for (size_t i = 0; i < members.size(); i++)
		{
			members[i]->visit(visitor);
		}
	}

	AstAccessType access_type = {};
	std::string identifier;
	std::vector<AstNode *> members;
};

class AstExpression;
class EnumValueTypeMember;

class AstEnumValueDeclaration : public AstNode
{
public:
	void visit(AstNameVisitor *visitor)
	{
		visitor->name(this);
	}

	std::string identifier;
	AstExpression *expression = nullptr;
	EnumValueTypeMember *sema_type = nullptr;
};

class AstEnumDeclaration : public AstNameDeclaration
{
public:
	void visit(AstNameVisitor *visitor)
	{
		visitor->name(this);
	}

	void visit_children(AstNameVisitor *visitor)
	{
		for (size_t i = 0; i < members.size(); i++)
		{
			members[i]->visit(visitor);
		}
	}

	std::string identifier;
	std::vector<AstEnumValueDeclaration *> members;
};
