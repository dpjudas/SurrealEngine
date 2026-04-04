
#include "theme_stylesheet_document.h"
#include <algorithm>

ThemeStylesheetDocument::ThemeStylesheetDocument(const std::string& stylesheet)
{
	read_stylesheet(stylesheet);
	create_sorted_selectors();
}

void ThemeStylesheetDocument::create_sorted_selectors()
{
	size_t a = 0;
	for (ThemeStyleRuleset& ruleset : rulesets)
	{
		for (ThemeStyleSelector& selector : ruleset.selectors)
		{
			size_t b = 0; // #id, not used
			size_t c = 0;
			size_t d = (selector.widget != "*") ? 1 : 0;
			if (selector.theme.empty())
				c++;
			if (selector.state.empty())
				c++;

			ThemeSortedSelector s;
			s.selector = &selector;
			s.ruleset = &ruleset;
			s.specificity = (a << 32) | (b << 24) | (c << 8) | d;
			selectors.push_back(std::move(s));
		}
		a++;
	}
	std::stable_sort(selectors.begin(), selectors.end());
}

void ThemeStylesheetDocument::read_stylesheet(const std::string& stylesheet)
{
	ThemeStylesheetTokenizer tokenizer(stylesheet);
	ThemeStylesheetToken token;

	while (true)
	{
		tokenizer.read(token, true);
		if (token.type == ThemeStylesheetTokenType::null)
		{
			break;
		}
		else if (token.type == ThemeStylesheetTokenType::atkeyword)
		{
			read_at_rule(tokenizer, token);
		}
		else if (token.type != ThemeStylesheetTokenType::whitespace)
		{
			read_statement(tokenizer, token);
		}
	}
}

void ThemeStylesheetDocument::read_statement(ThemeStylesheetTokenizer& tokenizer, ThemeStylesheetToken& token)
{
	ThemeStyleRuleset ruleset;
	while (true)
	{
		ThemeStyleSelector selector;
		if (read_selector(tokenizer, token, selector))
		{
			ruleset.selectors.push_back(std::move(selector));
			if (token.type == ThemeStylesheetTokenType::delim && token.value == ",")
			{
				tokenizer.read(token, true);
			}
			else if (token.type == ThemeStylesheetTokenType::curly_brace_begin)
			{
				break;
			}
			else
			{
				read_end_of_statement(tokenizer, token);
				return;
			}
		}
		else
		{
			read_end_of_statement(tokenizer, token);
			return;
		}
	}

	if (token.type == ThemeStylesheetTokenType::curly_brace_begin)
	{
		while (true)
		{
			tokenizer.read(token, true);

			ThemeStyleProperty prop;
			if (token.type == ThemeStylesheetTokenType::delim && token.value == "-")
			{
				tokenizer.read(token, true);
				if (token.type == ThemeStylesheetTokenType::ident)
				{
					prop.name = "-" + token.value;
					tokenizer.read(token, true);
				}
				else
				{
					bool important_flag = false;
					tokenizer.read_property_value(token, important_flag);
				}
			}
			else if (token.type == ThemeStylesheetTokenType::ident)
			{
				prop.name = token.value;
				tokenizer.read(token, true);
			}
			else if (token.type == ThemeStylesheetTokenType::curly_brace_end)
			{
				break;
			}
			else if (token.type == ThemeStylesheetTokenType::null)
			{
				return;
			}

			if (token.type == ThemeStylesheetTokenType::colon)
			{
				tokenizer.read(token, true);
				prop.value = tokenizer.read_property_value(token, prop.importantFlag);
				ruleset.properties.push_back(std::move(prop));
			}
			else
			{
				bool important_flag = false;
				tokenizer.read_property_value(token, important_flag);
			}
		}
		rulesets.push_back(std::move(ruleset));
	}
	else
	{
		read_end_of_statement(tokenizer, token);
	}
}

bool ThemeStylesheetDocument::read_selector(ThemeStylesheetTokenizer& tokenizer, ThemeStylesheetToken& token, ThemeStyleSelector& out_selector)
{
	while (token.type == ThemeStylesheetTokenType::whitespace)
		tokenizer.read(token, true);

	ThemeStyleSelector selector_link;
	if (token.type == ThemeStylesheetTokenType::ident)
	{
		// Simple Selector
		selector_link.widget = token.value;
		tokenizer.read(token, false);
	}
	else if (token.type == ThemeStylesheetTokenType::delim && token.value == "*")
	{
		// Universal Selector
		selector_link.widget = "*";
		tokenizer.read(token, false);
	}
	else if (token.type == ThemeStylesheetTokenType::hash ||
		token.type == ThemeStylesheetTokenType::colon ||
		token.type == ThemeStylesheetTokenType::square_bracket_begin ||
		(token.type == ThemeStylesheetTokenType::delim && token.value == "."))
	{
		// Implicit Universal Selector
		selector_link.widget = "*";
	}
	else
	{
		return false;
	}

	while (true)
	{
		if (token.type == ThemeStylesheetTokenType::colon)
		{
			tokenizer.read(token, false);
			if (token.type == ThemeStylesheetTokenType::ident)
			{
				if (!selector_link.state.empty())
					return false;
				selector_link.state = token.value;
			}
		}
		else if (token.type == ThemeStylesheetTokenType::delim && token.value == ".")
		{
			tokenizer.read(token, false);
			if (token.type == ThemeStylesheetTokenType::ident)
			{
				if (!selector_link.theme.empty())
					return false;
				selector_link.theme = token.value;
			}
			else
			{
				return false;
			}
		}
		else
		{
			break;
		}

		tokenizer.read(token, false);
	}

	while (token.type == ThemeStylesheetTokenType::whitespace)
		tokenizer.read(token, true);

	out_selector = selector_link;
	return true;
}

