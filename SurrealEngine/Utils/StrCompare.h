#pragma once

#include <string>

class StrCompare
{
public:
	static int ignore_case(const std::string& str1, const std::string& str2);
	static int ignore_case(const std::string& str1, const char* str2);
	static bool equals_ignore_case(const std::string& str1, const std::string& str2);
	static bool equals_ignore_case(const std::string& str1, const char* str2);
};
