
#include "Precomp.h"
#include "ExpressionSema.h"
#include "SemanticAnalysis.h"
#include "Compiler/Typesystem/TypeSystem.h"
#include "Compiler/Lex/TextUtil.h"
#include <cmath>

ExpressionSema::ExpressionSema(SemanticAnalysis& sema, NameScope& name_scope, TypeScope& type_scope) : sema(sema), name_scope(name_scope), type_scope(type_scope)
{
}

void ExpressionSema::analyze(AstExpression* expression)
{
	expression->visit(this);
}

void ExpressionSema::expression(AstArrayCreationExpression* node)
{
	throw SemaException("Arrays not supported yet", node);
}

void ExpressionSema::expression(AstLiteral* node)
{
	if (node->type == AstLiteralType::integer)
	{
		size_t len = node->value.size();
		char suffix1 = len > 1 ? node->value[len - 2] : 0;
		char suffix2 = len > 0 ? node->value[len - 1] : 0;
		bool hexidecimal = len > 2 && node->value[0] == '0' && node->value[1] == 'x';
		bool unsigned_suffix = suffix1 == 'U' || suffix1 == 'u' || suffix2 == 'U' || suffix2 == 'u';
		bool long_suffix = suffix1 == 'L' || suffix1 == 'l' || suffix2 == 'L' || suffix2 == 'l';

		uint64_t value = 0;
		if (hexidecimal)
		{
			for (size_t i = 2; i < len; i++)
			{
				char c = node->value[i];
				if (c >= '0' && c <= '9')
					value = (value << 4) + (c - '0');
				else if (c >= 'a' && c <= 'f')
					value = (value << 4) + 10 + (c - 'a');
				else if (c >= 'A' && c <= 'F')
					value = (value << 4) + 10 + (c - 'A');
				else
					break;
			}
		}
		else
		{
			for (size_t i = 0; i < len; i++)
			{
				char c = node->value[i];
				if (c >= '0' && c <= '9')
					value = value * 10 + (c - '0');
				else
					break;
			}
		}

		/*if (value > 0x7fff'ffff'ffff'ffffULL || (unsigned_suffix && long_suffix))
		{
			node->result = { sema.type_system().uint64_type, ExpressionClass::value };
			node->result.constval.u64 = value;
		}
		else if (value > 0xffff'ffffULL || long_suffix)
		{
			node->result = { sema.type_system().int64_type, ExpressionClass::value };
			node->result.constval.i64 = (int64_t)value;
		}
		else if (value > 0x7fff'ffffULL || unsigned_suffix)
		{
			node->result = { sema.type_system().uint_type, ExpressionClass::value };
			node->result.constval.u32 = (uint32_t)value;
		}
		else */
		{
			node->result = { sema.type_system().int_type, ExpressionClass::value };
			node->result.constval.i32 = (int32_t)value;
		}
	}
	else if (node->type == AstLiteralType::real)
	{
		if (node->value.back() == 'f')
		{
			node->result = { sema.type_system().single_type, ExpressionClass::value };
			node->result.constval.f32 = (float)std::atof(node->value.substr(0, node->value.size() - 1).c_str());
		}
		else
		{
			node->result = { sema.type_system().single_type, ExpressionClass::value };
			node->result.constval.f32 = (float)std::atof(node->value.c_str());
		}
	}
	else if (node->type == AstLiteralType::boolean)
	{
		node->result = { sema.type_system().boolean_type, ExpressionClass::value };
		node->result.constval.i1 = node->value == "true";
	}
	else if (node->type == AstLiteralType::string)
	{
		node->result = { sema.type_system().string_type, ExpressionClass::value };
		node->result.constval.str = node->value;
	}
	else if (node->type == AstLiteralType::none)
	{
		node->result = { sema.type_system().null_type, ExpressionClass::value };
	}
	else
	{
		throw SemaException("Unknown literal type", node);
	}

	node->result.constval.is_constant = true;
}

