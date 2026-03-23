
#include "Parser.h"

AstStatement *Parser::parse_statement()
{
	if (is_operator("{"))
	{
		return parse_block_statement();
	}
	else if (is_operator(";"))
	{
		next();
		return newNode<AstEmptyStatement>();
	}
	else if (is_keyword("const"))
	{
		return parse_constant_declaration_statement();
	}
	else if (is_keyword("if"))
	{
		return parse_if_statement();
	}
	else if (is_keyword("switch"))
	{
		return parse_switch_statement();
	}
	else if (is_keyword("while"))
	{
		return parse_while_statement();
	}
	else if (is_keyword("do"))
	{
		return parse_do_statement();
	}
	else if (is_keyword("for"))
	{
		return parse_for_statement();
	}
	else if (is_keyword("foreach"))
	{
		return parse_foreach_statement();
	}
	else if (is_keyword("break"))
	{
		return parse_break_statement();
	}
	else if (is_keyword("continue"))
	{
		return parse_continue_statement();
	}
	else if (is_keyword("goto"))
	{
		return parse_goto_statement();
	}
	else if (is_keyword("return"))
	{
		return parse_return_statement();
	}
	else
	{
		auto save = save_position();

		if (is_identifier())
		{
			std::string identifier = token.value;
			next();
			if (is_operator(":"))
			{
				next();

				AstLabeledStatement *labeled = newNode<AstLabeledStatement>();
				labeled->identifier = identifier;
				labeled->statement = parse_statement();
				return labeled;
			}
		}

		restore_position(save);

		if (is_identifier() || is_type_keyword())
		{
			AstName *type = parse_name();
			if (is_identifier())
			{
				return parse_variable_declaration_statement(type);
			}
		}

		restore_position(save);

		return parse_expression_statement();
	}
}

AstBlockStatement *Parser::parse_block_statement()
{
	if (!is_operator("{"))
		throw_parse_exception("{ expected");
	next();

	AstBlockStatement *block = newNode<AstBlockStatement>();

	while (!is_operator("}"))
	{
		block->statements.push_back(parse_statement());
	}

	next();

	return block;
}

AstConstantDeclarationStatement *Parser::parse_constant_declaration_statement()
{
	if (!is_keyword("const"))
		throw_parse_exception("const expected");
	next();

	AstConstantDeclarationStatement *statement = newNode<AstConstantDeclarationStatement>();

	statement->type = parse_name();

	while (true)
	{
		if (!is_identifier())
			throw_parse_exception("identifier expected");

		AstLocalConstantDeclarator *decl = newNode<AstLocalConstantDeclarator>();

		decl->identifier = token.value;
		next();

		if (is_operator("="))
		{
			decl->expression = parse_expression(comma_end | semicolon_end);
		}

		statement->variables.push_back(decl);

		if (!is_operator(","))
			break;
		next();
	}

	if (!is_operator(";"))
		throw_parse_exception("; expected");
	next();

	return statement;
}

AstVariableDeclarationStatement *Parser::parse_variable_declaration_statement(AstName *type)
{
	AstVariableDeclarationStatement *statement = newNode<AstVariableDeclarationStatement>();

	statement->variable_declaration = parse_local_variable_declaration(type);

	if (!is_operator(";"))
		throw_parse_exception("; expected");
	next();

	return statement;
}

AstIfStatement *Parser::parse_if_statement()
{
	if (!is_keyword("if"))
		throw_parse_exception("if expected");
	next();

	AstIfStatement *statement = newNode<AstIfStatement>();

	if (!is_operator("("))
		throw_parse_exception("( expected");
	next();

	statement->boolean_expression = parse_expression(paranthesis_end);

	if (!is_operator(")"))
		throw_parse_exception(") expected");
	next();

	statement->then_statement = parse_statement();

	if (is_keyword("else"))
	{
		next();
		statement->else_statement = parse_statement();
	}

	return statement;
}

AstSwitchStatement *Parser::parse_switch_statement()
{
	if (!is_keyword("switch"))
		throw_parse_exception("switch expected");
	next();

	AstSwitchStatement *statement = newNode<AstSwitchStatement>();

	if (!is_operator("("))
		throw_parse_exception("( expected");
	next();

	statement->expression = parse_expression(paranthesis_end);

	if (!is_operator(")"))
		throw_parse_exception(") expected");
	next();

	if (!is_operator("{"))
		throw_parse_exception("{ expected");
	next();

	AstSwitchSection *section = nullptr;

	while (!is_operator("}"))
	{
		AstSwitchLabel *label = nullptr;
		if (is_keyword("case"))
		{
			next();
			label = newNode<AstSwitchLabel>();
			label->constant_expression = parse_expression(colon_end);
			if (!is_operator(":"))
				throw_parse_exception(": expected");
			next();
		}
		else if (is_keyword("default"))
		{
			next();
			if (!is_operator(":"))
				throw_parse_exception(": expected");
			next();
			label = newNode<AstSwitchLabel>();
		}

		if (label)
		{
			if (section && !section->statements.empty())
			{
				statement->sections.push_back(section);
				section = nullptr;
			}

			if (!section)
				section = newNode<AstSwitchSection>();
		}
		else if (!section)
		{
			throw_parse_exception("expected case or default");
		}
		else
		{
			section->statements.push_back(parse_statement());
		}
	}
	next();

	if (section)
		statement->sections.push_back(section);

	return statement;
}

AstWhileStatement *Parser::parse_while_statement()
{
	if (!is_keyword("while"))
		throw_parse_exception("while expected");
	next();

	AstWhileStatement *statement = newNode<AstWhileStatement>();

	if (!is_operator("("))
		throw_parse_exception("( expected");
	next();

	statement->boolean_expression = parse_expression(paranthesis_end);

	if (!is_operator(")"))
		throw_parse_exception(") expected");
	next();

	statement->statement = parse_statement();

	return statement;
}

