
#pragma once

#include "Token.h"
#include <string>
#include <set>

class SavedTokenizerPos
{
public:
	size_t pos = 0;
	size_t line_start_pos = 0;
	size_t line_number = 1;
};

class Tokenizer
{
public:
	Tokenizer(const std::string &data);

	Token next();

	Token peek();

	SavedTokenizerPos save_position()
	{
		SavedTokenizerPos save;
		save.pos = pos;
		save.line_start_pos = line_start_pos;
		save.line_number = line_number;
		return save;
	}

	void restore_position(const SavedTokenizerPos &save)
	{
		pos = save.pos;
		line_start_pos = save.line_start_pos;
		line_number = save.line_number;
	}

private:
	bool read_newline();
	bool read_whitespace();
	bool read_comment();
	bool read_token(Token &token);

	bool read_identifier(Token &token);
	bool read_keyword(Token &token);
	bool read_bool_literal(Token &token);
	bool read_integer_literal(Token &token);
	bool read_real_literal(Token &token);
	bool read_character_literal(Token &token);
	bool read_string_literal(Token &token);
	bool read_null_literal(Token &token);
	bool read_operator_or_punctuator(Token &token);

	bool read_identifier_start_character(std::u32string::value_type &character);
	bool read_identifier_part_character(std::u32string::value_type &character);

	bool read_literal_character(std::u32string::value_type &character, std::string &error);


	bool read_pp_directive();
	bool read_pp_declaration();
	bool read_pp_conditional();
	bool read_pp_line();
	bool read_pp_diagnostic();
	bool read_pp_region();
	bool read_pp_pragma();

	bool read_pp_newline();
	bool read_pp_conditional_symbol();
	void read_pp_message();
	bool read_pp_comment();

	std::u32string data;
	size_t pos = 0;
	size_t line_start_pos = 0;
	size_t line_number = 1;

	static const std::set<std::string> keywords;
	static const char *operators[];
};