void ExpressionSema::expression(AstSimpleName* node)
{
	if (name_scope.variables.find(node->identifier) != name_scope.variables.end())
	{
		node->result = name_scope.variables[node->identifier];
		return;
	}

	for (auto it = type_scope.scopes.rbegin(); it != type_scope.scopes.rend(); ++it)
	{
		TypeName* scope = *it;
		MemberLookup lookup(sema.type_system());
		lookup.lookup(scope, node->identifier);
		if (lookup.members.size() == 1 && !dynamic_cast<MethodTypeMember*>(*lookup.members.begin()))
		{
			TypeName* member = *lookup.members.begin();
			if (FieldTypeMember* field = dynamic_cast<FieldTypeMember*>(member))
				node->result = { field->type, ExpressionClass::variable };
			else
				node->result = { member, ExpressionClass::type };
			node->result.member = member;
			return;
		}
		else if (!lookup.members.empty())
		{
			std::vector<FunctionMember*> method_group;
			for (TypeName* member : lookup.members)
			{
				if (auto method = dynamic_cast<MethodTypeMember*>(member))
					method_group.push_back(method);
			}
			if (method_group.size() != lookup.members.size())
				throw SemaException("Ambiguous member lookup", node);
			node->result = { name_scope.variables["this"].type, ExpressionClass::method_group };
			node->result.method_group = method_group;
			return;
		}
	}

	AstIdentifierName name;
	name.name = node->identifier;
	TypeName* type_name = type_scope.lookup_type(&name);
	if (!type_name)
		throw SemaException("Unknown identifier or type name", node);
	node->result = { type_name, ExpressionClass::type };
}

void ExpressionSema::expression(AstNamedObject* node)
{
	// To do: implement this
}

void ExpressionSema::expression(AstMemberAccess* node)
{
	ExpressionResult operand;

	if (node->expression)
	{
		node->expression->visit(this);
		operand = node->expression->result;
	}
	else
	{
		throw SemaException("Invalid member access node", node);
	}

	if (operand.variant == ExpressionClass::type)
	{
		MemberLookup lookup(sema.type_system());
		lookup.lookup(operand.type, node->identifier);
		if (lookup.members.size() == 1 && !dynamic_cast<MethodTypeMember*>(*lookup.members.begin()))
		{
			// To do: throw error if field isn't static

			TypeName* member = *lookup.members.begin();
			if (Type* type = dynamic_cast<Type*>(member))
				node->result = { member, ExpressionClass::type };
			else if (FieldTypeMember* field = dynamic_cast<FieldTypeMember*>(member))
				node->result = { field->type, ExpressionClass::variable };
			//else if (ConstantTypeMember *constant = dynamic_cast<ConstantTypeMember*>(member))
			//	node->result = { constant->type, ExpressionClass::value };
			//else if (EnumValueTypeMember *enumval = dynamic_cast<EnumValueTypeMember*>(member))
			//	node->result = { enumval->type, ExpressionClass::value };
			else
				throw SemaException("Invalid member reference", node);
			node->result.member = member;
			return;
		}
		else if (!lookup.members.empty())
		{
			std::vector<FunctionMember*> method_group;
			for (TypeName* member : lookup.members)
			{
				if (auto method = dynamic_cast<MethodTypeMember*>(member))
					method_group.push_back(method);
			}
			if (method_group.size() != lookup.members.size())
				throw SemaException("Ambiguous member lookup", node);
			node->result = { sema.type_system().void_type, ExpressionClass::method_group };
			node->result.method_group = method_group;
			return;
		}
	}
	else
	{
		MemberLookup lookup(sema.type_system());
		lookup.lookup(operand.type, node->identifier);

		if (lookup.members.size() == 1 && !dynamic_cast<MethodTypeMember*>(*lookup.members.begin()))
		{
			TypeName* member = *lookup.members.begin();
			if (FieldTypeMember* field = dynamic_cast<FieldTypeMember*>(member))
				node->result = { field->type, ExpressionClass::variable };
			else
				throw SemaException("Invalid member reference", node);
			node->result.member = member;
			return;
		}
		else if (!lookup.members.empty())
		{
			std::vector<FunctionMember*> method_group;
			for (TypeName* member : lookup.members)
			{
				if (auto method = dynamic_cast<MethodTypeMember*>(member))
					method_group.push_back(method);
			}
			if (method_group.size() != lookup.members.size())
				throw SemaException("Ambiguous member lookup", node);
			node->result = { operand.type, ExpressionClass::method_group };
			node->result.method_group = method_group;
			return;
		}
	}
	throw SemaException("Invalid member reference", node);
}

void ExpressionSema::expression(AstInvocationExpression* node)
{
	node->expression->visit(this);

	if (node->expression->result.variant != ExpressionClass::method_group)
		throw SemaException("Method group expected", node);

	std::vector<ExpressionResult> args;
	for (size_t i = 0; i < node->args.size(); i++)
	{
		node->args[i]->expression->visit(this);
		args.push_back(node->args[i]->expression->result);
	}

	FunctionMember* func = sema.type_system().find_best_function(node->expression->result.method_group, args);
	if (!func)
		throw SemaException("No suitable overload found", node);

	node->result = { func->type, ExpressionClass::value, func };
}