AstDoStatement *Parser::parse_do_statement()
{
	if (!is_keyword("do"))
		throw_parse_exception("do expected");
	next();

	AstDoStatement *statement = newNode<AstDoStatement>();

	statement->statement = parse_statement();

	if (!is_keyword("while"))
		throw_parse_exception("while expected");
	next();

	if (!is_operator("("))
		throw_parse_exception("( expected");
	next();

	statement->boolean_expression = parse_expression(paranthesis_end);

	if (!is_operator(")"))
		throw_parse_exception(") expected");
	next();

	if (!is_operator(";"))
		throw_parse_exception("; expected");
	next();

	return statement;
}

AstForStatement *Parser::parse_for_statement()
{
	if (!is_keyword("for"))
		throw_parse_exception("for expected");
	next();

	AstForStatement *statement = newNode<AstForStatement>();

	if (!is_operator("("))
		throw_parse_exception("( expected");
	next();

	if (!is_operator(";"))
	{
		if (is_identifier() || is_type_keyword())
		{
			auto save = save_position();
			AstName *type = parse_name();
			if (is_identifier())
			{
				statement->init_variable_declaration = parse_local_variable_declaration(type);

				if (!is_operator(";"))
					throw_parse_exception("; expected");
			}
			else
			{
				restore_position(save);
			}
		}

		if (!statement->init_variable_declaration)
		{
			while (true)
			{
				statement->init_expressions.push_back(parse_expression(comma_end|semicolon_end));

				if (is_operator(";"))
				{
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
		}
	}
	next();

	if (!is_operator(";"))
	{
		statement->condition = parse_expression(semicolon_end);
		if (!is_operator(";"))
			throw_parse_exception("; expected");
	}
	next();

	if (!is_operator(")"))
	{
		while (true)
		{
			statement->iterator_expressions.push_back(parse_expression(comma_end|paranthesis_end));

			if (is_operator(")"))
			{
				break;
			}
			else if (is_operator(","))
			{
				next();
			}
			else
			{
				throw_parse_exception(", or ) expected");
			}
		}
	}
	next();

	statement->statement = parse_statement();

	return statement;
}

AstForeachStatement *Parser::parse_foreach_statement()
{
	if (!is_keyword("foreach"))
		throw_parse_exception("foreach expected");
	next();

	AstForeachStatement *statement = newNode<AstForeachStatement>();

	if (!is_operator("("))
		throw_parse_exception("( expected");
	next();

	statement->type = parse_name();

	if (!is_identifier())
		throw_parse_exception("identifier expected");
	statement->identifier = token.value;
	next();

	if (!is_keyword("in"))
		throw_parse_exception("in expected");
	next();

	statement->expression = parse_expression(paranthesis_end);

	if (!is_operator(")"))
		throw_parse_exception(") expected");
	next();

	statement->statement = parse_statement();

	return statement;
}

AstBreakStatement *Parser::parse_break_statement()
{
	if (!is_keyword("break"))
		throw_parse_exception("break expected");
	next();

	if (!is_operator(";"))
		throw_parse_exception("; expected");
	next();

	AstBreakStatement *statement = newNode<AstBreakStatement>();
	return statement;
}

AstContinueStatement *Parser::parse_continue_statement()
{
	if (!is_keyword("continue"))
		throw_parse_exception("continue expected");
	next();

	if (!is_operator(";"))
		throw_parse_exception("; expected");
	next();

	AstContinueStatement *statement = newNode<AstContinueStatement>();
	return statement;
}

AstGotoStatement *Parser::parse_goto_statement()
{
	if (!is_keyword("goto"))
		throw_parse_exception("goto expected");
	next();

	AstGotoStatement *statement = newNode<AstGotoStatement>();

	if (is_keyword("default"))
	{
		next();
	}
	else if (is_keyword("case"))
	{
		next();
		statement->expression = parse_expression(semicolon_end);
	}
	else if (is_identifier())
	{
		statement->identifier = token.value;
		next();
	}
	else
	{
		throw_parse_exception("default, case or identifier expected");
	}

	if (!is_operator(";"))
		throw_parse_exception("; expected");
	next();

	return statement;
}

AstReturnStatement *Parser::parse_return_statement()
{
	if (!is_keyword("return"))
		throw_parse_exception("return expected");
	next();

	AstReturnStatement *statement = newNode<AstReturnStatement>();

	if (!is_operator(";"))
	{
		statement->expression = parse_expression(semicolon_end);

		if (!is_operator(";"))
			throw_parse_exception("; expected");
	}
	next();

	return statement;
}

AstExpressionStatement *Parser::parse_expression_statement()
{
	AstExpressionStatement *statement = newNode<AstExpressionStatement>();
	statement->expression = parse_expression(semicolon_end);

	if (!is_operator(";"))
		throw_parse_exception("; expected");
	next();

	return statement;
}

AstLocalVariableDeclaration *Parser::parse_local_variable_declaration(AstName *type)
{
	AstLocalVariableDeclaration *variables = newNode<AstLocalVariableDeclaration>();
	variables->type = type;

	while (true)
	{
		if (!is_identifier())
			throw_parse_exception("identifier expected");

		AstLocalVariableDeclarator *decl = newNode<AstLocalVariableDeclarator>();

		decl->identifier = token.value;
		next();

		if (is_operator("="))
		{
			next();

			if (is_operator("{"))
			{
				decl->array_initializer = parse_array_initializer();
			}
			else
			{
				decl->expression = parse_expression(comma_end | semicolon_end);
			}
		}

		variables->variables.push_back(decl);

		if (!is_operator(","))
			break;
		next();
	}

	return variables;
}