bool ThemeStylesheetDocument::read_end_of_statement(ThemeStylesheetTokenizer& tokenizer, ThemeStylesheetToken& token)
{
	int curly_count = 0;
	while (true)
	{
		if (token.type == ThemeStylesheetTokenType::null)
		{
			break;
		}
		else if (token.type == ThemeStylesheetTokenType::curly_brace_begin)
		{
			curly_count++;
		}
		else if (token.type == ThemeStylesheetTokenType::curly_brace_end)
		{
			curly_count--;
			if (curly_count <= 0)
				break;
		}
		else if (token.type == ThemeStylesheetTokenType::semi_colon)
		{
			if (curly_count == 0)
				break;
		}
		tokenizer.read(token, true);
	}

	return curly_count < 0;
}

void ThemeStylesheetDocument::read_at_rule(ThemeStylesheetTokenizer& tokenizer, ThemeStylesheetToken& token)
{
	// We have no at rules currently - skip it
	read_end_of_at_rule(tokenizer, token);
}

void ThemeStylesheetDocument::read_end_of_at_rule(ThemeStylesheetTokenizer& tokenizer, ThemeStylesheetToken& token)
{
	int curly_count = 0;
	while (true)
	{
		if (token.type == ThemeStylesheetTokenType::null)
		{
			break;
		}
		else if (token.type == ThemeStylesheetTokenType::semi_colon && curly_count == 0)
		{
			break;
		}
		else if (token.type == ThemeStylesheetTokenType::curly_brace_begin)
		{
			curly_count++;
		}
		else if (token.type == ThemeStylesheetTokenType::curly_brace_end)
		{
			curly_count--;
			if (curly_count == 0)
				break;
		}
		tokenizer.read(token, false);
	}
}

static ThemeStylesheetToken next_token(size_t& pos, const std::vector<ThemeStylesheetToken>& tokens, bool skip_whitespace = true)
{
	ThemeStylesheetToken token;
	do
	{
		if (pos != tokens.size())
		{
			token = tokens[pos];
			pos++;
		}
		else
		{
			token = ThemeStylesheetToken();
		}
	} while (token.type == ThemeStylesheetTokenType::whitespace);
	return token;
}

static bool equals(const std::string& a, const std::string& b)
{
	return ThemeStylesheetTokenizer::compare_case_insensitive(a, b);
}

