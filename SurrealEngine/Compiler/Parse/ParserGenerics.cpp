
#include "Precomp.h"
#include "Parser.h"

AstTypeParameterList *Parser::try_parse_type_parameter_list()
{
	if (!is_operator("<"))
		return 0;

	auto save = save_position();
	next();

	AstTypeParameterList *parameter_list = newNode<AstTypeParameterList>();

	while (true)
	{
		AstTypeParameter *parameter = newNode<AstTypeParameter>();

		if (!is_type_keyword() && !is_identifier())
		{
			restore_position(save);
			return 0;
		}

		parameter->name = parse_name();

		parameter_list->parameters.push_back(parameter);

		if (is_operator(","))
		{
			next();
		}
		else if (is_operator(">"))
		{
			next();
			break;
		}
		else
		{
			restore_position(save);
			return 0;
		}
	}

	return parameter_list;
}
