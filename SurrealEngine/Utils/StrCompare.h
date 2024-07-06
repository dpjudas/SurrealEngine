#pragma once

#include <string>
#include <string_view>

class StrCompare
{
public:
	static bool equals_ignore_case(const std::string& str1, const std::string& str2);
	static bool equals_ignore_case(const std::string& str1, const char* str2);
	static bool equals_ignore_case(const std::string_view& str1, const std::string_view& str2);
};
