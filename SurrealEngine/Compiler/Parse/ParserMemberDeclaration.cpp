
#include "Parser.h"

AstNode *Parser::parse_class_member()
{
	TypeModifierPreparseData preparse = preparse_type();

	if (is_keyword("const"))
	{
		return parse_const_declaration(preparse);
	}
	else if (is_identifier() || is_type_keyword())
	{
		AstName *type = parse_name();
		
		if (is_identifier() || is_keyword("this"))
		{
			std::string value1 = token.value;
			std::string value2;
			next();

			if (is_operator("."))
			{
				next();
				if (is_identifier() || is_keyword("this"))
				{
					value2 = token.value;
					next();
				}
				else
				{
					throw_parse_exception("identifier or this expected");
				}
			}

			if (is_operator("(")) // Method
			{
				return parse_method_declaration(preparse, type, value1, value2);
			}
			else // Field
			{
				return parse_field_declaration(preparse, type, value1, value2);
			}
		}
		else if (is_keyword("operator")) // unary and binary operators
		{
			return parse_operator_declaration(preparse, type);
		}
		else
		{
			throw_parse_exception("syntax error");
		}
	}
	else if (is_keyword("struct"))
	{
		return parse_struct_declaration(preparse);
	}
	else if (is_keyword("enum"))
	{
		return parse_enum_declaration(preparse);
	}
	else
	{
		throw_parse_exception("syntax error");
	}
}

AstNode *Parser::parse_const_declaration(const TypeModifierPreparseData &preparse)
{
	if (!is_keyword("const"))
		throw_parse_exception("const expected");
	next();

	AstName *type = parse_name();

	AstConstantDeclaration *const_decl = newNode<AstConstantDeclaration>();
	const_decl->access_type = preparse.access_type;
	const_decl->type = type;

	while (true)
	{
		if (!is_identifier())
			throw_parse_exception("identifier expected");
		std::string identifier = token.value;
		next();

		if (!is_operator("="))
			throw_parse_exception("= expected");
		next();

		AstExpression *expression = parse_expression(semicolon_end | comma_end);

		AstConstantDeclarator *declarator = newNode<AstConstantDeclarator>();
		declarator->identifier = identifier;
		declarator->expression = expression;
		const_decl->declarators.push_back(declarator);

		if (is_operator(";"))
		{
			next();
			break;
		}
		else if (is_operator(","))
		{
			next();
		}
		else
		{
			throw_parse_exception(", or ; expected");
		}
	}

	return const_decl;
}

std::vector<AstMethodParameter *> Parser::parse_formal_parameter_list(const char *end_operator)
{
	std::vector<AstMethodParameter *> parameters;

	while (!is_operator(end_operator))
	{
		if (is_keyword("params"))
		{
			next();

			AstMethodParameterArray *params = newNode<AstMethodParameterArray>();
			params->array_type = parse_name();

			if (!is_identifier())
				throw_parse_exception("identifier expected");
			params->identifier = token.value;
			next();

			if (!is_operator(")"))
				throw_parse_exception(") expected");

			parameters.push_back(params);
		}
		else
		{
			AstMethodFixedParameter *parameter = newNode<AstMethodFixedParameter>();

			if (is_keyword("ref"))
			{
				parameter->is_ref = true;
				next();
			}
			else if (is_keyword("out"))
			{
				parameter->is_out = true;
				next();
			}

			parameter->type = parse_name();

			if (is_identifier())
			{
				parameter->identifier = token.value;
				next();
			}

			parameters.push_back(parameter);
		}

		if (!is_operator(end_operator))
		{
			if (!is_operator(","))
				throw_parse_exception(", expected");
			next();
		}
	}
	next();

	return parameters;
}

AstNode *Parser::parse_method_declaration(const TypeModifierPreparseData &preparse, AstName *return_type, const std::string &identifier1, const std::string &identifier2)
{
	std::string member_name = identifier1;

	AstMethodDeclaration *method_decl = newNode<AstMethodDeclaration>();
	method_decl->is_static = preparse.is_static;
	method_decl->is_event = preparse.is_event;
	method_decl->return_type = return_type;
	method_decl->identifier = member_name;

	if (!is_operator("("))
		throw_parse_exception("( expected");
	next();

	method_decl->parameters = parse_formal_parameter_list(")");

	// type-parameter-constraints-clauses <- generics

	if (is_operator("{"))
	{
		method_decl->block = parse_block_statement();
	}
	else if (is_operator(";"))
	{
		next();
	}
	else
	{
		throw_parse_exception("{ or ; expected");
	}

	return method_decl;
}

