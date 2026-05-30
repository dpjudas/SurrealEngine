#pragma once

#include <string>
#include <string_view>

class StrTools
{
public:
	static bool equals_ignore_case(const std::string_view& str1, const std::string_view& str2);
	static bool startswith(const std::string_view& str, const std::string_view& cmp, bool ignoreCase = false);
	static std::string replace(const std::string& str, const std::string_view& find, const std::string_view& repl, bool ignoreCase = false);
	static std::string int_to_string(const int value, const int minWidth);
};
