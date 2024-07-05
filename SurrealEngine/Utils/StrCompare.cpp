#include "StrCompare.h"

int StrCompare::ignore_case(const std::string& str1, const std::string& str2)
{
#ifdef WIN32
	return _stricmp(str1.c_str(), str2.c_str());
#else
	return strcasecmp(str1.c_str(), str2.c_str());
#endif
}

int StrCompare::ignore_case(const std::string& str1, const char* str2)
{
#ifdef WIN32
	return _stricmp(str1.c_str(), str2);
#else
	return strcasecmp(str1.c_str(), str2);
#endif
}

bool StrCompare::equals_ignore_case(const std::string& str1, const std::string& str2)
{
	return ignore_case(str1, str2) == 0;
}

bool StrCompare::equals_ignore_case(const std::string& str1, const char* str2)
{
	return ignore_case(str1, str2) == 0;
}