void ExpressionSema::expression(AstElementAccess* node)
{
	throw SemaException("Arrays not yet supported", node);
#if 0
	node->expression->visit(this);

	ExpressionResult member = node->expression->result;

	if (dynamic_cast<ArrayType*>(member.type))
	{
		// To do: add array support
		throw SemaException("Arrays not yet supported");
	}
	else
	{
		std::vector<FunctionMember*> candidates;

		if (auto class_type = dynamic_cast<ClassType*>(member.type))
		{
			if (class_type->is_abstract)
				throw SemaException("new not allowed for abstract class types");

			candidates.reserve(class_type->indexers.size());
			for (auto c : class_type->indexers)
				candidates.push_back(c);
		}
		else if (auto struct_type = dynamic_cast<StructType*>(member.type))
		{
			candidates.reserve(struct_type->indexers.size());
			for (auto c : struct_type->indexers)
				candidates.push_back(c);
		}
		else
		{
			throw SemaException("Invalid type specified");
		}

		std::vector<ExpressionResult> args;
		for (size_t i = 0; i < node->args.size(); i++)
		{
			node->args[i]->visit(this);
			args.push_back(node->args[i]->result);
		}

		FunctionMember* indexer = sema.type_system().find_best_function(candidates, args);
		if (!indexer)
			throw SemaException("No suitable indexer found");

		node->result = { indexer->type, ExpressionClass::indexer, indexer };
	}
#endif
}

void ExpressionSema::expression(AstBaseAccess* node)
{
	ExpressionResult operand = name_scope.variables["this"];

	// To do: convert 'this' type to 'base' type

	node->result = operand;
}

void ExpressionSema::expression(AstPostIncrementExpression* node)
{
	node->expression->visit(this);

	FunctionMember* func = get_increment_overload(node->expression->result);
	node->result = { func->type, ExpressionClass::value, func };
}

void ExpressionSema::expression(AstPostDecrementExpression* node)
{
	node->expression->visit(this);

	FunctionMember* func = get_decrement_overload(node->expression->result);
	node->result = { func->type, ExpressionClass::value, func };
}

void ExpressionSema::expression(AstNewExpression* node)
{
	Type* type = type_scope.lookup_type(node->type);
#if 0
	std::vector<FunctionMember*> candidates;

	if (auto class_type = dynamic_cast<ClassType*>(type))
	{
		if (class_type->is_abstract)
			throw SemaException("new not allowed for abstract class types");

		candidates.reserve(class_type->constructors.size());
		for (auto c : class_type->constructors)
			candidates.push_back(c);
	}
	else if (auto struct_type = dynamic_cast<StructType*>(type))
	{
		candidates.reserve(struct_type->constructors.size());
		for (auto c : struct_type->constructors)
			candidates.push_back(c);
	}
	else
	{
		throw SemaException("Invalid type specified");
	}

	std::vector<ExpressionResult> args;
	for (size_t i = 0; i < node->args.size(); i++)
	{
		node->args[i]->visit(this);
		args.push_back(node->args[i]->result);
	}

	FunctionMember* constructor = sema.type_system().find_best_function(candidates, args);
	if (!constructor)
		throw SemaException("No suitable constructor found");

	node->result = { type, ExpressionClass::variable, constructor };
#endif
}

void ExpressionSema::expression(AstTypeofExpression* node)
{
	throw SemaException("typeof not supported", node);
}

void ExpressionSema::expression(AstParenthesizedExpression* node)
{
	node->expression->visit(this);
	node->result = node->expression->result;

	if (node->result.variant == ExpressionClass::type)
		throw SemaException("type not allowed", node);
}

void ExpressionSema::expression(AstSizeofExpression* node)
{
	node->result = { sema.type_system().int_type, ExpressionClass::value };
}

void ExpressionSema::expression(AstAnonymousMethodExpression* node)
{
	throw SemaException("anonymous functions not supported", node);
}

void ExpressionSema::expression(AstUnaryPlusExpression* node)
{
	node->operand->visit(this);

	FunctionMember* func = get_plus_overload(node->operand->result);
	node->result = { func->type, ExpressionClass::value, func };
}

