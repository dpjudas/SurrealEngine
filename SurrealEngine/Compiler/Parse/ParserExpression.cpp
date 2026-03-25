
#include "Parser.h"

AstExpression *Parser::parse_expression(ExpressionEndCondition end_condition)
{
	// To do: add support for the trinary select (?:) operator
	AstExpression *lhs = parse_unary();
	return parse_binary(0, lhs, end_condition);
}

AstExpression *Parser::parse_primary()
{
	AstExpression *expression = 0;

	if (is_keyword("super"))
	{
		next();

		AstBaseAccess *base_access = newNode<AstBaseAccess>();
		expression = base_access;
	}
	else if (is_keyword("new"))
	{
		next();

		AstNewExpression* new_expression = newNode<AstNewExpression>();
		if (is_operator("("))
		{
			next();
			
			while (!is_operator(")"))
			{
				if (!is_operator(",") && !is_operator(")"))
					new_expression->args.push_back(parse_expression(comma_end | paranthesis_end));

				if (is_operator(","))
					next();
				else if (!is_operator(")"))
					throw_parse_exception(", or ) expected");
			}
			next();
		}

		AstName* type = parse_name();
		new_expression->type = type;
		expression = new_expression;
	}
	else if (is_keyword("typeof"))
	{
		next();

		if (!is_operator("("))
			throw_parse_exception("( expected");
		next();

		AstTypeofExpression *typeof_expression = newNode<AstTypeofExpression>();
		typeof_expression->type = parse_name();

		if (!is_operator(")"))
			throw_parse_exception(") expected");
		next();

		expression = typeof_expression;
	}
	else if (is_keyword("sizeof"))
	{
		next();

		if (!is_operator("("))
			throw_parse_exception("( expected");
		next();

		AstSizeofExpression *sizeof_expression = newNode<AstSizeofExpression>();
		sizeof_expression->type = parse_name();

		if (!is_operator(")"))
			throw_parse_exception(") expected");
		next();

		expression = sizeof_expression;
	}
	else if (is_identifier())
	{
		std::string identifier = token.value;
		next();

		if (is_name())
		{
			std::string obj_name = token.value;
			next();

			AstNamedObject* name = newNode<AstNamedObject>();
			name->class_name = identifier;
			name->object_name = obj_name;
			expression = name;
		}
		else
		{
			AstSimpleName* name = newNode<AstSimpleName>();
			name->identifier = identifier;
			expression = name;
		}
	}
	else if (token.type == Token::type_bool)
	{
		AstLiteral *literal = newNode<AstLiteral>();
		literal->type = AstLiteralType::boolean;
		literal->value = token.value;
		expression = literal;
		next();
	}
	else if (token.type == Token::type_integer)
	{
		AstLiteral *literal = newNode<AstLiteral>();
		literal->type = AstLiteralType::integer;
		literal->value = token.value;
		expression = literal;
		next();
	}
	else if (token.type == Token::type_real)
	{
		AstLiteral *literal = newNode<AstLiteral>();
		literal->type = AstLiteralType::real;
		literal->value = token.value;
		expression = literal;
		next();
	}
	else if (token.type == Token::type_name)
	{
		AstLiteral *literal = newNode<AstLiteral>();
		literal->type = AstLiteralType::name;
		literal->value = token.value;
		expression = literal;
		next();
	}
	else if (token.type == Token::type_string)
	{
		AstLiteral *literal = newNode<AstLiteral>();
		literal->type = AstLiteralType::string;
		literal->value = token.value;
		expression = literal;
		next();
	}
	else if (token.type == Token::type_none)
	{
		AstLiteral *literal = newNode<AstLiteral>();
		literal->type = AstLiteralType::none;
		literal->value = "none";
		expression = literal;
		next();
	}
	else if (is_operator("("))
	{
		next();

		AstParenthesizedExpression *parenthesis = newNode<AstParenthesizedExpression>();
		parenthesis->expression = parse_expression(paranthesis_end);
		expression = parenthesis;

		if (!is_operator(")"))
			throw_parse_exception(") expected");
		next();
	}
	else
	{
		throw_parse_exception("syntax error");
	}

	while (true)
	{
		if (is_operator("["))
		{
			next();

			AstElementAccess *element_access = newNode<AstElementAccess>();
			element_access->expression = expression;

			while (!is_operator("]"))
			{
				element_access->args.push_back(parse_expression(bracket_end|comma_end));

				if (is_operator(","))
					next();
				else if (!is_operator("]"))
					throw_parse_exception(", or ] expected");
			}
			next();

			expression = element_access;
		}
		else if (is_operator("++"))
		{
			next();

			AstPostIncrementExpression *incr = newNode<AstPostIncrementExpression>();
			incr->expression = expression;
			expression = incr;
		}
		else if (is_operator("--"))
		{
			next();

			AstPostDecrementExpression *decr = newNode<AstPostDecrementExpression>();
			decr->expression = expression;
			expression = decr;
		}
		else if (is_operator("."))
		{
			next();

			if (!is_identifier())
				throw_parse_exception("identifier expected");

			std::string identifier = token.value;
			next();

			AstMemberAccess *access = newNode<AstMemberAccess>();
			access->expression = expression;
			access->identifier = identifier;

			expression = access;
		}
		else if (is_operator("("))
		{
			next();

			AstInvocationExpression *invocation = newNode<AstInvocationExpression>();
			invocation->expression = expression;

			while (!is_operator(")"))
			{
				AstInvocationArgument *arg = newNode<AstInvocationArgument>();

				if (!is_operator(",") && !is_operator(")"))
				{
					arg->expression = parse_expression(comma_end | paranthesis_end);
				}

				invocation->args.push_back(arg);

				if (is_operator(","))
					next();
				else if (!is_operator(")"))
					throw_parse_exception(", or ) expected");
			}
			next();

			expression = invocation;
		}
		else
		{
			break;
		}
	}

	return expression;
}

