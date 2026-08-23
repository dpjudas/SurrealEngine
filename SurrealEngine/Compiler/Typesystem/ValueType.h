
#pragma once

#include "Type.h"
#include "TypeMember.h"

class ValueType : public Type
{
public:
	ValueType(TypeName *parent, const std::string &name) : Type(parent, name) { }
};

/////////////////////////////////////////////////////////////////////////////
// Simple Type:

class ByteType : public ValueType
{
public:
	ByteType(TypeName *parent) : ValueType(parent, "byte") { }

	void visit(TypeVisitor *visitor)
	{
		visitor->type(this);
	}
};

class IntType : public ValueType
{
public:
	IntType(TypeName *parent) : ValueType(parent, "int") { }

	void visit(TypeVisitor *visitor)
	{
		visitor->type(this);
	}
};

class SingleType : public ValueType
{
public:
	SingleType(TypeName *parent) : ValueType(parent, "float") { }

	void visit(TypeVisitor *visitor)
	{
		visitor->type(this);
	}
};

class BooleanType : public ValueType
{
public:
	BooleanType(TypeName *parent) : ValueType(parent, "bool") { }

	void visit(TypeVisitor *visitor)
	{
		visitor->type(this);
	}
};

class NameType : public ValueType
{
public:
	NameType(TypeName* parent) : ValueType(parent, "name") {}

	void visit(TypeVisitor* visitor)
	{
		visitor->type(this);
	}
};

class StringType : public ValueType
{
public:
	StringType(TypeName* parent) : ValueType(parent, "string") {}

	void visit(TypeVisitor* visitor)
	{
		visitor->type(this);
	}
};

/////////////////////////////////////////////////////////////////////////////

class StructType : public ValueType
{
public:
	StructType(TypeName *parent, const std::string &name) : ValueType(parent, name) { }

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
	}

	std::vector<Type *> subtypes;

	StructType* base = nullptr;

	std::vector<ConstantTypeMember *> constants;
	std::vector<FieldTypeMember *> fields;
	std::vector<MethodTypeMember *> methods;
};

class EnumType : public ValueType
{
public:
	EnumType(TypeName *parent, const std::string &name) : ValueType(parent, name) { }

	std::vector<EnumValueTypeMember *> values;

	void visit(TypeVisitor *visitor)
	{
		visitor->type(this);
	}

	void visit_members(TypeMemberVisitor *visitor)
	{
		for (size_t i = 0; i < values.size(); i++)
			values[i]->visit(visitor);
	}
};