void ExpressionSema::expression(AstUnaryMinusExpression* node)
{
	node->operand->visit(this);

	FunctionMember* func = get_minus_overload(node->operand->result);
	node->result = { func->type, ExpressionClass::value, func };
}

void ExpressionSema::expression(AstUnaryLogicalNotExpression* node)
{
	node->operand->visit(this);

	FunctionMember* func = get_logical_not_overload(node->operand->result);
	node->result = { func->type, ExpressionClass::value, func };
}

void ExpressionSema::expression(AstUnaryBitwiseComplementExpression* node)
{
	node->operand->visit(this);

	FunctionMember* func = get_bitwise_complement_overload(node->operand->result);
	node->result = { func->type, ExpressionClass::value, func };
}

void ExpressionSema::expression(AstUnaryPreIncrementExpression* node)
{
	node->operand->visit(this);

	FunctionMember* func = get_increment_overload(node->operand->result);
	node->result = { func->type, ExpressionClass::value, func };
}

void ExpressionSema::expression(AstUnaryPreDecrementExpression* node)
{
	node->operand->visit(this);

	FunctionMember* func = get_decrement_overload(node->operand->result);
	node->result = { func->type, ExpressionClass::value, func };
}

void ExpressionSema::expression(AstMultiplicationExpression* node)
{
	node->operand1->visit(this);
	node->operand2->visit(this);

	FunctionMember* func = get_multiplication_overload(node->operand1->result, node->operand2->result);
	node->result = { func->type, ExpressionClass::value, func };
}

void ExpressionSema::expression(AstExponentiationExpression* node)
{
	// To do: implement this
}

void ExpressionSema::expression(AstDivisionExpression* node)
{
	node->operand1->visit(this);
	node->operand2->visit(this);

	FunctionMember* func = get_division_overload(node->operand1->result, node->operand2->result);
	node->result = { func->type, ExpressionClass::value, func };
}

void ExpressionSema::expression(AstRemainderExpression* node)
{
	node->operand1->visit(this);
	node->operand2->visit(this);

	FunctionMember* func = get_remainder_overload(node->operand1->result, node->operand2->result);
	node->result = { func->type, ExpressionClass::value, func };
}

void ExpressionSema::expression(AstAdditionExpression* node)
{
	node->operand1->visit(this);
	node->operand2->visit(this);

	FunctionMember* func = get_addition_overload(node->operand1->result, node->operand2->result);
	node->result = { func->type, ExpressionClass::value, func };
}

void ExpressionSema::expression(AstSubtractionExpression* node)
{
	node->operand1->visit(this);
	node->operand2->visit(this);

	FunctionMember* func = get_subtraction_overload(node->operand1->result, node->operand2->result);
	node->result = { func->type, ExpressionClass::value, func };
}

void ExpressionSema::expression(AstStringConcatExpression* node)
{
	// To do: implement this
}

void ExpressionSema::expression(AstStringSpaceConcatExpression* node)
{
	// To do: implement this
}

void ExpressionSema::expression(AstDotProductExpression* node)
{
	// To do: implement this
}

void ExpressionSema::expression(AstCrossProductExpression* node)
{
	// To do: implement this
}

void ExpressionSema::expression(AstShiftLeftExpression* node)
{
	node->operand1->visit(this);
	node->operand2->visit(this);

	FunctionMember* func = get_shift_left_overload(node->operand1->result, node->operand2->result);
	node->result = { func->type, ExpressionClass::value, func };
}

void ExpressionSema::expression(AstShiftRightExpression* node)
{
	node->operand1->visit(this);
	node->operand2->visit(this);

	FunctionMember* func = get_shift_right_overload(node->operand1->result, node->operand2->result);
	node->result = { func->type, ExpressionClass::value, func };
}

void ExpressionSema::expression(AstUnsignedShiftRightExpression* node)
{
	// To do: implement this
}

void ExpressionSema::expression(AstLessExpression* node)
{
	node->operand1->visit(this);
	node->operand2->visit(this);

	FunctionMember* func = get_less_overload(node->operand1->result, node->operand2->result);
	node->result = { func->type, ExpressionClass::value, func };
}

void ExpressionSema::expression(AstGreaterExpression* node)
{
	node->operand1->visit(this);
	node->operand2->visit(this);

	FunctionMember* func = get_greater_overload(node->operand1->result, node->operand2->result);
	node->result = { func->type, ExpressionClass::value, func };
}

