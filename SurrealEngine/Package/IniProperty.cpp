#include "IniProperty.h"
#include <stdexcept>

template<>
int IniPropertyConverter<int>::FromString(const std::string& valueString)
{
	return std::stoi(valueString);
}

template<>
int IniPropertyConverter<int>::FromIniFile(const IniFile& iniFile, const NameString& section, const NameString& keyName, const int& default_value)
{
	return std::stoi(iniFile.GetValue(section, keyName, ToString(default_value)));
}

template<>
float IniPropertyConverter<float>::FromString(const std::string& valueString)
{
	return std::stof(valueString);
}

template<>
float IniPropertyConverter<float>::FromIniFile(const IniFile& iniFile, const NameString& section, const NameString& keyName, const float& default_value)
{
	return std::stof(iniFile.GetValue(section, keyName, ToString(default_value)));
}

template<>
uint8_t IniPropertyConverter<uint8_t>::FromString(const std::string& valueString)
{
	return (uint8_t)std::stoul(valueString);
}

template<>
uint8_t IniPropertyConverter<uint8_t>::FromIniFile(const IniFile& iniFile, const NameString& section, const NameString& keyName, const uint8_t& default_value)
{
	return std::stoul(iniFile.GetValue(section, keyName, ToString(default_value)));
}

template<>
bool IniPropertyConverter<bool>::FromString(const std::string& valueString)
{
	if (valueString == "True" || valueString == "true" || valueString == "1")
		return true;
	else if (valueString == "False" || valueString == "false" || valueString == "0")
		return false;
	else
		throw std::runtime_error("Encountered a non-boolean value: " + valueString);
}

template<>
bool IniPropertyConverter<bool>::FromIniFile(const IniFile& iniFile, const NameString& section, const NameString& keyName, const bool& default_value)
{
	return FromString(iniFile.GetValue(section, keyName, ToString(default_value)));
}

template<>
std::string IniPropertyConverter<std::string>::FromString(const std::string& valueString)
{
	return valueString;
}

template<>
std::string IniPropertyConverter<std::string>::FromIniFile(const IniFile& iniFile, const NameString& section, const NameString& keyName, const std::string& default_value)
{
	return iniFile.GetValue(section, keyName, default_value);
}

template<>
AudioFrequency IniPropertyConverter<AudioFrequency>::FromString(const std::string& valueString)
{
	if (auto pos = valueString.find("Hz"); pos != std::string::npos)
	{
		int frequency = std::stoi(valueString.substr(0, pos));
		return AudioFrequency(frequency);
	}
	throw std::runtime_error("Invalid value " + valueString + " for changing frequency.");
}

template<>
AudioFrequency IniPropertyConverter<AudioFrequency>::FromIniFile(const IniFile& iniFile, const NameString& section, const NameString& keyName, const AudioFrequency& default_value)
{
	return FromString(iniFile.GetValue(section, keyName, ToString(default_value)));
}
