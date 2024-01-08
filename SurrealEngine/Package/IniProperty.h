#pragma once

#include <string>

#include "IniFile.h"

template <typename T>
class IniProperty
{
public:
	IniProperty(T& value) : value(value) {}
	IniProperty(T&& value) : value(value) {}
	IniProperty(const std::string& valueString) { FromString(valueString); }
	IniProperty(const IniFile& iniFile, const NameString& section, const NameString& keyName) { FromIniFile(iniFile, section, keyName); }

	T Value() const { return value; }
	void FromString(const std::string& valueString);
	void FromIniFile(const IniFile& iniFile, const NameString& section, const NameString& keyName)
	{
		FromString(iniFile.GetValue(section, keyName, ToString()));
	}

	std::string ToString() const { return std::to_string(value); }

	void operator=(const T& other) { value = other; }
	bool operator==(const T& other) const { return value == other; }
	bool operator!=(const T& other) const { return value != other; }
	bool operator!() const { return !value; }

private:
	T value;
};

template<> std::string IniProperty<bool>::ToString() const 
{
	return value ? "True" : "False";
}

template<> void IniProperty<std::string>::FromIniFile(const IniFile& iniFile, const NameString& section, const NameString& keyName)
{
	value = iniFile.GetValue(section, keyName, value);
}