void ExpressionSema::expression(AstLessEqualExpression* node)
{
	node->operand1->visit(this);
	node->operand2->visit(this);

	FunctionMember* func = get_less_equal_overload(node->operand1->result, node->operand2->result);
	node->result = { func->type, ExpressionClass::value, func };
}

void ExpressionSema::expression(AstGreaterEqualExpression* node)
{
	node->operand1->visit(this);
	node->operand2->visit(this);

	FunctionMember* func = get_greater_equal_overload(node->operand1->result, node->operand2->result);
	node->result = { func->type, ExpressionClass::value, func };
}

void ExpressionSema::expression(AstIsExpression* node)
{
	throw SemaException("is keyword not supported", node);
}

void ExpressionSema::expression(AstAsExpression* node)
{
	throw SemaException("as keyword not supported", node);
}

void ExpressionSema::expression(AstEqualExpression* node)
{
	node->operand1->visit(this);
	node->operand2->visit(this);

	FunctionMember* func = get_equal_overload(node->operand1->result, node->operand2->result);
	node->result = { func->type, ExpressionClass::value, func };
}

void ExpressionSema::expression(AstNotEqualExpression* node)
{
	node->operand1->visit(this);
	node->operand2->visit(this);

	FunctionMember* func = get_not_equal_overload(node->operand1->result, node->operand2->result);
	node->result = { func->type, ExpressionClass::value, func };
}

void ExpressionSema::expression(AstCaseInsensitiveEqualExpression* node)
{
	// To do: implement this
}

void ExpressionSema::expression(AstLogicalAndExpression* node)
{
	node->operand1->visit(this);
	node->operand2->visit(this);

	FunctionMember* func = get_logical_and_overload(node->operand1->result, node->operand2->result);
	node->result = { func->type, ExpressionClass::value, func };
}

void ExpressionSema::expression(AstLogicalOrExpression* node)
{
	node->operand1->visit(this);
	node->operand2->visit(this);

	FunctionMember* func = get_logical_or_overload(node->operand1->result, node->operand2->result);
	node->result = { func->type, ExpressionClass::value, func };
}

void ExpressionSema::expression(AstLogicalXorExpression* node)
{
	node->operand1->visit(this);
	node->operand2->visit(this);

	FunctionMember* func = get_logical_xor_overload(node->operand1->result, node->operand2->result);
	node->result = { func->type, ExpressionClass::value, func };
}

void ExpressionSema::expression(AstLogicalXorXorExpression* node)
{
	// To do: implement this
}

void ExpressionSema::expression(AstConditionalAndExpression* node)
{
	node->operand1->visit(this);
	node->operand2->visit(this);

	FunctionMember* func = get_conditional_and_overload(node->operand1->result, node->operand2->result);
	node->result = { sema.type_system().boolean_type, ExpressionClass::value, func };
}

void ExpressionSema::expression(AstConditionalOrExpression* node)
{
	node->operand1->visit(this);
	node->operand2->visit(this);

	FunctionMember* func = get_conditional_or_overload(node->operand1->result, node->operand2->result);
	node->result = { sema.type_system().boolean_type, ExpressionClass::value, func };
}

void ExpressionSema::expression(AstAssignmentExpression* node)
{
	node->operand1->visit(this);
	node->operand2->visit(this);

	if (node->assignment_type == "=")
	{
		if (!sema.type_system().implicit_convert_allowed(node->operand2->result.type, node->operand1->result.type))
			throw SemaException("No suitable operator found", node);

		node->result = { node->operand1->result.type, ExpressionClass::value };
	}
	else
	{
		FunctionMember* func;

		if (node->assignment_type == "+=")
			func = get_addition_overload(node->operand1->result, node->operand2->result);
		else if (node->assignment_type == "-=")
			func = get_subtraction_overload(node->operand1->result, node->operand2->result);
		else if (node->assignment_type == "*=")
			func = get_multiplication_overload(node->operand1->result, node->operand2->result);
		else if (node->assignment_type == "/=")
			func = get_division_overload(node->operand1->result, node->operand2->result);
		else if (node->assignment_type == "%=")
			func = get_remainder_overload(node->operand1->result, node->operand2->result);
		else if (node->assignment_type == "&=")
			func = get_logical_and_overload(node->operand1->result, node->operand2->result);
		else if (node->assignment_type == "|=")
			func = get_logical_or_overload(node->operand1->result, node->operand2->result);
		else if (node->assignment_type == "^=")
			func = get_logical_xor_overload(node->operand1->result, node->operand2->result);
		else if (node->assignment_type == "<<=")
			func = get_shift_left_overload(node->operand1->result, node->operand2->result);
		else if (node->assignment_type == ">>=")
			func = get_shift_right_overload(node->operand1->result, node->operand2->result);
		else
			throw SemaException("Unknown assignment type", node);

		TypeName* x = node->operand1->result.type;
		TypeName* y = node->operand2->result.type;

		auto& ts = sema.type_system();
		bool convert_allowed = ts.implicit_convert_allowed(func->type, x);
		if (!convert_allowed)
			convert_allowed = ts.explicit_convert_allowed(func->type, x) && (ts.implicit_convert_allowed(x, y) || node->assignment_type == "<<=" || node->assignment_type == ">>=");

		if (!convert_allowed)
			throw SemaException("No suitable operator found", node);

		node->result = { func->type, ExpressionClass::value, func };
	}
}

