
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

class MethodFixedParameter;
class TypeSystem;

class FunctionMember : public TypeMember
{
public:
	FunctionMember(TypeName *parent, const std::string &name, Type *return_type) : TypeMember(parent, name), type(return_type) { }
	FunctionMember(TypeSystem *type_system, Type *return_type, std::initializer_list<Type*> args);

	void visit(TypeMemberVisitor *visitor) { }

	Type *type = nullptr;
	std::vector<MethodFixedParameter *> parameters;
};

class MethodTypeMember : public FunctionMember
{
public:
	MethodTypeMember(TypeName *parent, const std::string &name, bool is_static, bool is_event, bool is_native, bool is_exec, bool is_singular, bool is_simulated, bool is_final, bool is_latent, bool is_iterator, bool is_operator, bool is_preoperator, bool is_postoperator, Type *return_type)
		: FunctionMember(parent, name, return_type), 
		is_static(is_static),
		is_event(is_event),
		is_native(is_native),
		is_exec(is_exec),
		is_singular(is_singular),
		is_simulated(is_simulated),
		is_final(is_final),
		is_latent(is_latent),
		is_iterator(is_iterator),
		is_operator(is_operator),
		is_preoperator(is_preoperator),
		is_postoperator(is_postoperator)
	{
	}

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

	bool is_static = false;
	bool is_event = false;
	bool is_native = false;
	bool is_exec = false;
	bool is_singular = false;
	bool is_simulated = false;
	bool is_final = false;
	bool is_latent = false;
	bool is_iterator = false;
	bool is_operator = false;
	bool is_preoperator = false;
	bool is_postoperator = false;

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

	void visit(TypeMemberVisitor *visitor)
	{
		visitor->member(this);
	}
};
