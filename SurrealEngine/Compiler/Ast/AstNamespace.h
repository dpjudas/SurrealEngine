
#pragma once

#include <vector>
#include "AstNode.h"
#include "AstName.h"

class AstUsingDirective;
class TypeNamespace;

class AstNamespace : public AstNode
{
public:
	AstNamespace() : name(), sema_namespace() { }

	void visit(AstNameVisitor *visitor)
	{
		visitor->name(this);
	}

	void visit_children(AstNameVisitor *visitor)
	{
		for (size_t i = 0; i < namespace_members.size(); i++)
		{
			namespace_members[i]->visit(visitor);
		}
	}

	AstIdentifierName *name;

	// AstExternAlias, AstUsingDirective, AstNamespace, AstNameDeclaration or AstAttributeSections
	std::vector<AstNode *> namespace_members;

	TypeNamespace *sema_namespace;
};