void ExpressionSema::expression(AstTrinaryExpression* node)
{
	throw SemaException("Trinary operator not implemented", node);
}

FunctionMember* ExpressionSema::get_increment_overload(const ExpressionResult& a)
{
	auto& ts = sema.type_system();
	std::vector<FunctionMember*> candidates =
	{
		ts.unary_operator_byte,
		ts.unary_operator_int,
		ts.unary_operator_single,
	};

	//enum // one exists for every enum

	FunctionMember* func = ts.find_best_function(candidates, { a });
	if (!func)
		throw SemaException("No suitable operator found", nullptr);
	return func;
}

FunctionMember* ExpressionSema::get_decrement_overload(const ExpressionResult& a)
{
	auto& ts = sema.type_system();
	std::vector<FunctionMember*> candidates =
	{
		ts.unary_operator_byte,
		ts.unary_operator_int,
		ts.unary_operator_single,
	};

	//enum // one exists for every enum

	FunctionMember* func = ts.find_best_function(candidates, { a });
	if (!func)
		throw SemaException("No suitable operator found", nullptr);
	return func;
}

FunctionMember* ExpressionSema::get_plus_overload(const ExpressionResult& a)
{
	auto& ts = sema.type_system();
	std::vector<FunctionMember*> candidates =
	{
		ts.unary_operator_int,
		ts.unary_operator_single,
	};

	FunctionMember* func = ts.find_best_function(candidates, { a });
	if (!func)
		throw SemaException("No suitable operator found", nullptr);
	return func;
}

FunctionMember* ExpressionSema::get_minus_overload(const ExpressionResult& a)
{
	auto& ts = sema.type_system();
	std::vector<FunctionMember*> candidates =
	{
		ts.unary_operator_int,
		ts.unary_operator_single,
	};

	FunctionMember* func = ts.find_best_function(candidates, { a });
	if (!func)
		throw SemaException("No suitable operator found", nullptr);
	return func;
}

FunctionMember* ExpressionSema::get_logical_not_overload(const ExpressionResult& a)
{
	auto& ts = sema.type_system();
	std::vector<FunctionMember*> candidates =
	{
		ts.unary_operator_boolean
	};

	FunctionMember* func = ts.find_best_function(candidates, { a });
	if (!func)
		throw SemaException("No suitable operator found", nullptr);
	return func;
}

FunctionMember* ExpressionSema::get_bitwise_complement_overload(const ExpressionResult& a)
{
	auto& ts = sema.type_system();
	std::vector<FunctionMember*> candidates =
	{
		ts.unary_operator_int,
	};

	//enum // one exists for every enum

	FunctionMember* func = ts.find_best_function(candidates, { a });
	if (!func)
		throw SemaException("No suitable operator found", nullptr);
	return func;
}

FunctionMember* ExpressionSema::get_addition_overload(const ExpressionResult& a, const ExpressionResult& b)
{
	auto& ts = sema.type_system();
	std::vector<FunctionMember*> candidates =
	{
		ts.binary_operator_int,
		ts.binary_operator_single,
		ts.binary_operator_string,
	};

	//addition_enum_int, // one exists for every enum and its underlying type
	//addition_int_enum, // one exists for every enum and its underlying type

	FunctionMember* func = ts.find_best_function(candidates, { a, b });
	if (!func)
		throw SemaException("No suitable operator found", nullptr);
	return func;
}

