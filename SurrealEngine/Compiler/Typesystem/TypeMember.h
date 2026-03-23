
#pragma once

#include "Type.h"
#include "TypeMemberVisitor.h"

class TypeConstantExpression;

class TypeMember : public TypeName
{
public:
	TypeMember(TypeName *parent, const std::string &name) : TypeName(parent, name) { }

	virtual void visit(TypeMemberVisitor *visitor) = 0;
};

class EnumValueTypeMember : public TypeMember
{
public:
	EnumValueTypeMember(TypeName* parent, const std::string& name) : TypeMember(parent, name) { }

	void visit(TypeMemberVisitor* visitor)
	{
		visitor->member(this);
	}

	TypeConstantExpression *expression = nullptr;
};

class ConstantTypeMember : public TypeMember
{
public:
	ConstantTypeMember(TypeName *parent, const std::string &name, Type *type) : TypeMember(parent, name), type(type) { }

	void visit(TypeMemberVisitor *visitor)
	{
		visitor->member(this);
	}

	Type *type = nullptr;
	TypeConstantExpression *expression = nullptr;
};

class FieldTypeMember : public TypeMember
{
public:
	FieldTypeMember(TypeName *parent, const std::string &name, Type *type) : TypeMember(parent, name), type(type) { }

	void visit(TypeMemberVisitor *visitor)
	{
		visitor->member(this);
	}

	Type *type;
};

class MethodFixedParameter
{
public:
	virtual ~MethodFixedParameter() = default;

	bool is_ref = false;
	bool is_out = false;
	Type *type = nullptr;
	std::string name;
};

class MethodParameterArray
{
public:
	virtual ~MethodParameterArray() = default;

	Type *array_type = nullptr;
	std::string name;
};

class MethodFixedParameter;
class MethodParameterArray;
class TypeSystem;

class FunctionMember : public TypeMember
{
public:
	FunctionMember(TypeName *parent, const std::string &name, Type *return_type) : TypeMember(parent, name), type(return_type) { }
	FunctionMember(TypeSystem *type_system, Type *return_type, std::initializer_list<Type*> args);

	void visit(TypeMemberVisitor *visitor) { }

	Type *type = nullptr;
	std::vector<MethodFixedParameter *> parameters;
	MethodParameterArray *parameter_array = nullptr;
};

class MethodTypeMember : public FunctionMember
{
public:
	MethodTypeMember(TypeName *parent, const std::string &name, bool is_static, bool is_abstract, bool is_virtual, bool is_override, Type *return_type)
		: FunctionMember(parent, name, return_type), is_static(is_static), is_abstract(is_abstract), is_virtual(is_virtual), is_override(is_override) { }

	std::string get_mangled_name()
	{
		std::string mangled_name = "method." + get_qualified_name();
		if (!parameters.empty())
		{
			for (size_t i = 0; i < parameters.size(); i++)
			{
				if (parameters[i]->is_ref || parameters[i]->is_out)
					mangled_name += "..&" + parameters[i]->type->get_qualified_name();
				else
					mangled_name += ".." + parameters[i]->type->get_qualified_name();
			}
		}
		return mangled_name;
	}

	bool is_static;
	bool is_abstract;
	bool is_virtual;
	bool is_override;

	void visit(TypeMemberVisitor *visitor)
	{
		visitor->member(this);
	}
};

class OperatorTypeMember : public TypeMember
{
public:
	OperatorTypeMember(TypeName *parent, const std::string &name) : TypeMember(parent, name) { }

	Type *type = nullptr;
	std::vector<MethodFixedParameter *> parameters;
	MethodParameterArray *parameter_array = nullptr;

	void visit(TypeMemberVisitor *visitor)
	{
		visitor->member(this);
	}
};