AstExpression *Parser::parse_unary()
{
	if (is_operator("+"))
	{
		next();

		AstUnaryPlusExpression *expression = newNode<AstUnaryPlusExpression>();
		expression->operand = parse_unary();
		return expression;
	}
	else if (is_operator("-"))
	{
		next();

		AstUnaryMinusExpression *expression = newNode<AstUnaryMinusExpression>();
		expression->operand = parse_unary();
		return expression;
	}
	else if (is_operator("!"))
	{
		next();

		AstUnaryLogicalNotExpression *expression = newNode<AstUnaryLogicalNotExpression>();
		expression->operand = parse_unary();
		return expression;
	}
	else if (is_operator("~"))
	{
		next();

		AstUnaryBitwiseComplementExpression *expression = newNode<AstUnaryBitwiseComplementExpression>();
		expression->operand = parse_unary();
		return expression;
	}
	else if (is_operator("++"))
	{
		next();

		AstUnaryPreIncrementExpression *expression = newNode<AstUnaryPreIncrementExpression>();
		expression->operand = parse_unary();
		return expression;
	}
	else if (is_operator("--"))
	{
		next();

		AstUnaryPreDecrementExpression *expression = newNode<AstUnaryPreDecrementExpression>();
		expression->operand = parse_unary();
		return expression;
	}
	else
	{
		return parse_primary();
	}
}

AstExpression *Parser::parse_binary(int lhs_precedence, AstExpression *lhs, ExpressionEndCondition end_condition)
{
	while (!is_end_condition(end_condition))
	{
		int binary_precedence = get_token_precedence();
		if (binary_precedence < lhs_precedence)
			break;

		AstBinaryExpression *expression = create_token_expression();
		next();

		AstExpression *rhs = parse_unary();

		int rhs_precedence = get_token_precedence();
		if (binary_precedence < rhs_precedence)
			rhs = parse_binary(binary_precedence + 1, rhs, end_condition);

		expression->operand1 = lhs;
		expression->operand2 = rhs;
		lhs = expression;
	}
	return lhs;
}

int Parser::get_token_precedence()
{
	if (is_operator("*") || is_operator("/") || is_operator("%"))
	{
		return 13; // highest
	}
	else if (is_operator("+") || is_operator("-") || is_operator("$") || is_keyword("dot") || is_keyword("cross"))
	{
		return 12;
	}
	else if (is_operator("<<") || is_operator(">>")) // To do: operator >> is split into two > tokens
	{
		return 11;
	}
	else if (is_operator("<") || is_operator(">") || is_operator("<=") || is_operator(">=") || is_keyword("is") || is_keyword("as"))
	{
		return 10;
	}
	else if (is_operator("==") || is_operator("!=") || is_operator("~="))
	{
		return 9;
	}
	else if (is_operator("&"))
	{
		return 8;
	}
	else if (is_operator("^"))
	{
		return 7;
	}
	else if (is_operator("|"))
	{
		return 6;
	}
	else if (is_operator("&&"))
	{
		return 5;
	}
	else if (is_operator("||"))
	{
		return 4;
	}
	else if (is_operator("?"))
	{
		return 2;
	}
	else if (is_operator("=") || is_operator("*=") || is_operator("/=") || is_operator("%=") || is_operator("+=") || is_operator("-=") || is_operator("<<=") || is_operator(">>=") ||
		is_operator("&=") || is_operator("^=") || is_operator("|="))// To do: operator >>= is split into two > tokens
	{
		return 1; // lowest
	}
	else
	{
		return -1;
	}
}

