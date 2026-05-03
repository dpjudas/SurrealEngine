
#include "Precomp.h"
#include "Parser.h"

Parser::Parser(const std::string &source)
	: tokenizer(source)
{
}

std::shared_ptr<AstCompilationUnit> Parser::parse()
{
	ast.reset(new AstCompilationUnit());
	next();
	ast->class_decl = parse_class_declaration();
	return ast;
}

void Parser::set_line_info(AstNode* node)
{
	node->line = token.line;
	node->column = token.column;
}

void Parser::skip_until_body()
{
	while (!is_operator("{"))
		next();
}

void Parser::skip_body()
{
	if (!is_operator("{"))
		throw_parse_exception("syntax error");

	next();

	int paranthesis_count = 1;
	while (paranthesis_count > 0)
	{
		if (is_operator("{"))
			paranthesis_count++;
		else if (is_operator("}"))
			paranthesis_count--;
		
		next();
	}
}

void Parser::next()
{
	token = tokenizer.next();
	if (token.type == Token::type_error)
		throw_parse_exception("syntax error");
}

bool Parser::is_type_keyword() const
{
	return
		is_keyword("bool") ||
		is_keyword("byte") ||
		is_keyword("char") ||
		is_keyword("double") ||
		is_keyword("float") ||
		is_keyword("int") ||
		is_keyword("long") ||
		is_keyword("object") ||
		is_keyword("sbyte") ||
		is_keyword("short") ||
		is_keyword("string") ||
		is_keyword("uint") ||
		is_keyword("ulong") ||
		is_keyword("ushort") ||
		is_keyword("void");
}

bool Parser::is_keyword(const char *keyword) const
{
	if (token.type != Token::type_identifier)
		return false;

	if (token.value.size() > 50)
		return false;

	int size = (int)token.value.size();
	int i;
	for (i = 0; keyword[i] != 0; i++)
	{
		if (i == size)
			return false;

		char c = token.value[i];
		if (c >= 'A' && c <= 'Z')
			c = c - 'A' + 'a';

		if (c != keyword[i])
			return false;
	}
	return i == size;
}

bool Parser::is_name() const
{
	return token.type == Token::type_name;
}

bool Parser::is_identifier() const
{
	return token.type == Token::type_identifier;
}

bool Parser::is_identifier(const char *identifier) const
{
	return token.type == Token::type_identifier && token.value == identifier;
}

bool Parser::is_operator(const char *oper) const
{
	return token.type == Token::type_operator && token.value == oper;
}

SavedParserPos Parser::save_position()
{
	SavedParserPos save;
	save.token = token;
	save.tokenpos = tokenizer.save_position();
	return save;
}

void Parser::restore_position(const SavedParserPos &save)
{
	token = save.token;
	tokenizer.restore_position(save.tokenpos);
}

void Parser::throw_parse_exception(const std::string &message)
{
	throw ParseException(message, token.line, token.column);
}
