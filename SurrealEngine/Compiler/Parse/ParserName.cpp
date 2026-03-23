
#include "Parser.h"

AstName *Parser::parse_name()
{
	AstName *name = 0;

	if (is_type_keyword())
	{
		AstKeywordType *type = newNode<AstKeywordType>();
		type->type = token.value;
		next();

		name = type;
	}
	else
	{
		name = parse_identifier_name();
	}

	while (is_operator("["))
	{
		auto save = save_position();
		next();
		if (!is_operator("]") && !is_operator(","))
		{
			restore_position(save);
			break;
		}

		int rank = 1;
		while (is_operator(","))
		{
			rank++;
			next();
		}

		if (!is_operator("]"))
			throw_parse_exception("] expected");
		next();

		name->array_rank_specifiers.push_back(rank);
	}

	return name;
}

AstIdentifierName *Parser::parse_identifier_name()
{
	AstIdentifierName *current_name = 0;
	while (true)
	{
		if (!is_identifier())
			throw_parse_exception("identifier expected");

		AstIdentifierName *next_name = newNode<AstIdentifierName>();
		next_name->name = token.value;
		next_name->prev_name = current_name;

		current_name = next_name;

		next();

		current_name->parameter_list = try_parse_type_parameter_list();

		if (!is_operator("."))
			break;
		next();
	}
	return current_name;
}
