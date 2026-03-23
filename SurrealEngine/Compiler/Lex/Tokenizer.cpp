
#include "Tokenizer.h"
#include "TextUtil.h"

#ifdef _MSC_VER
#pragma warning(disable: 4267) // warning C4267: '=': conversion from 'size_t' to 'int', possible loss of data
#endif

Tokenizer::Tokenizer(const std::string &data) : data(TextUtil::utf8_to_utf32(data))
{
}

Token Tokenizer::peek()
{
	size_t last_pos = pos;
	try
	{
		Token token = next();
		pos = last_pos;
		return token;
	}
	catch (...)
	{
		pos = last_pos;
		throw;
	}
}

Token Tokenizer::next()
{
	Token token;

	if (pos == data.size())
	{
		token.line = line_number;
		token.column = pos - line_start_pos;
		token.type = Token::type_eof;
	}
	else
	{
		while (true)
		{
			if (read_newline())
			{
				read_pp_directive();
			}
			else if (!read_whitespace() && !read_comment())
			{
				break;
			}
		}
		// To do: check for pp-directive after a newline
		while (read_newline() || read_whitespace() || read_comment());

		if (pos == data.size())
		{
			token.line = line_number;
			token.column = pos - line_start_pos;
			token.type = Token::type_eof;
		}
		else if (!read_token(token))
		{
			token.line = line_number;
			token.column = pos - line_start_pos;
			token.type = Token::type_error;
			token.value = "Unexpected character";
		}
	}

	return token;
}

bool Tokenizer::read_whitespace()
{
	size_t start_pos = pos;

	for (; pos != data.size(); pos++)
	{
		bool found = false;
		switch (data[pos])
		{
		default:
			break;

		case 0x0020: // To do: Add "Any character with Unicode class Zs"
		case 0x0009:
		case 0x000b:
		case 0x000c:
			found = true;
			break;
		}
		if (!found)
			break;
	}

	return start_pos != pos;
}

bool Tokenizer::read_comment()
{
	if (pos + 1 >= data.size())
		return false;

	if (data[pos] == '/' && data[pos + 1] == '/')
	{
		pos += 2;

		for (; pos != data.size(); pos++)
		{
			bool found = true;
			switch (data[pos])
			{
			default:
				break;

			case 0x000a:
				line_start_pos = pos;
				line_number++;
				found = false;
				break;

			case 0x000d:
			case 0x0085:
			case 0x2028:
			case 0x2029:
				found = false;
				break;
			}
			if (!found)
				break;
		}

		return true;
	}
	else if (data[pos] == '/' && data[pos + 1] == '*')
	{
		pos += 2;
		for (; pos != data.size(); pos++)
		{
			if (data[pos] == '*' && pos + 1 != data.size() && data[pos + 1] == '/')
			{
				pos += 2;
				break;
			}
		}
		return true;
	}
	else
	{
		return false;
	}
}

bool Tokenizer::read_token(Token &token)
{
	return
		read_bool_literal(token) ||
		read_none_literal(token) ||
		read_real_literal(token) ||
		read_integer_literal(token) ||
		read_name_literal(token) ||
		read_string_literal(token) ||
		read_operator_or_punctuator(token) ||
		read_object_name_literal(token) ||
		read_identifier(token);
}

bool Tokenizer::read_newline()
{
	size_t start_pos = pos;

	bool found = false;
	switch (data[pos])
	{
	default:
		return false;

	case 0x000d:
		if (pos + 1 != data.size() && data[pos + 1] == 0x000a)
		{
			pos++;
			line_start_pos = pos + 1;
			line_number++;
		}
		pos++;
		return true;

	case 0x000a:
		pos++;
		line_start_pos = pos;
		line_number++;
		return true;

	case 0x2085:
	case 0x2028:
	case 0x2029:
		pos++;
		return true;
	}
}

bool Tokenizer::read_object_name_literal(Token& token)
{
	size_t start_pos = pos;

	std::u32string::value_type character = 0;
	if (read_identifier_start_character(character))
	{
		std::u32string identifier;
		identifier.push_back(character);

		while (read_identifier_part_character(character))
			identifier.push_back(character);

		if (pos == data.size() || data[pos] != '\'')
		{
			pos = start_pos;
			return false;
		}
		identifier.push_back('\'');
		pos++;

		while (read_identifier_part_character(character))
			identifier.push_back(character);

		if (pos == data.size() || data[pos] != '\'')
		{
			pos = start_pos;
			return false;
		}
		identifier.push_back('\'');
		pos++;

		token.line = line_number;
		token.column = start_pos - line_start_pos;
		token.type = Token::type_object_name;
		token.value = TextUtil::utf32_to_utf8(identifier);

		return true;
	}
	else
	{
		pos = start_pos;
		return false;
	}
}

