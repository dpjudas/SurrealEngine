
#include "Precomp.h"
#include "Parser.h"

AstArrayInitializer *Parser::parse_array_initializer()
{
	if (!is_operator("{"))
		throw_parse_exception("{ expected");
	next();

	AstArrayInitializer *initializer = newNode<AstArrayInitializer>();
	set_line_info(initializer);

	if (!is_operator("}"))
	{
		next();
		while (true)
		{
			AstArrayVariableInitializer *variable_initializer = newNode<AstArrayVariableInitializer>();
			set_line_info(variable_initializer);

			if (is_operator("{"))
			{
				variable_initializer->array_initializer = parse_array_initializer();
			}
			else
			{
				variable_initializer->expression = parse_expression(comma_end|brace_end);
			}

			initializer->variable_initializer_list.push_back(variable_initializer);

			if (is_operator(","))
				next();
			else if (!is_operator("}"))
				throw_parse_exception(", or } expected");

			if (is_operator("}"))
				break;
		}
	}
	next();

	return initializer;
}
