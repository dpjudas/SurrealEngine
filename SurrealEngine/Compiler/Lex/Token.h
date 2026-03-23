
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
		type_bool,
		type_integer,
		type_real,
		type_name,
		type_string,
		type_none,
		type_operator,
		type_object_name,
	};

	Token() = default;
	Token(Type type, const std::string &value, int line, int column) : type(type), value(value), line(line), column(column) { }

	Type type = type_eof;
	std::string value;
	int line = 0;
	int column = 0;
};
