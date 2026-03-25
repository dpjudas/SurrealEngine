
#include "Parser.h"

AstNode *Parser::parse_class_member()
{
	if (is_keyword("var") || is_keyword("enum") || is_keyword("struct"))
	{
		return parse_field_declaration();
	}
	else if (is_keyword("state") || is_keyword("auto"))
	{
		return parse_state_declaration();
	}
	else if (is_keyword("const"))
	{
		return parse_const_declaration();
	}
	else if (is_keyword("replication"))
	{
		return parse_replication_declaration();
	}
	else
	{
		return parse_method_declaration();
	}
}

AstNode* Parser::parse_state_declaration()
{
	bool is_auto = false;
	if (is_keyword("auto"))
	{
		is_auto = true;
		next();
	}

	if (!is_keyword("state"))
		throw_parse_exception("state expected");
	next();

	AstStateDeclaration* state_decl = newNode<AstStateDeclaration>();

	if (is_operator("("))
	{
		next();
		if (!is_operator(")"))
			throw_parse_exception(") expected");
		next();
	}

	state_decl->is_auto = is_auto;

	if (!is_identifier())
		throw_parse_exception("identifier expected");
	state_decl->identifier = token.value;
	next();

	if (is_keyword("extends"))
	{
		next();
		state_decl->base_state = parse_identifier_name();
	}

	if (is_operator("{"))
	{
		state_decl->block = parse_block_statement(true);
	}
	else if (is_operator(";"))
	{
		next();
	}
	else
	{
		throw_parse_exception("{ or ; expected");
	}

	return state_decl;
}