bool Tokenizer::read_identifier(Token &token)
{
	size_t start_pos = pos;

	std::u32string::value_type character = 0;
	if (read_identifier_start_character(character))
	{
		std::u32string identifier;
		identifier.push_back(character);

		while (read_identifier_part_character(character))
			identifier.push_back(character);

		token.line = line_number;
		token.column = start_pos - line_start_pos;
		token.type = Token::type_identifier;
		token.value = TextUtil::utf32_to_utf8(identifier);

		return true;
	}
	else
	{
		pos = start_pos;
		return false;
	}
}

bool Tokenizer::read_identifier_start_character(std::u32string::value_type &character)
{
	if (pos == data.size())
		return false;

	if (data[pos] == '_' ||
		(data[pos] >= 'a' && data[pos] <= 'z') ||
		(data[pos] >= 'A' && data[pos] <= 'Z'))
	{
		character = data[pos];
		pos++;
		return true;
	}

	return false;
}

bool Tokenizer::read_identifier_part_character(std::u32string::value_type &character)
{
	if (pos == data.size())
		return false;

	if (data[pos] == '_' ||
		(data[pos] >= 'a' && data[pos] <= 'z') ||
		(data[pos] >= 'A' && data[pos] <= 'Z') ||
		(data[pos] >= '0' && data[pos] <= '9'))
	{
		character = data[pos];
		pos++;
		return true;
	}

	return false;
}

bool Tokenizer::read_bool_literal(Token &token)
{
	if (data[pos] == 't' && pos + 4 <= data.size())
	{
		const char *keyword = "true";
		for (int i = 0; i < 4; i++)
		{
			if (data[pos + i] != keyword[i])
				return false;
		}
		token.line = line_number;
		token.column = pos - line_start_pos;
		token.type = Token::type_bool;
		token.value = keyword;
		pos += 4;
		return true;
	}
	else if (data[pos] == 'f' && pos + 5 <= data.size())
	{
		const char *keyword = "false";
		for (int i = 0; i < 5; i++)
		{
			if (data[pos + i] != keyword[i])
				return false;
		}
		token.line = line_number;
		token.column = pos - line_start_pos;
		token.type = Token::type_bool;
		token.value = keyword;
		pos += 5;
		return true;
	}

	return false;
}

bool Tokenizer::read_integer_literal(Token &token)
{
	size_t start_pos = pos;

	if (pos + 2 <= data.size() && data[pos] == '0' && data[pos + 1] == 'x') // hex
	{
		pos += 2;

		for (; pos != data.size(); pos++)
		{
			bool is_hex_digit = (data[pos] >= '0' && data[pos] <= '9') || (data[pos] >= 'a' && data[pos] <= 'f') || (data[pos] >= 'A' && data[pos] <= 'F');
			if (!is_hex_digit)
				break;
		}

		if (pos - start_pos == 2)
		{
			pos = start_pos;
			return false;
		}
	}
	else if (data[pos] >= '0' && data[pos] <= '9') // decimal
	{
		pos++;
		for (; pos != data.size(); pos++)
		{
			if (data[pos] < '0' || data[pos] > '9')
				break;
		}
	}
	else
	{
		return false;
	}

	// Suffix:

	bool is_unsigned = false;
	bool is_long = false;

	if (pos != data.size())
	{
		if (data[pos] == 'U' || data[pos] == 'u')
		{
			is_unsigned = true;
			pos++;
		}
		else if (data[pos] == 'L' || data[pos] == 'l')
		{
			is_long = true;
			pos++;
		}
	}

	if (is_unsigned && pos != data.size() && (data[pos] == 'L' || data[pos] == 'l'))
	{
		is_long = true;
		pos++;
	}
	else if (is_long && pos != data.size() && (data[pos] == 'U' || data[pos] == 'u'))
	{
		is_unsigned = true;
		pos++;
	}

	token.line = line_number;
	token.column = start_pos - line_start_pos;
	token.type = Token::type_integer;
	token.value = TextUtil::utf32_to_utf8(data.substr(start_pos, pos - start_pos));
	return true;
}

