
#include "Precomp.h"
#include "TypeSystem.h"
#include "Compiler/Ast/Ast.h"

TypeSystem::TypeSystem()
{
	void_type = newType<VoidType>(nullptr);
	pointer_type = newType<PointerType>(nullptr);
	byte_type = newType<ByteType>(nullptr);
	int_type = newType<IntType>(nullptr);
	single_type = newType<SingleType>(nullptr);
	boolean_type = newType<BooleanType>(nullptr);
	string_type = newType<StringType>(nullptr);

	// To do: import this from the Core package
	object = newType<ClassType>(nullptr, "Object", false);
	addType(object);
	classObject = newType<ClassType>(nullptr, "Class", false);
	classObject->base = object;
	addType(classObject);

	addType(void_type);
	addType(byte_type);
	addType(int_type);
	addType(single_type);
	addType(boolean_type);
	addType(string_type);

	unary_operator_byte = newType<FunctionMember>(this, byte_type, std::initializer_list<Type*>{ byte_type });
	unary_operator_int = newType<FunctionMember>(this, int_type, std::initializer_list<Type*>{ int_type });
	unary_operator_single = newType<FunctionMember>(this, single_type, std::initializer_list<Type*>{ single_type });
	unary_operator_boolean = newType<FunctionMember>(this, boolean_type, std::initializer_list<Type*>{ boolean_type });

	binary_operator_int = newType<FunctionMember>(this, int_type, std::initializer_list<Type*>{ int_type, int_type });
	binary_operator_single = newType<FunctionMember>(this, single_type, std::initializer_list<Type*>{ single_type, single_type });
	binary_operator_boolean = newType<FunctionMember>(this, boolean_type, std::initializer_list<Type*>{ boolean_type, boolean_type });
	binary_operator_string = newType<FunctionMember>(this, string_type, std::initializer_list<Type*>{ string_type, string_type });

	compare_operator_int = newType<FunctionMember>(this, boolean_type, std::initializer_list<Type*>{ int_type, int_type });
	compare_operator_single = newType<FunctionMember>(this, boolean_type, std::initializer_list<Type*>{ single_type, single_type });
	compare_operator_boolean = newType<FunctionMember>(this, boolean_type, std::initializer_list<Type*>{ boolean_type, boolean_type });
	compare_operator_string = newType<FunctionMember>(this, boolean_type, std::initializer_list<Type*>{ string_type, string_type });
}

TypeSystem::~TypeSystem()
{
}

void TypeSystem::addType(Type* type)
{
	types.push_back(type);
	if (!type->name.empty())
		nameToType[type->name] = type;
}

FunctionMember *TypeSystem::find_best_function(const std::vector<FunctionMember *> &candidates, const std::vector<ExpressionResult> &args)
{
	std::vector<FunctionMember *> applicableFuncs;
	applicableFuncs.reserve(candidates.size());
	for (FunctionMember *c : candidates)
	{
		bool applicable = true;
		if (c->parameters.size() == args.size())
		{
			int i = 0;
			for (MethodFixedParameter *p : c->parameters)
			{
				const ExpressionResult &arg = args[i++];
				if (((p->is_out || p->is_ref) && p->type != arg.type) || !implicit_convert_allowed(arg.type, p->type))
				{
					applicable = false;
					break;
				}
			}
		}
		else
		{
			applicable = false;
		}

		if (applicable)
			applicableFuncs.push_back(c);
	}

	if (applicableFuncs.empty())
		return nullptr;

	FunctionMember *best_func = applicableFuncs.front();

	for (size_t i = 1; i < applicableFuncs.size(); i++)
	{
		FunctionMember *c = applicableFuncs[i];

		int better = 0; // 0 = same, -1 = best_func is better, 1 = c is better
		for (size_t j = 0; j < c->parameters.size(); j++)
		{
			int compare = compare_conversion(args[j], best_func->parameters[j], c->parameters[j]);
			if (better == 0)
				better = compare;

			if (compare != better)
				return nullptr; // one isn't better than the other
		}

		if (better == 0)
			return nullptr; // at least one argument has to be better
		else if (better == 1)
			best_func = c;

		// To do: this is more complicated for expanded form or for generics (14.4.2.2 Better function member)
	}

	return best_func;
}

bool TypeSystem::explicit_convert_allowed(TypeName *src, TypeName *dest)
{
	if (implicit_convert_allowed(src, dest))
		return true;

	if (src == byte_type)
	{
		return false;
	}
	else if (src == int_type)
	{
		return dest == byte_type;
	}
	else if (src == single_type)
	{
		return dest == byte_type || dest == int_type;
	}
	else
	{
		/*
		To do:
		numeric to enum
		enum to numeric
		enum to enum
		object to ref
		class to class
		array to array
		*/
	}
	return false;
}

bool TypeSystem::implicit_convert_allowed(TypeName *src, TypeName *dest)
{
	if (src == dest) return true;

	if (src == byte_type)
	{
		return dest == int_type || dest == single_type;
	}
	else if (src == int_type)
	{
		return dest == single_type;
	}
	else if (src == single_type)
	{
		return false;
	}
	else
	{
		/*
		To do:
		ref to object
		class to class
		array to array
		null to ref
		*/
	}

	return false;
}

int TypeSystem::compare_conversion(const ExpressionResult &src, MethodFixedParameter *t1, MethodFixedParameter *t2)
{
	if (t1->type == t2->type) return 0;
	if (src.type == t1->type) return -1;
	if (src.type == t2->type) return 1;

	bool conv1 = implicit_convert_allowed(t1->type, t2->type);
	bool conv2 = implicit_convert_allowed(t2->type, t1->type);
	if (conv1 && !conv2) return -1;
	if (!conv1 && conv2) return 1;
	return 0;
}

FunctionMember::FunctionMember(TypeSystem *type_system, Type *return_type, std::initializer_list<Type*> args) : TypeMember(nullptr, {})
{
	type = return_type;
	for (Type *arg : args)
	{
		auto fixed_param = type_system->newFixedParameter<MethodFixedParameter>();
		fixed_param->type = arg;
		parameters.push_back(fixed_param);
	}
}