FunctionMember* ExpressionSema::get_subtraction_overload(const ExpressionResult& a, const ExpressionResult& b)
{
	auto& ts = sema.type_system();
	std::vector<FunctionMember*> candidates =
	{
		ts.binary_operator_int,
		ts.binary_operator_single,
	};

	//addition_enum_int, // one exists for every enum and its underlying type
	//addition_int_enum, // one exists for every enum and its underlying type

	FunctionMember* func = ts.find_best_function(candidates, { a, b });
	if (!func)
		throw SemaException("No suitable operator found", nullptr);
	return func;
}

FunctionMember* ExpressionSema::get_multiplication_overload(const ExpressionResult& a, const ExpressionResult& b)
{
	auto& ts = sema.type_system();
	std::vector<FunctionMember*> candidates =
	{
		ts.binary_operator_int,
		ts.binary_operator_single,
	};

	FunctionMember* func = ts.find_best_function(candidates, { a, b });
	if (!func)
		throw SemaException("No suitable operator found", nullptr);
	return func;
}

FunctionMember* ExpressionSema::get_division_overload(const ExpressionResult& a, const ExpressionResult& b)
{
	auto& ts = sema.type_system();
	std::vector<FunctionMember*> candidates =
	{
		ts.binary_operator_int,
		ts.binary_operator_single,
	};

	FunctionMember* func = ts.find_best_function(candidates, { a, b });
	if (!func)
		throw SemaException("No suitable operator found", nullptr);
	return func;
}

FunctionMember* ExpressionSema::get_remainder_overload(const ExpressionResult& a, const ExpressionResult& b)
{
	auto& ts = sema.type_system();
	std::vector<FunctionMember*> candidates =
	{
		ts.binary_operator_int,
		ts.binary_operator_single,
	};

	FunctionMember* func = ts.find_best_function(candidates, { a, b });
	if (!func)
		throw SemaException("No suitable operator found", nullptr);
	return func;
}

FunctionMember* ExpressionSema::get_logical_and_overload(const ExpressionResult& a, const ExpressionResult& b)
{
	auto& ts = sema.type_system();
	std::vector<FunctionMember*> candidates =
	{
		ts.binary_operator_int,
		ts.binary_operator_boolean,
	};

	//and_enum, // one exists for every enum type, only considered for overload resolution if one of the operands already match

	FunctionMember* func = ts.find_best_function(candidates, { a, b });
	if (!func)
		throw SemaException("No suitable operator found", nullptr);
	return func;
}

FunctionMember* ExpressionSema::get_logical_or_overload(const ExpressionResult& a, const ExpressionResult& b)
{
	auto& ts = sema.type_system();
	std::vector<FunctionMember*> candidates =
	{
		ts.binary_operator_int,
		ts.binary_operator_boolean,
	};

	//and_enum, // one exists for every enum type, only considered for overload resolution if one of the operands already match

	FunctionMember* func = ts.find_best_function(candidates, { a, b });
	if (!func)
		throw SemaException("No suitable operator found", nullptr);
	return func;
}

FunctionMember* ExpressionSema::get_logical_xor_overload(const ExpressionResult& a, const ExpressionResult& b)
{
	auto& ts = sema.type_system();
	std::vector<FunctionMember*> candidates =
	{
		ts.binary_operator_int,
		ts.binary_operator_boolean,
	};

	//and_enum, // one exists for every enum type, only considered for overload resolution if one of the operands already match

	FunctionMember* func = ts.find_best_function(candidates, { a, b });
	if (!func)
		throw SemaException("No suitable operator found", nullptr);
	return func;
}

FunctionMember* ExpressionSema::get_shift_left_overload(const ExpressionResult& a, const ExpressionResult& b)
{
	auto& ts = sema.type_system();
	std::vector<FunctionMember*> candidates =
	{
		ts.binary_operator_int,
	};

	FunctionMember* func = ts.find_best_function(candidates, { a, b });
	if (!func)
		throw SemaException("No suitable operator found", nullptr);
	return func;
}

FunctionMember* ExpressionSema::get_shift_right_overload(const ExpressionResult& a, const ExpressionResult& b)
{
	auto& ts = sema.type_system();
	std::vector<FunctionMember*> candidates =
	{
		ts.binary_operator_int,
	};

	FunctionMember* func = ts.find_best_function(candidates, { a, b });
	if (!func)
		throw SemaException("No suitable operator found", nullptr);
	return func;
}

