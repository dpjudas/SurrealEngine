
#include "Parser.h"

AstClassDeclaration *Parser::parse_class_declaration()
{
	if (!is_keyword("class"))
		throw_parse_exception("class expected");
	next();

	AstClassDeclaration *class_decl = newNode<AstClassDeclaration>();

	if (!is_identifier())
		throw_parse_exception("identifier expected");
	class_decl->identifier = token.value;
	next();

	while (!is_operator(";"))
	{
		if (is_keyword("extends"))
		{
			next();
			if (class_decl->base_type)
				throw_parse_exception("base class already specified");
			class_decl->base_type = parse_identifier_name();
		}
		else if (is_keyword("native") || is_keyword("intrinsic"))
		{
			next();
			class_decl->is_native = true;
		}
		else if (is_keyword("abstract"))
		{
			next();
			class_decl->is_abstract = true;
		}
		else if (is_keyword("noexport"))
		{
			next();
			class_decl->no_export = true;
		}
		else
		{
			throw_parse_exception("; expected");
		}
	}
	next();

	while (token.type != Token::type_eof)
	{
		if (is_keyword("defaultproperties") || is_keyword("_defaultproperties"))
			break;
		class_decl->members.push_back(parse_class_member());
	}
	next();

	if (is_keyword("defaultproperties") || is_keyword("_defaultproperties"))
	{
		// To do: parse default properties
		while (token.type != Token::type_eof)
			next();
	}

	return class_decl;
}

AstStructDeclaration *Parser::parse_struct_declaration()
{
	if (!is_keyword("struct"))
		throw_parse_exception("struct expected");
	next();

	AstStructDeclaration *struct_decl = newNode<AstStructDeclaration>();

	if (!is_identifier())
		throw_parse_exception("identifier expected");
	struct_decl->identifier = token.value;
	next();

	if (!is_operator("{"))
		throw_parse_exception("{ expected");
	next();

	while (!is_operator("}"))
	{
		struct_decl->members.push_back(parse_class_member());
	}
	next();

	return struct_decl;
}

AstEnumDeclaration *Parser::parse_enum_declaration()
{
	if (!is_keyword("enum"))
		throw_parse_exception("enum expected");
	next();

	AstEnumDeclaration *enum_decl = newNode<AstEnumDeclaration>();

	if (!is_identifier())
		throw_parse_exception("identifier expected");
	enum_decl->identifier = token.value;
	next();

	if (!is_operator("{"))
		throw_parse_exception("{ expected");
	next();

	while (!is_operator("}"))
	{
		AstEnumValueDeclaration *value = newNode<AstEnumValueDeclaration>();

		if (!is_identifier())
			throw_parse_exception("identifier expected");
		value->identifier = token.value;
		next();

		if (is_operator("="))
		{
			next();
			value->expression = parse_expression(comma_end | brace_end);
		}

		if (is_operator(","))
			next();
		else if (!is_operator("}"))
			throw_parse_exception("{ or , expected");

		enum_decl->members.push_back(value);
	}
	next();

	return enum_decl;
}
