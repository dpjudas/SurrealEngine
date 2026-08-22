
#pragma once

#include <string>
#include <vector>
#include "TypeVisitor.h"

class TypeName
{
public:
	TypeName(TypeName *parent, const std::string &name) : parent(parent), name(name) { }
	virtual ~TypeName() = default;

	virtual std::string get_qualified_name()
	{
		if (parent)
		{
			std::string parent_name = parent->get_qualified_name();
			if (!parent_name.empty())
				return parent_name + "." + name;
			else
				return name;
		}
		else
		{
			return name;
		}
	}

	TypeName *parent;
	std::string name;
};

class Type : public TypeName
{
public:
	Type(TypeName *parent, const std::string &name) : TypeName(parent, name) { }

	virtual void visit(TypeVisitor *visitor) = 0;
};

class VoidType : public Type
{
public:
	VoidType(TypeName *parent) : Type(parent, "void") { }

	void visit(TypeVisitor *visitor)
	{
		visitor->type(this);
	}
};

class PointerType : public Type
{
public:
	PointerType(TypeName *parent) : Type(parent, "pointer") { }

	void visit(TypeVisitor *visitor)
	{
		visitor->type(this);
	}
};