bool ThemeStylesheetDocument::parse_color(const std::vector<ThemeStylesheetToken>& tokens, size_t& in_out_pos, Colorf& out_color)
{
	size_t pos = in_out_pos;
	ThemeStylesheetToken token = next_token(pos, tokens);
	if (token.type == ThemeStylesheetTokenType::ident)
	{
		if (equals(token.value, "transparent"))
		{
			out_color = Colorf(0.0f, 0.0f, 0.0f, 0.0f);
			in_out_pos = pos;
			return true;
		}
	}
	else if (token.type == ThemeStylesheetTokenType::function && equals(token.value, "rgb"))
	{
		int color[3] = { 0, 0, 0 };
		for (int channel = 0; channel < 3; channel++)
		{
			token = next_token(pos, tokens);
			if (token.type == ThemeStylesheetTokenType::number)
			{
				int value = std::atoi(token.value.c_str());
				value = std::min(255, value);
				value = std::max(0, value);
				color[channel] = value;
			}
			else if (token.type == ThemeStylesheetTokenType::percentage)
			{
				float value = (float)std::atof(token.value.c_str()) / 100.0f;
				value = std::min(1.0f, value);
				value = std::max(0.0f, value);
				color[channel] = (int)(value * 255.0f);
			}
			else
			{
				return false;
			}

			if (channel < 2)
			{
				token = next_token(pos, tokens);
				if (token.type != ThemeStylesheetTokenType::delim || token.value != ",")
					return false;
			}
		}
		token = next_token(pos, tokens);
		if (token.type == ThemeStylesheetTokenType::bracket_end)
		{
			out_color = Colorf(color[0] / 255.0f, color[1] / 255.0f, color[2] / 255.0f, 1.0f);
			in_out_pos = pos;
			return true;
		}
	}
	else if (token.type == ThemeStylesheetTokenType::function && equals(token.value, "rgba"))
	{
		float color[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		for (int channel = 0; channel < 4; channel++)
		{
			token = next_token(pos, tokens);
			if (token.type == ThemeStylesheetTokenType::number)
			{
				if (channel < 3)
				{
					int value = std::atoi(token.value.c_str());
					value = std::min(255, value);
					value = std::max(0, value);
					color[channel] = value / 255.0f;
				}
				else
				{
					color[channel] = (float)std::atof(token.value.c_str());
				}
			}
			else if (token.type == ThemeStylesheetTokenType::percentage)
			{
				float value = (float)std::atof(token.value.c_str()) / 100.0f;
				value = std::min(1.0f, value);
				value = std::max(0.0f, value);
				color[channel] = value;
			}
			else
			{
				return false;
			}

			if (channel < 3)
			{
				token = next_token(pos, tokens);
				if (token.type != ThemeStylesheetTokenType::delim || token.value != ",")
					return false;
			}
		}
		token = next_token(pos, tokens);
		if (token.type == ThemeStylesheetTokenType::bracket_end)
		{
			out_color = Colorf(color[0], color[1], color[2], color[3]);
			in_out_pos = pos;
			return true;
		}
	}
	else if (token.type == ThemeStylesheetTokenType::hash)
	{
		if (token.value.length() == 3)
		{
			float channels[3] = { 0.0f, 0.0f, 0.0f };
			for (int c = 0; c < 3; c++)
			{
				int v = 0;
				if (token.value[c] >= '0' && token.value[c] <= '9')
					v = token.value[c] - '0';
				else if (token.value[c] >= 'a' && token.value[c] <= 'f')
					v = token.value[c] - 'a' + 10;
				else if (token.value[c] >= 'A' && token.value[c] <= 'F')
					v = token.value[c] - 'A' + 10;
				else
					return false;
				v = (v << 4) + v;
				channels[c] = v / 255.0f;
			}
			out_color = Colorf(channels[0], channels[1], channels[2]);
			in_out_pos = pos;
			return true;
		}
		else if (token.value.length() == 4)
		{
			float channels[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
			for (int c = 0; c < 4; c++)
			{
				int v = 0;
				if (token.value[c] >= '0' && token.value[c] <= '9')
					v = token.value[c] - '0';
				else if (token.value[c] >= 'a' && token.value[c] <= 'f')
					v = token.value[c] - 'a' + 10;
				else if (token.value[c] >= 'A' && token.value[c] <= 'F')
					v = token.value[c] - 'A' + 10;
				else
					return false;
				v = (v << 4) + v;
				channels[c] = v / 255.0f;
			}
			out_color = Colorf(channels[0], channels[1], channels[2], channels[3]);
			in_out_pos = pos;
			return true;
		}
		else if (token.value.length() == 6)
		{
			float channels[3] = { 0.0f, 0.0f, 0.0f };
			for (int c = 0; c < 3; c++)
			{
				int v = 0;
				if (token.value[c * 2] >= '0' && token.value[c * 2] <= '9')
					v = token.value[c * 2] - '0';
				else if (token.value[c * 2] >= 'a' && token.value[c * 2] <= 'f')
					v = token.value[c * 2] - 'a' + 10;
				else if (token.value[c * 2] >= 'A' && token.value[c * 2] <= 'F')
					v = token.value[c * 2] - 'A' + 10;
				else
					return false;

				v <<= 4;

				if (token.value[c * 2 + 1] >= '0' && token.value[c * 2 + 1] <= '9')
					v += token.value[c * 2 + 1] - '0';
				else if (token.value[c * 2 + 1] >= 'a' && token.value[c * 2 + 1] <= 'f')
					v += token.value[c * 2 + 1] - 'a' + 10;
				else if (token.value[c * 2 + 1] >= 'A' && token.value[c * 2 + 1] <= 'F')
					v += token.value[c * 2 + 1] - 'A' + 10;
				else
					return false;

				channels[c] = v / 255.0f;
			}
			out_color = Colorf(channels[0], channels[1], channels[2]);
			in_out_pos = pos;
			return true;
		}
		else if (token.value.length() == 8)
		{
			float channels[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
			for (int c = 0; c < 4; c++)
			{
				int v = 0;
				if (token.value[c * 2] >= '0' && token.value[c * 2] <= '9')
					v = token.value[c * 2] - '0';
				else if (token.value[c * 2] >= 'a' && token.value[c * 2] <= 'f')
					v = token.value[c * 2] - 'a' + 10;
				else if (token.value[c * 2] >= 'A' && token.value[c * 2] <= 'F')
					v = token.value[c * 2] - 'A' + 10;
				else
					return false;

				v <<= 4;

				if (token.value[c * 2 + 1] >= '0' && token.value[c * 2 + 1] <= '9')
					v += token.value[c * 2 + 1] - '0';
				else if (token.value[c * 2 + 1] >= 'a' && token.value[c * 2 + 1] <= 'f')
					v += token.value[c * 2 + 1] - 'a' + 10;
				else if (token.value[c * 2 + 1] >= 'A' && token.value[c * 2 + 1] <= 'F')
					v += token.value[c * 2 + 1] - 'A' + 10;
				else
					return false;

				channels[c] = v / 255.0f;
			}
			out_color = Colorf(channels[0], channels[1], channels[2], channels[3]);
			in_out_pos = pos;
			return true;
		}
	}

	return false;
}