AstNode *Parser::parse_field_declaration(const TypeModifierPreparseData &preparse, AstName *type, const std::string &identifier1, const std::string &identifier2)
{
	AstFieldDeclaration *field_decl = newNode<AstFieldDeclaration>();
	field_decl->is_static = preparse.is_static;
	field_decl->is_const = preparse.is_const;
	field_decl->access_type = preparse.access_type;
	field_decl->type = type;

	AstVariableDeclarator *var_decl = newNode<AstVariableDeclarator>();
	var_decl->identifier = identifier1;
	field_decl->declarators.push_back(var_decl);

	while (true)
	{
		if (is_operator("="))
		{
			next();
			if (is_operator("{"))
			{
				var_decl->array_initializer = parse_array_initializer();
			}
			else
			{
				var_decl->expression = parse_expression(semicolon_end|comma_end);
			}
		}

		if (is_operator(";"))
		{
			next();
			break;
		}
		else if (is_operator(","))
		{
			next();
						
			if (!is_identifier())
				throw_parse_exception("identifier expected");

			var_decl = newNode<AstVariableDeclarator>();
			var_decl->identifier = token.value;
			field_decl->declarators.push_back(var_decl);

			next();
		}
		else
		{
			throw_parse_exception(", or ; expected");
		}
	}

	return field_decl;
}

AstNode *Parser::parse_operator_declaration(const TypeModifierPreparseData &preparse, AstName *return_type)
{
	if (!is_keyword("operator"))
		throw_parse_exception("operator expected");
	next();

	if (token.type != Token::type_operator && token.type != Token::type_bool)
		throw_parse_exception("operator, true or false expected");

	std::string operator_type = token.value;
	if (is_operator(">"))
	{
		next();
		if (is_operator(">"))
		{
			operator_type = ">>";
			next();
		}
	}
	else
	{
		next();
	}

	if (!is_operator("("))
		throw_parse_exception("( expected");
	next();

	AstName *param_type1 = parse_name();
	AstName *param_name1 = parse_name();
	AstName *param_type2 = 0;
	AstName *param_name2 = 0;

	if (is_operator(","))
	{
		next();
		param_type2 = parse_name();
		param_name2 = parse_name();
	}

	if (!is_operator(")"))
		throw_parse_exception(") expected");
	next();

	AstOperatorDeclaration *operator_decl = newNode<AstOperatorDeclaration>();

	if (is_operator("{"))
	{
		operator_decl->block = parse_block_statement();
	}
	else if (!is_operator(";"))
	{
		throw_parse_exception("{ or ; expected");
	}

	return operator_decl;
}

Parser::TypeModifierPreparseData Parser::preparse_type()
{
	TypeModifierPreparseData preparse;
	bool is_access_type_set = false;
	while (true)
	{
		if (is_keyword("public"))
		{
			if (is_access_type_set)
				throw_parse_exception("public already specified");
			is_access_type_set = true;
			preparse.access_type = access_public;
		}
		else if (is_keyword("private"))
		{
			if (is_access_type_set)
				throw_parse_exception("private already specified");
			is_access_type_set = true;
			preparse.access_type = access_private;
		}
		else if (is_keyword("abstract"))
		{
			if (preparse.is_abstract)
				throw_parse_exception("abstract already specified");
			preparse.is_abstract = true;
		}
		else if (is_keyword("static"))
		{
			if (preparse.is_static)
				throw_parse_exception("static already specified");
			preparse.is_static = true;
		}
		else if (is_keyword("const"))
		{
			if (preparse.is_const)
				throw_parse_exception("const already specified");
			preparse.is_const = true;
		}
		else if (is_keyword("event"))
		{
			if (preparse.is_event)
				throw_parse_exception("event already specified");
			preparse.is_event = true;
		}
		else
		{
			break;
		}

		next();
	}

	return preparse;
}
