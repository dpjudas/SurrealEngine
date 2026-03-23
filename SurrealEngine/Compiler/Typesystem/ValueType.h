
#pragma once

#include "Type.h"
#include "TypeMember.h"

class ValueType : public Type
{
public:
	ValueType(TypeName *parent) : Type(parent) { }
	ValueType(TypeName *parent, const std::string &name) : Type(parent, name) { }
};

/////////////////////////////////////////////////////////////////////////////
// Simple Type:

class SByteType : public ValueType
{
public:
	SByteType(TypeName *parent) : ValueType(parent, "SByte") { }

	void visit(TypeVisitor *visitor)
	{
		visitor->type(this);
	}
};

class ByteType : public ValueType
{
public:
	ByteType(TypeName *parent) : ValueType(parent, "Byte") { }

	void visit(TypeVisitor *visitor)
	{
		visitor->type(this);
	}
};

class Int16Type : public ValueType
{
public:
	Int16Type(TypeName *parent) : ValueType(parent, "Int16") { }

	void visit(TypeVisitor *visitor)
	{
		visitor->type(this);
	}
};

class UInt16Type : public ValueType
{
public:
	UInt16Type(TypeName *parent) : ValueType(parent, "UInt16") { }

	void visit(TypeVisitor *visitor)
	{
		visitor->type(this);
	}
};

class Int32Type : public ValueType
{
public:
	Int32Type(TypeName *parent) : ValueType(parent, "Int32") { }

	void visit(TypeVisitor *visitor)
	{
		visitor->type(this);
	}
};

class UInt32Type : public ValueType
{
public:
	UInt32Type(TypeName *parent) : ValueType(parent, "UInt32") { }

	void visit(TypeVisitor *visitor)
	{
		visitor->type(this);
	}
};

class Int64Type : public ValueType
{
public:
	Int64Type(TypeName *parent) : ValueType(parent, "Int64") { }

	void visit(TypeVisitor *visitor)
	{
		visitor->type(this);
	}
};

class UInt64Type : public ValueType
{
public:
	UInt64Type(TypeName *parent) : ValueType(parent, "UInt64") { }

	void visit(TypeVisitor *visitor)
	{
		visitor->type(this);
	}
};

class CharType : public ValueType
{
public:
	CharType(TypeName *parent) : ValueType(parent, "Char") { }

	void visit(TypeVisitor *visitor)
	{
		visitor->type(this);
	}
};

class SingleType : public ValueType
{
public:
	SingleType(TypeName *parent) : ValueType(parent, "Single") { }

	void visit(TypeVisitor *visitor)
	{
		visitor->type(this);
	}
};

class DoubleType : public ValueType
{
public:
	DoubleType(TypeName *parent) : ValueType(parent, "Double") { }

	void visit(TypeVisitor *visitor)
	{
		visitor->type(this);
	}
};

class BooleanType : public ValueType
{
public:
	BooleanType(TypeName *parent) : ValueType(parent, "Boolean") { }

	void visit(TypeVisitor *visitor)
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
		for (size_t i = 0; i < operators.size(); i++)
			operators[i]->visit(visitor);
	}

	std::vector<Type *> subtypes;

	StructType* base = nullptr;

	std::vector<ConstantTypeMember *> constants;
	std::vector<FieldTypeMember *> fields;
	std::vector<MethodTypeMember *> methods;
	std::vector<OperatorTypeMember *> operators;
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