bool Tokenizer::read_real_literal(Token &token)
{
	if (data[pos] != '.' && (data[pos] < '0' || data[pos] > '9'))
		return false;

	size_t start_pos = pos;

	bool digits_encountered = false;
	bool dot_encountered = false;
	bool exponent_encountered = false;
	bool suffix_encountered = false;

	for (; pos != data.size(); pos++)
	{
		if (data[pos] < '0' || data[pos] > '9')
			break;

		digits_encountered = true;
	}

	if (pos != data.size() && data[pos] == '.')
	{
		pos++;

		for (; pos != data.size(); pos++)
		{
			if (data[pos] < '0' || data[pos] > '9')
				break;
			digits_encountered = true;
		}

		dot_encountered = true;
	}

	if (!digits_encountered) // dot operator
	{
		pos = start_pos;
		return false;
	}

	if (pos != data.size() && (data[pos] == 'e' || data[pos] == 'E'))
	{
		pos++;
		if (pos != data.size() && (data[pos] == '+' || data[pos] == '-'))
			pos++;

		size_t exponent_pos = pos;
		for (; pos != data.size(); pos++)
		{
			if (data[pos] < '0' || data[pos] > '9')
				break;
		}

		if (pos == exponent_pos)
		{
			pos = start_pos;
			return false;
		}

		exponent_encountered = true;
	}

	if (pos != data.size() && (data[pos] == 'F' || data[pos] == 'f' || data[pos] == 'D' || data[pos] == 'd' || data[pos] == 'M' || data[pos] == 'm'))
	{
		pos++;
		suffix_encountered = true;
	}

	if (!dot_encountered && !exponent_encountered && !suffix_encountered) // integer literal
	{
		pos = start_pos;
		return false;
	}

	token.line = line_number;
	token.column = start_pos - line_start_pos;
	token.type = Token::type_real;
	token.value = TextUtil::utf32_to_utf8(data.substr(start_pos, pos - start_pos));

	return true;
}

bool Tokenizer::read_name_literal(Token &token)
{
	size_t start_pos = pos;
	if (data[pos] == '\'')
	{
		pos++;

		std::u32string str_literal;

		while (pos != data.size())
		{
			if (data[pos] != '\'')
			{
				str_literal.push_back(data[pos]);
				pos++;
			}
			else if (pos + 2 <= data.size() && data[pos + 1] == '\'')
			{
				str_literal.push_back('\'');
				pos += 2;
			}
			else
			{
				break;
			}
		}

		if (pos == data.size())
		{
			token.line = line_number;
			token.column = pos - line_start_pos;
			token.type = Token::type_error;
			token.value = "Unexpected end of file";
			return true;
		}

		pos++;

		token.line = line_number;
		token.column = start_pos - line_start_pos;
		token.type = Token::type_name;
		token.value = TextUtil::utf32_to_utf8(str_literal);

		return true;
	}

	return false;
}

bool Tokenizer::read_string_literal(Token &token)
{
	size_t start_pos = pos;

	if (data[pos] == '"')
	{
		pos++;

		std::u32string str_literal;

		while (pos != data.size() && data[pos] != '"')
		{
			std::u32string::value_type character = 0;
			std::string error;
			if (!read_literal_character(character, error))
			{
				token.line = line_number;
				token.column = pos - line_start_pos;
				token.type = Token::type_error;
				token.value = error;
				return true;
			}

			str_literal.push_back(character);
		}

		if (pos == data.size())
		{
			token.line = line_number;
			token.column = pos - line_start_pos;
			token.type = Token::type_error;
			token.value = "Unexpected end of file";
			return true;
		}

		pos++;

		token.line = line_number;
		token.column = start_pos - line_start_pos;
		token.type = Token::type_string;
		token.value = TextUtil::utf32_to_utf8(str_literal);

		return true;
	}

	return false;
}

