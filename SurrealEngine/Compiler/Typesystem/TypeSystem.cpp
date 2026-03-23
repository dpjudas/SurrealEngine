
#include "TypeSystem.h"
#include "Compiler/Ast/Ast.h"
// #include "Compiler/Codegen/Codegen.h"

TypeSystem::TypeSystem()
{
	root_package = newType<TypePackage>(nullptr);
	core_package = newType<TypePackage>(root_package, "Core");

	native_handle_type = newType<NativeHandleType>(core_package);

	void_type = newType<VoidType>(core_package);
	sbyte_type = newType<SByteType>(core_package);
	byte_type = newType<ByteType>(core_package);
	int16_type = newType<Int16Type>(core_package);
	uint16_type = newType<UInt16Type>(core_package);
	int32_type = newType<Int32Type>(core_package);
	uint32_type = newType<UInt32Type>(core_package);
	int64_type = newType<Int64Type>(core_package);
	uint64_type = newType<UInt64Type>(core_package);
	char_type = newType<CharType>(core_package);
	single_type = newType<SingleType>(core_package);
	double_type = newType<DoubleType>(core_package);
	boolean_type = newType<BooleanType>(core_package);

	// These are populated by parsing the Core package
	core_object = nullptr;
	core_string = nullptr;
	core_value_type = nullptr;
	core_enum = nullptr;

	core_package->types.push_back(native_handle_type);

	core_package->types.push_back(void_type);
	core_package->types.push_back(sbyte_type);
	core_package->types.push_back(byte_type);
	core_package->types.push_back(int16_type);
	core_package->types.push_back(uint16_type);
	core_package->types.push_back(int32_type);
	core_package->types.push_back(uint32_type);
	core_package->types.push_back(int64_type);
	core_package->types.push_back(uint64_type);
	core_package->types.push_back(char_type);
	core_package->types.push_back(single_type);
	core_package->types.push_back(double_type);
	core_package->types.push_back(boolean_type);

	root_package->packages.push_back(core_package);

	unary_operator_sbyte = newType<FunctionMember>(this, sbyte_type, std::initializer_list<Type*>{ sbyte_type });
	unary_operator_byte = newType<FunctionMember>(this, byte_type, std::initializer_list<Type*>{ byte_type });
	unary_operator_int16 = newType<FunctionMember>(this, int16_type, std::initializer_list<Type*>{ int16_type });
	unary_operator_uint16 = newType<FunctionMember>(this, uint16_type, std::initializer_list<Type*>{ uint16_type });
	unary_operator_int32 = newType<FunctionMember>(this, int32_type, std::initializer_list<Type*>{ int32_type });
	unary_operator_uint32 = newType<FunctionMember>(this, uint32_type, std::initializer_list<Type*>{ uint32_type });
	unary_operator_int64 = newType<FunctionMember>(this, int64_type, std::initializer_list<Type*>{ int64_type });
	unary_operator_uint64 = newType<FunctionMember>(this, uint64_type, std::initializer_list<Type*>{ uint64_type });
	unary_operator_char = newType<FunctionMember>(this, char_type, std::initializer_list<Type*>{ char_type });
	unary_operator_single = newType<FunctionMember>(this, single_type, std::initializer_list<Type*>{ single_type });
	unary_operator_double = newType<FunctionMember>(this, double_type, std::initializer_list<Type*>{ double_type });
	unary_operator_boolean = newType<FunctionMember>(this, boolean_type, std::initializer_list<Type*>{ boolean_type });
	unary_operator_error = newType<FunctionMember>(this, void_type, std::initializer_list<Type*>{ uint64_type });

	binary_operator_int32 = newType<FunctionMember>(this, int32_type, std::initializer_list<Type*>{ int32_type, int32_type });
	binary_operator_uint32 = newType<FunctionMember>(this, uint32_type, std::initializer_list<Type*>{ uint32_type, uint32_type });
	binary_operator_uint32_int32 = newType<FunctionMember>(this, uint32_type, std::initializer_list<Type*>{ uint32_type, int32_type });
	binary_operator_int64 = newType<FunctionMember>(this, int64_type, std::initializer_list<Type*>{ int64_type, int64_type });
	binary_operator_int64_int32 = newType<FunctionMember>(this, int64_type, std::initializer_list<Type*>{ int64_type, int32_type });
	binary_operator_uint64 = newType<FunctionMember>(this, uint64_type, std::initializer_list<Type*>{ uint64_type, uint64_type });
	binary_operator_uint64_int32 = newType<FunctionMember>(this, uint64_type, std::initializer_list<Type*>{ uint64_type, int32_type });
	binary_operator_single = newType<FunctionMember>(this, single_type, std::initializer_list<Type*>{ single_type, single_type });
	binary_operator_double = newType<FunctionMember>(this, double_type, std::initializer_list<Type*>{ double_type, double_type });
	binary_operator_boolean = newType<FunctionMember>(this, boolean_type, std::initializer_list<Type*>{ boolean_type, boolean_type });
	binary_operator_string = newType<FunctionMember>(this, core_string, std::initializer_list<Type*>{ core_string, core_string });
	binary_operator_tostring1 = newType<FunctionMember>(this, core_string, std::initializer_list<Type*>{ core_string, core_object });
	binary_operator_tostring2 = newType<FunctionMember>(this, core_string, std::initializer_list<Type*>{ core_object, core_string });
	binary_operator_error1 = newType<FunctionMember>(this, void_type, std::initializer_list<Type*>{ int64_type, uint64_type });
	binary_operator_error2 = newType<FunctionMember>(this, void_type, std::initializer_list<Type*>{ uint64_type, int64_type });

	compare_operator_int32 = newType<FunctionMember>(this, boolean_type, std::initializer_list<Type*>{ int32_type, int32_type });
	compare_operator_uint32 = newType<FunctionMember>(this, boolean_type, std::initializer_list<Type*>{ uint32_type, uint32_type });
	compare_operator_int64 = newType<FunctionMember>(this, boolean_type, std::initializer_list<Type*>{ int64_type, int64_type });
	compare_operator_uint64 = newType<FunctionMember>(this, boolean_type, std::initializer_list<Type*>{ uint64_type, uint64_type });
	compare_operator_single = newType<FunctionMember>(this, boolean_type, std::initializer_list<Type*>{ single_type, single_type });
	compare_operator_double = newType<FunctionMember>(this, boolean_type, std::initializer_list<Type*>{ double_type, double_type });
	compare_operator_boolean = newType<FunctionMember>(this, boolean_type, std::initializer_list<Type*>{ boolean_type, boolean_type });
	compare_operator_string = newType<FunctionMember>(this, boolean_type, std::initializer_list<Type*>{ core_string, core_string });
	compare_operator_error1 = newType<FunctionMember>(this, void_type, std::initializer_list<Type*>{ int64_type, uint64_type });
	compare_operator_error2 = newType<FunctionMember>(this, void_type, std::initializer_list<Type*>{ uint64_type, int64_type });
}

