#pragma once

#include "theme_stylesheet_tokenizer.h"
#include "surrealwidgets/core/colorf.h"

#include <string>
#include <vector>

class ThemeStyleSelector // css selector link
{
public:
	std::string widget; // css element
	std::string theme; // css class
	std::string state; // css pseudo class
};

class ThemeStyleProperty
{
public:
	std::string name;
	std::vector<ThemeStylesheetToken> value;
	bool importantFlag = false;
};

class ThemeStyleRuleset
{
public:
	std::vector<ThemeStyleSelector> selectors;
	std::vector<ThemeStyleProperty> properties;
};

class ThemeSortedSelector
{
public:
	size_t specificity = 0;
	ThemeStyleSelector* selector = nullptr;
	ThemeStyleRuleset* ruleset = nullptr;

	bool operator<(const ThemeSortedSelector& other) const { return specificity < other.specificity; }
};

class ThemeStylesheetDocument
{
public:
	ThemeStylesheetDocument(const std::string& stylesheet);

	static bool parse_color(const std::vector<ThemeStylesheetToken>& tokens, size_t& in_out_pos, Colorf& out_color);

	std::vector<ThemeStyleRuleset> rulesets;
	std::vector<ThemeSortedSelector> selectors;

private:
	void create_sorted_selectors();
	void read_stylesheet(const std::string& stylesheet);
	void read_statement(ThemeStylesheetTokenizer& tokenizer, ThemeStylesheetToken& token);
	bool read_selector(ThemeStylesheetTokenizer& tokenizer, ThemeStylesheetToken& token, ThemeStyleSelector& out_selector);
	bool read_end_of_statement(ThemeStylesheetTokenizer& tokenizer, ThemeStylesheetToken& token);
	void read_at_rule(ThemeStylesheetTokenizer& tokenizer, ThemeStylesheetToken& token);
	void read_end_of_at_rule(ThemeStylesheetTokenizer& tokenizer, ThemeStylesheetToken& token);
};