AstBinaryExpression *Parser::create_token_expression()
{
	if (is_operator("*"))
		return newNode<AstMultiplicationExpression>();
	else if (is_operator("/"))
		return newNode<AstDivisionExpression>();
	else if (is_operator("%"))
		return newNode<AstRemainderExpression>();
	else if (is_operator("+"))
		return newNode<AstAdditionExpression>();
	else if (is_operator("-"))
		return newNode<AstSubtractionExpression>();
	else if (is_operator("$"))
		return newNode<AstStringConcatExpression>();
	else if (is_operator("<<"))
		return newNode<AstShiftLeftExpression>();
	else if (is_operator("<"))
		return newNode<AstLessExpression>();
	else if (is_operator(">"))
	{
		// operator >> and >>= are split into two > tokens
		auto pos = save_position();
		next();
		if (is_operator(">"))
		{
			return newNode<AstShiftRightExpression>();
		}
		else if (is_operator(">="))
		{
			return newNode<AstAssignmentExpression>(">>=");
		}
		else
		{
			restore_position(pos);
			return newNode<AstGreaterExpression>();
		}
	}
	else if (is_operator("<="))
		return newNode<AstLessEqualExpression>();
	else if (is_operator(">="))
		return newNode<AstGreaterEqualExpression>();
	else if (is_keyword("is"))
		return newNode<AstIsExpression>();
	else if (is_keyword("as"))
		return newNode<AstAsExpression>();
	else if (is_operator("=="))
		return newNode<AstEqualExpression>();
	else if (is_operator("!="))
		return newNode<AstNotEqualExpression>();
	else if (is_operator("~="))
		return newNode<AstCaseInsensitiveEqualExpression>();
	else if (is_operator("&"))
		return newNode<AstLogicalAndExpression>();
	else if (is_operator("^"))
		return newNode<AstLogicalXorExpression>();
	else if (is_operator("|"))
		return newNode<AstLogicalOrExpression>();
	else if (is_operator("&&"))
		return newNode<AstConditionalAndExpression>();
	else if (is_operator("||"))
		return newNode<AstConditionalOrExpression>();
	else if (is_operator("="))
		return newNode<AstAssignmentExpression>("=");
	else if (is_operator("*="))
		return newNode<AstAssignmentExpression>("*=");
	else if (is_operator("/="))
		return newNode<AstAssignmentExpression>("/=");
	else if (is_operator("%="))
		return newNode<AstAssignmentExpression>("%=");
	else if (is_operator("+="))
		return newNode<AstAssignmentExpression>("+=");
	else if (is_operator("-="))
		return newNode<AstAssignmentExpression>("-=");
	else if (is_operator("<<="))
		return newNode<AstAssignmentExpression>("<<=");
	else if (is_operator("&="))
		return newNode<AstAssignmentExpression>("&=");
	else if (is_operator("^="))
		return newNode<AstAssignmentExpression>("^=");
	else if (is_operator("|="))
		return newNode<AstAssignmentExpression>("|=");
	else if (is_keyword("dot"))
		return newNode<AstDotProductExpression>();
	else if (is_keyword("cross"))
		return newNode<AstCrossProductExpression>();
	else if (is_operator("**"))
		return newNode<AstExponentiationExpression>();

	throw_parse_exception("syntax error");
}

bool Parser::is_end_condition(ExpressionEndCondition end_condition) const
{
	return
		((end_condition & semicolon_end) && is_operator(";")) ||
		((end_condition & paranthesis_end) && is_operator(")")) ||
		((end_condition & bracket_end) && is_operator("]")) ||
		((end_condition & colon_end) && is_operator(":")) ||
		((end_condition & comma_end) && is_operator(",")) ||
		((end_condition & brace_end) && is_operator("}")) ||
		((end_condition & brace_begin) && is_operator("{"));
}
