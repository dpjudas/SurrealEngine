
#pragma once

#include <string>
#include <vector>
#include "TypeVisitor.h"

class TypeName
{
public:
	TypeName(TypeName *parent) : parent(parent) { }
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
	Type(TypeName *parent) : TypeName(parent) { }
	Type(TypeName *parent, const std::string &name) : TypeName(parent, name) { }

	virtual void visit(TypeVisitor *visitor) = 0;
};

class TypePackage : public TypeName
{
public:
	TypePackage(TypeName *parent) : TypeName(parent) { }
	TypePackage(TypeName *parent, const std::string &name) : TypeName(parent, name) { }

	void visit(TypeVisitor *visitor)
	{
		for (size_t i = 0; i < packages.size(); i++)
			packages[i]->visit(visitor);

		for (size_t i = 0; i < types.size(); i++)
			types[i]->visit(visitor);
	}

	std::vector<TypePackage *> packages;
	std::vector<Type *> types;
};

class VoidType : public Type
{
public:
	VoidType(TypeName *parent) : Type(parent, "Void") { }

	void visit(TypeVisitor *visitor)
	{
		visitor->type(this);
	}
};

class NativeHandleType : public Type
{
public:
	NativeHandleType(TypeName *parent) : Type(parent, "NativeHandle") { }

	void visit(TypeVisitor *visitor)
	{
		visitor->type(this);
	}
};
