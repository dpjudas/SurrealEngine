
#pragma once

#include <string>

class Token
{
public:
	enum Type
	{
		type_eof,
		type_error,
		type_identifier,
		type_keyword,
		type_bool,
		type_integer,
		type_real,
		type_character,
		type_string,
		type_null,
		type_operator
	};

	Token() = default;
	Token(Type type, const std::string &value, int line, int column) : type(type), value(value), line(line), column(column) { }

	Type type = type_eof;
	std::string value;
	int line = 0;
	int column = 0;
};