bool Tokenizer::read_literal_character(std::u32string::value_type &character, std::string &error)
{
	error.clear();

	if (read_newline())
	{
		error = "Unexpected newline";
		return false;
	}
	else if (pos == data.size())
	{
		error = "Unexpected end of file";
		return false;
	}
	else if (data[pos] == 0x005c)
	{
		error = "Unexpected character";
		return false;
	}
	else if (data[pos] == '\\')
	{
		pos++;
		if (pos + 1 < data.size())
		{
			error = "Unexpected end of file";
			return false;
		}

		if (data[pos] == 'x')
		{
			pos++;

			unsigned int hex_value = 0;

			size_t hex_start = pos;
			for (; pos != data.size(); pos++)
			{
				if ((data[pos] >= '0' && data[pos] <= '9'))
				{
					hex_value <<= 4;
					hex_value += data[pos] - '0';
				}
				else if (data[pos] >= 'a' && data[pos] <= 'f')
				{
					hex_value <<= 4;
					hex_value += 10 + (data[pos] - 'a');
				}
				else if (data[pos] >= 'A' && data[pos] <= 'F')
				{
					hex_value <<= 4;
					hex_value += 10 + (data[pos] - 'A');
				}
				else
				{
					break;
				}
			}

			size_t hex_length = pos - hex_start;
			if (hex_length == 0 || hex_length > 4)
			{
				error = "Invalid hex escape";
				return false;
			}

			character = hex_value;
		}
		else if (data[pos] == '\'' || data[pos] == '"' || data[pos] == '\\')
		{
			character = data[pos];
		}
		else if (data[pos] == '0')
		{
			character = 0;
		}
		else if (data[pos] == 'a')
		{
			character = '\a';
		}
		else if (data[pos] == 'b')
		{
			character = '\b';
		}
		else if (data[pos] == 'f')
		{
			character = '\f';
		}
		else if (data[pos] == 'n')
		{
			character = '\n';
		}
		else if (data[pos] == 'r')
		{
			character = '\r';
		}
		else if (data[pos] == 't')
		{
			character = '\t';
		}
		else if (data[pos] == 'v')
		{
			character = '\v';
		}
		else
		{
			pos++;
			error = "Unrecognized escape character";
			return false;
		}
		pos++;
		return true;
	}
	else
	{
		character = data[pos];
		pos++;
		return true;
	}
}

bool Tokenizer::read_none_literal(Token &token)
{
	if ((data[pos] == 'n' || data[pos] == 'N') && pos + 4 <= data.size())
	{
		const char *keyword = "none";
		for (int i = 0; i < 4; i++)
		{
			std::u32string::value_type c = data[pos + i];
			if (c >= 'A' && c <= 'Z')
				c = c - 'A' + 'a';
			if (c != keyword[i])
				return false;
		}
		token.line = line_number;
		token.column = pos - line_start_pos;
		token.type = Token::type_none;
		token.value = keyword;
		pos += 4;
		return true;
	}

	return false;
}

bool Tokenizer::read_operator_or_punctuator(Token &token)
{
	for (int i = 0; operators[i] != 0; i++)
	{
		size_t j;
		for (j = 0; operators[i][j] != 0; j++)
		{
			if (pos + j == data.size() || data[pos + j] != operators[i][j])
				break;
		}

		if (operators[i][j] == 0)
		{
			token.line = line_number;
			token.column = pos - line_start_pos;
			token.type = Token::type_operator;
			token.value = operators[i];
			pos += j;
			return true;
		}
	}

	return false;
}

const char *Tokenizer::operators[] = 
{
	"??",
	"::",
	"++",
	"--",
	"&&",
	"||",
	"->",
	"==",
	"!=",
	"<=",
	">=",
	"+=",
	"-=",
	"*=",
	"/=",
	"%=",
	"&=",
	"|=",
	"^=",
	"<<=",
	"<<",
	"{",
	"}",
	"[",
	"]",
	"(",
	")",
	".",
	",",
	":",
	";",
	"$",
	"+",
	"-",
	"*",
	"/",
	"%",
	"&",
	"|",
	"^",
	"!",
	"~",
	"=",
	"<",
	">",
	"?",
	0
};



bool Tokenizer::read_pp_directive()
{
	size_t start_pos = pos;

	read_whitespace();

	if (pos == data.size())
		return false;

	if (data[pos] != '#')
		return false;

	pos++;
	read_whitespace();

	bool result =
		read_pp_declaration() ||
		read_pp_conditional() ||
		read_pp_line() ||
		read_pp_diagnostic() ||
		read_pp_region() ||
		read_pp_pragma();

	if (!result)
	{
		pos = start_pos;
	}

	return result;
}

