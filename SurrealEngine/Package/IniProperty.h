#pragma once

#include <string>

#include "IniFile.h"

template <typename T>
class IniPropertyConverter
{
public:
	static std::string ToString(const T& value) { return std::to_string(value); }
	static T FromString(const std::string& valueString);
	static T FromString(const char* valueString) { return FromString(std::string(valueString)); }
	static T FromIniFile(const IniFile& iniFile, const NameString& section, const NameString& keyName)
	{
		return FromString(iniFile.GetValue(section, keyName));
	}

	IniPropertyConverter() = delete;
};
