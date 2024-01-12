#pragma once

#include <string>

#include "IniFile.h"
#include "UObject/USound.h"

template <typename T>
class IniPropertyConverter
{
public:
	static std::string ToString(const T& value);
	static T FromString(const std::string& valueString);
	static T FromString(const char* valueString) { return FromString(std::string(valueString)); }
	static T FromIniFile(const IniFile& iniFile, const NameString& section, const NameString& keyName, const T& default_value, const int index = 0);

	IniPropertyConverter() = delete;
};