FunctionMember* ExpressionSema::get_less_overload(const ExpressionResult& a, const ExpressionResult& b)
{
	auto& ts = sema.type_system();
	std::vector<FunctionMember*> candidates =
	{
		ts.compare_operator_int,
		ts.compare_operator_single,
	};

	//less_enum, // one exists for every enum type, only considered for overload resolution if one of the operands already match

	FunctionMember* func = ts.find_best_function(candidates, { a, b });
	if (!func)
		throw SemaException("No suitable operator found", nullptr);
	return func;
}

FunctionMember* ExpressionSema::get_greater_overload(const ExpressionResult& a, const ExpressionResult& b)
{
	auto& ts = sema.type_system();
	std::vector<FunctionMember*> candidates =
	{
		ts.compare_operator_int,
		ts.compare_operator_single,
	};

	//greater_enum, // one exists for every enum type, only considered for overload resolution if one of the operands already match

	FunctionMember* func = ts.find_best_function(candidates, { a, b });
	if (!func)
		throw SemaException("No suitable operator found", nullptr);
	return func;
}

FunctionMember* ExpressionSema::get_less_equal_overload(const ExpressionResult& a, const ExpressionResult& b)
{
	auto& ts = sema.type_system();
	std::vector<FunctionMember*> candidates =
	{
		ts.compare_operator_int,
		ts.compare_operator_single,
	};

	//less_equal_enum, // one exists for every enum type, only considered for overload resolution if one of the operands already match

	FunctionMember* func = ts.find_best_function(candidates, { a, b });
	if (!func)
		throw SemaException("No suitable operator found", nullptr);
	return func;
}

FunctionMember* ExpressionSema::get_greater_equal_overload(const ExpressionResult& a, const ExpressionResult& b)
{
	auto& ts = sema.type_system();
	std::vector<FunctionMember*> candidates =
	{
		ts.compare_operator_int,
		ts.compare_operator_single,
	};

	//greater_equal_enum, // one exists for every enum type, only considered for overload resolution if one of the operands already match

	FunctionMember* func = ts.find_best_function(candidates, { a, b });
	if (!func)
		throw SemaException("No suitable operator found", nullptr);
	return func;
}

FunctionMember* ExpressionSema::get_equal_overload(const ExpressionResult& a, const ExpressionResult& b)
{
	auto& ts = sema.type_system();
	std::vector<FunctionMember*> candidates =
	{
		ts.compare_operator_int,
		ts.compare_operator_single,
		ts.compare_operator_boolean,
		ts.compare_operator_string,
	};

	//equal_enum, // one exists for every enum type, only considered for overload resolution if one of the operands already match
	//equal_reference, // one exists for every class, special rules for when its applicable

	FunctionMember* func = ts.find_best_function(candidates, { a, b });
	if (!func)
		throw SemaException("No suitable operator found", nullptr);
	return func;
}

FunctionMember* ExpressionSema::get_not_equal_overload(const ExpressionResult& a, const ExpressionResult& b)
{
	auto& ts = sema.type_system();
	std::vector<FunctionMember*> candidates =
	{
		ts.compare_operator_int,
		ts.compare_operator_single,
		ts.compare_operator_string,
	};

	//notequal_enum, // one exists for every enum type, only considered for overload resolution if one of the operands already match
	//notequal_reference, // one exists for every class, special rules for when its applicable

	FunctionMember* func = ts.find_best_function(candidates, { a, b });
	if (!func)
		throw SemaException("No suitable operator found", nullptr);
	return func;
}

FunctionMember* ExpressionSema::get_conditional_and_overload(const ExpressionResult& a, const ExpressionResult& b)
{
	auto& ts = sema.type_system();
	std::vector<FunctionMember*> candidates =
	{
		ts.binary_operator_int,
		ts.binary_operator_boolean,
	};

	//and_enum, // one exists for every enum type, only considered for overload resolution if one of the operands already match

	FunctionMember* func = ts.find_best_function(candidates, { a, b });
	if (func != ts.binary_operator_boolean)
		throw SemaException("No suitable operator found", nullptr);
	return func;
}

FunctionMember* ExpressionSema::get_conditional_or_overload(const ExpressionResult& a, const ExpressionResult& b)
{
	auto& ts = sema.type_system();
	std::vector<FunctionMember*> candidates =
	{
		ts.binary_operator_int,
		ts.binary_operator_boolean,
	};

	//and_enum, // one exists for every enum type, only considered for overload resolution if one of the operands already match

	FunctionMember* func = ts.find_best_function(candidates, { a, b });
	if (func != ts.binary_operator_boolean)
		throw SemaException("No suitable operator found", nullptr);
	return func;
}