AstNode *Parser::parse_const_declaration()
{
	if (!is_keyword("const"))
		throw_parse_exception("const expected");
	next();

	AstConstantDeclaration *const_decl = newNode<AstConstantDeclaration>();

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
		AstMethodFixedParameter *parameter = newNode<AstMethodFixedParameter>();

		while (true)
		{
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
			else if (is_keyword("optional"))
			{
				parameter->is_optional = true;
				next();
			}
			else if (is_keyword("coerce"))
			{
				parameter->is_coerce = true;
				next();
			}
			else
			{
				break;
			}
		}

		parameter->type = parse_name();

		if (is_identifier())
		{
			parameter->identifier = token.value;
			next();
		}

		if (is_operator("["))
		{
			next();

			if (token.type != Token::type_integer)
				throw_parse_exception("integer expected");
			parameter->array_dimension = std::stoi(token.value);
			next();

			if (!is_operator("]"))
				throw_parse_exception("] expected");
			next();
		}

		parameters.push_back(parameter);

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

AstMethodDeclaration* Parser::parse_method_declaration()
{
	AstMethodDeclaration* method_decl = newNode<AstMethodDeclaration>();

	bool is_access_type_set = false;
	while (true)
	{
		if (is_keyword("public"))
		{
			if (is_access_type_set)
				throw_parse_exception("public already specified");
			next();
			is_access_type_set = true;
			method_decl->access_type = access_public;
		}
		else if (is_keyword("private"))
		{
			if (is_access_type_set)
				throw_parse_exception("private already specified");
			next();
			is_access_type_set = true;
			method_decl->access_type = access_private;
		}
		else if (is_keyword("singular"))
		{
			if (method_decl->is_singular)
				throw_parse_exception("singular already specified");
			next();
			method_decl->is_singular = true;
		}
		else if (is_keyword("simulated"))
		{
			if (method_decl->is_simulated)
				throw_parse_exception("simulated already specified");
			next();
			method_decl->is_simulated = true;
		}
		else if (is_keyword("native") || is_keyword("intrinsic"))
		{
			if (method_decl->is_native)
				throw_parse_exception("intrinsic already specified");
			next();
			method_decl->is_native = true;

			if (is_operator("("))
			{
				next();

				if (token.type == Token::type_integer)
				{
					method_decl->native_index = std::stoi(token.value);
					next();
				}

				if (!is_operator(")"))
					throw_parse_exception(") specified");
				next();
			}
		}
		else if (is_keyword("final"))
		{
			if (method_decl->is_final)
				throw_parse_exception("final already specified");
			next();
			method_decl->is_final = true;
		}
		else if (is_keyword("iterator"))
		{
			if (method_decl->is_iterator)
				throw_parse_exception("iterator already specified");
			next();
			method_decl->is_iterator = true;
		}
		else if (is_keyword("static"))
		{
			if (method_decl->is_static)
				throw_parse_exception("static already specified");
			next();
			method_decl->is_static = true;
		}
		else
		{
			break;
		}
	}

	if (is_keyword("event"))
	{
		method_decl->is_event = true;
		next();
	}
	else if (is_keyword("function"))
	{
		next();
	}
	/*else if (is_keyword("operator"))
	{
		method_decl->is_operator = true;
		next();
	}*/
	else
	{
		throw_parse_exception("function, event or operator expected");
	}

	bool returnValueFound = false;
	SavedParserPos pos = save_position();
	if (is_keyword("class") || is_type_keyword())
	{
		returnValueFound = true;
	}
	else if (is_identifier())
	{
		next();
		if (is_identifier())
			returnValueFound = true;
		restore_position(pos);
	}

	if (returnValueFound)
	{
		method_decl->return_type = parse_name();
	}

	if (!is_identifier())
		throw_parse_exception("identifier expected");
	method_decl->identifier = token.value;
	next();

	if (!is_operator("("))
		throw_parse_exception("( expected");
	next();

	method_decl->parameters = parse_formal_parameter_list(")");

	if (is_operator("{"))
	{
		method_decl->block = parse_block_statement(false);
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

AstNode *Parser::parse_field_declaration()
{
	std::string group;
	if (is_keyword("var"))
	{
		next();
		if (is_operator("("))
		{
			next();

			if (is_identifier())
			{
				group = token.value;
				next();
			}

			if (!is_operator(")"))
				throw_parse_exception(") expected");
			next();
		}
	}
	else if (!is_keyword("enum") && !is_keyword("struct"))
	{
		throw_parse_exception("var, struct or enum expected");
	}

	AstFieldDeclaration *field_decl = newNode<AstFieldDeclaration>();
	field_decl->group = group;

	bool is_access_type_set = false;
	while (true)
	{
		if (is_keyword("public"))
		{
			if (is_access_type_set)
				throw_parse_exception("public already specified");
			is_access_type_set = true;
			field_decl->access_type = access_public;
		}
		else if (is_keyword("private"))
		{
			if (is_access_type_set)
				throw_parse_exception("private already specified");
			is_access_type_set = true;
			field_decl->access_type = access_private;
		}
		else if (is_keyword("const"))
		{
			if (field_decl->is_const)
				throw_parse_exception("const already specified");
			field_decl->is_const = true;
		}
		else if (is_keyword("localized"))
		{
			if (field_decl->is_localized)
				throw_parse_exception("localized already specified");
			field_decl->is_localized = true;
		}
		else if (is_keyword("native") || is_keyword("intrinsic"))
		{
			if (field_decl->is_native)
				throw_parse_exception("intrinsic already specified");
			field_decl->is_native = true;
		}
		else if (is_keyword("travel"))
		{
			if (field_decl->is_travel)
				throw_parse_exception("travel already specified");
			field_decl->is_travel = true;
		}
		else if (is_keyword("transient"))
		{
			if (field_decl->is_transient)
				throw_parse_exception("transient already specified");
			field_decl->is_transient = true;
		}
		else if (is_keyword("editconst"))
		{
			if (field_decl->editconst)
				throw_parse_exception("editconst already specified");
			field_decl->editconst = true;
		}
		else
		{
			break;
		}
		next();
	}

	if (is_keyword("enum"))
	{
		field_decl->enum_decl = parse_enum_declaration();
	}
	else if (is_keyword("struct"))
	{
		field_decl->struct_decl = parse_struct_declaration();
	}
	else
	{
		field_decl->type = parse_name();
	}

	AstVariableDeclarator *var_decl = newNode<AstVariableDeclarator>();

	if (!is_identifier())
		throw_parse_exception("identifier expected");
	var_decl->identifier = token.value;
	next();

	field_decl->declarators.push_back(var_decl);

	while (true)
	{
		if (is_operator("["))
		{
			next();

			if (token.type != Token::type_integer)
				throw_parse_exception("integer expected");

			var_decl->array_dimension = std::stoi(token.value);
			next();

			if (!is_operator("]"))
				throw_parse_exception("] expected");
			next();
		}

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

AstNode* Parser::parse_replication_declaration()
{
	if (!is_keyword("replication"))
		throw_parse_exception("replication expected");
	next();

	AstReplicationDeclaration* repl_decl = newNode<AstReplicationDeclaration>();

	if (!is_operator("{"))
		throw_parse_exception("{ expected");
	next();

	while (!is_operator("}"))
	{
		bool if_reliable = is_keyword("reliable");
		bool if_unreliable = is_keyword("unreliable");
		if (if_reliable || if_unreliable)
		{
			next();

			if (!is_keyword("if"))
				throw_parse_exception("if expected");
			next();

			AstReplicationRule* rule = newNode<AstReplicationRule>();

			rule->if_reliable = if_reliable;
			rule->if_unreliable = if_unreliable;

			if (!is_operator("("))
				throw_parse_exception("( expected");
			next();

			rule->boolean_expression = parse_expression(paranthesis_end);

			if (!is_operator(")"))
				throw_parse_exception(") expected");
			next();

			while (true)
			{
				if (!is_identifier())
					throw_parse_exception("identifier expected");
				rule->properties.push_back(token.value);
				next();

				if (is_operator(";"))
					break;
				else if (!is_operator(","))
					throw_parse_exception(", or ; expected");
				next();
			}
			next();

			repl_decl->rules.push_back(rule);
		}
		else
		{
			throw_parse_exception("reliable or unreliable expected");
		}
	}
	next();

	return repl_decl;
}
