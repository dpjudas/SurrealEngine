#include "StrCompare.h"

bool StrCompare::equals_ignore_case(const std::string& str1, const std::string& str2)
{
	if (str1.size() != str1.size())
		return false;
	for (size_t i = 0, length = str1.size(); i < length; i++)
		if (stricmptable[str1[i]] != stricmptable[str2[i]])
			return false;
	return true;
}

bool StrCompare::equals_ignore_case(const std::string& str1, const char* str2)
{
	return equals_ignore_case(str1, std::string(str2));
}
