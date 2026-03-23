
#pragma once

#include "Type.h"
#include "ValueType.h"

class TypeMember;

class ReferenceType : public Type
{
public:
	ReferenceType(TypeName *parent, const std::string &name) : Type(parent, name) { }
};

class ClassType : public ReferenceType
{
public:
	ClassType(TypeName *parent, const std::string &name, bool is_abstract) : ReferenceType(parent, name), is_abstract(is_abstract) { }

	void visit(TypeVisitor *visitor)
	{
		visitor->type(this);
	}

	void visit_members(TypeMemberVisitor *visitor)
	{
		for (size_t i = 0; i < constants.size(); i++)
			constants[i]->visit(visitor);
		for (size_t i = 0; i < fields.size(); i++)
			fields[i]->visit(visitor);
		for (size_t i = 0; i < methods.size(); i++)
			methods[i]->visit(visitor);
		for (size_t i = 0; i < operators.size(); i++)
			operators[i]->visit(visitor);
	}

	std::vector<Type *> subtypes;

	bool is_abstract = false;

	ClassType *base = nullptr;

	std::vector<ConstantTypeMember *> constants;
	std::vector<FieldTypeMember *> fields;
	std::vector<MethodTypeMember *> methods;
	std::vector<OperatorTypeMember *> operators;
};

class ArrayType : public ReferenceType
{
public:
	ArrayType(TypeName *parent) : ReferenceType(parent, std::string()) { }

	void visit(TypeVisitor *visitor)
	{
		visitor->type(this);
	}

	Type *type = nullptr;
	std::vector<std::vector<int> > rank_specifiers;
};

class NullType : public ReferenceType
{
public:
	NullType(TypeName *parent) : ReferenceType(parent, std::string()) { }

	void visit(TypeVisitor *visitor)
	{
		visitor->type(this);
	}
};

class BoxedType : public ReferenceType
{
public:
	BoxedType(TypeName *parent) : ReferenceType(parent, std::string()), type() { }

	void visit(TypeVisitor *visitor)
	{
		visitor->type(this);
	}

	ValueType *type;
};