bool Tokenizer::read_pp_declaration()
{
	if (pos + 6 <= data.size() && data.substr(pos, 6) == TextUtil::utf8_to_utf32("define"))
	{
		pos += 6;
		read_whitespace();
		return
			read_pp_conditional_symbol() &&
			read_pp_newline();
	}
	else if (pos + 5 <= data.size() && data.substr(pos, 5) == TextUtil::utf8_to_utf32("undef"))
	{
		pos += 5;
		read_whitespace();
		return
			read_pp_conditional_symbol() &&
			read_pp_newline();
	}
	return false;
}

bool Tokenizer::read_pp_conditional()
{
	// To do: parse and skip blocks accordingly

	if (pos + 2 <= data.size() && data.substr(pos, 2) == TextUtil::utf8_to_utf32("if"))
	{
		pos += 2;
		return read_pp_comment();
	}
	else if (pos + 4 <= data.size() && data.substr(pos, 4) == TextUtil::utf8_to_utf32("elif"))
	{
		pos += 4;
		return read_pp_comment();
	}
	else if (pos + 4 <= data.size() && data.substr(pos, 4) == TextUtil::utf8_to_utf32("else"))
	{
		pos += 4;
		return read_pp_comment();
	}
	else if (pos + 5 <= data.size() && data.substr(pos, 5) == TextUtil::utf8_to_utf32("endif"))
	{
		pos += 5;
		return read_pp_comment();
	}

	return false;
}

bool Tokenizer::read_pp_line()
{
	if (pos + 4 > data.size() || data.substr(pos, 4) != TextUtil::utf8_to_utf32("line"))
		return false;

	pos += 4;
	read_whitespace();

	// decimal-digits whitespace file-name
	// decimal-digits
	// identifier-or-keyword

	read_pp_message(); // To do: parse this properly like described above
	return true;
}

bool Tokenizer::read_pp_diagnostic()
{
	if (pos + 5 <= data.size() && data.substr(pos, 5) == TextUtil::utf8_to_utf32("error"))
	{
		pos += 5;
		read_whitespace();
		return
			read_pp_conditional_symbol() &&
			read_pp_newline();
	}
	else if (pos + 7 <= data.size() && data.substr(pos, 7) == TextUtil::utf8_to_utf32("warning"))
	{
		pos += 7;
		read_whitespace();
		return
			read_pp_conditional_symbol() &&
			read_pp_newline();
	}
	return false;
}

bool Tokenizer::read_pp_region()
{
	if (pos + 6 <= data.size() && data.substr(pos, 6) == TextUtil::utf8_to_utf32("region"))
	{
		pos += 6;
		read_pp_message();

		return true;
	}
	else if (pos + 9 <= data.size() && data.substr(pos, 9) == TextUtil::utf8_to_utf32("endregion"))
	{
		pos += 9;
		read_pp_message();

		return true;
	}
	return false;
}

bool Tokenizer::read_pp_pragma()
{
	if (pos + 6 > data.size() || data.substr(pos, 6) != TextUtil::utf8_to_utf32("pragma"))
		return false;

	pos += 6;
	read_pp_message();

	return true;
}

void Tokenizer::read_pp_message()
{
	read_whitespace();

	while (pos != data.size())
	{
		read_pp_comment();
		if (read_newline())
			break;
		pos++;
	}
}

bool Tokenizer::read_pp_newline()
{
	read_whitespace();
	read_pp_comment();
	return read_newline();
}

bool Tokenizer::read_pp_conditional_symbol()
{
	Token token;
	return read_identifier(token);
}

bool Tokenizer::read_pp_comment()
{
	if (pos + 1 >= data.size())
		return false;

	if (data[pos] == '/' && data[pos + 1] == '/')
	{
		pos += 2;

		for (; pos != data.size(); pos++)
		{
			bool found = true;
			switch (data[pos])
			{
			default:
				break;

			case 0x000a:
				line_start_pos = pos;
				line_number++;
				found = false;
				break;

			case 0x000d:
			case 0x0085:
			case 0x2028:
			case 0x2029:
				found = false;
				break;
			}
			if (!found)
				break;
		}

		return true;
	}
	else
	{
		return false;
	}
}