TypeSystem::~TypeSystem()
{
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

		if (c->parameter_array)
		{
			// to do: test for expanded form (14.4.2.1 Applicable function member)
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

	if (src == sbyte_type)
	{
		return dest == byte_type || dest == uint16_type || dest == uint32_type || dest == uint64_type || dest == char_type;
	}
	else if (src == byte_type)
	{
		return dest == sbyte_type || dest == char_type;
	}
	else if (src == int16_type)
	{
		return dest == sbyte_type || dest == byte_type || dest == uint16_type || dest == uint32_type || dest == uint64_type || dest == char_type;
	}
	else if (src == uint16_type)
	{
		return dest == sbyte_type || dest == byte_type || dest == uint16_type || dest == char_type;
	}
	else if (src == int32_type)
	{
		return dest == sbyte_type || dest == byte_type || dest == int16_type || dest == uint16_type || dest == uint32_type || dest == uint64_type || dest == char_type;
	}
	else if (src == uint32_type)
	{
		return dest == sbyte_type || dest == byte_type || dest == int16_type || dest == uint16_type || dest == int32_type || dest == char_type;
	}
	else if (src == int64_type)
	{
		return dest == sbyte_type || dest == byte_type || dest == int16_type || dest == uint16_type || dest == int32_type || dest == uint32_type || dest == uint64_type || dest == char_type;
	}
	else if (src == uint64_type)
	{
		return dest == sbyte_type || dest == byte_type || dest == int16_type || dest == uint16_type || dest == int32_type || dest == uint32_type || dest == int64_type || dest == char_type;
	}
	else if (src == char_type)
	{
		return dest == sbyte_type || dest == byte_type || dest == int16_type;
	}
	else if (src == single_type)
	{
		return dest == sbyte_type || dest == byte_type || dest == int16_type || dest == uint16_type || dest == int32_type || dest == uint32_type || dest == int64_type || dest == uint64_type || dest == char_type;
	}
	else if (src == double_type)
	{
		return dest == sbyte_type || dest == byte_type || dest == int16_type || dest == uint16_type || dest == int32_type || dest == uint32_type || dest == int64_type || dest == uint64_type || dest == char_type || dest == single_type;
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
		system array to array
		array to ilist
		ilist to array
		unbox value
		*/
	}
	return false;
}

bool TypeSystem::implicit_convert_allowed(TypeName *src, TypeName *dest)
{
	if (src == dest) return true;

	if (src == sbyte_type)
	{
		return dest == int16_type || dest == int32_type || dest == int64_type || dest == single_type || dest == double_type;
	}
	else if (src == byte_type)
	{
		return dest == int16_type || dest == int32_type || dest == int64_type || dest == uint16_type || dest == uint32_type || dest == uint64_type || dest == single_type || dest == double_type;
	}
	else if (src == int16_type)
	{
		return dest == int32_type || dest == int64_type || dest == single_type || dest == double_type;
	}
	else if (src == uint16_type)
	{
		return dest == int32_type || dest == int64_type || dest == uint32_type || dest == uint64_type || dest == single_type || dest == double_type;
	}
	else if (src == int32_type)
	{
		return dest == int64_type || dest == single_type || dest == double_type;
	}
	else if (src == uint32_type)
	{
		return dest == int64_type || dest == uint64_type || dest == single_type || dest == double_type;
	}
	else if (src == int64_type || src == uint64_type)
	{
		return dest == single_type || dest == double_type;
	}
	else if (src == char_type)
	{
		return dest == uint16_type || dest == int32_type || dest == int64_type || dest == uint32_type || dest == uint64_type || dest == single_type || dest == double_type;
	}
	else if (src == single_type)
	{
		return dest == double_type;
	}
	else
	{
		/*
		To do:
		ref to object
		class to class
		class to interface
		interface to interface
		array to array
		array to ilist
		array to system array
		delegate to system delegate
		array to system array interface
		delegate to iclonable
		null to ref
		box value
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

	if (t1->type == sbyte_type && (t2->type == byte_type || t2->type == uint16_type || t2->type == uint32_type || t2->type == uint64_type)) return -1;
	if (t2->type == sbyte_type && (t1->type == byte_type || t1->type == uint16_type || t1->type == uint32_type || t1->type == uint64_type)) return 1;
	if (t1->type == int16_type && (t2->type == uint16_type || t2->type == uint32_type || t2->type == uint64_type)) return -1;
	if (t2->type == int16_type && (t1->type == uint16_type || t1->type == uint32_type || t1->type == uint64_type)) return 1;
	if (t1->type == int32_type && (t2->type == uint32_type || t2->type == uint64_type)) return -1;
	if (t2->type == int32_type && (t1->type == uint32_type || t1->type == uint64_type)) return 1;
	if (t1->type == int64_type && t2->type == uint64_type) return -1;
	if (t2->type == int64_type && t1->type == uint64_type) return 1;

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
