
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
	else if (is_keyword("class"))
	{
		AstClassName* type = newNode<AstClassName>();
		next();

		if (is_operator("<"))
		{
			next();

			if (is_identifier())
			{
				type->name = token.value;
				next();

				if (!is_operator(">"))
					throw_parse_exception("> expected");
			}
			else if (!is_operator(">"))
			{
				throw_parse_exception("identifier or > expected");
			}

			next();
		}

		name = type;
	}
	else if (is_object_name())
	{
		AstObjectName* objname = newNode<AstObjectName>();
		objname->name = token.value;
		next();

		name = objname;
	}
	else
	{
		name = parse_identifier_name();
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
