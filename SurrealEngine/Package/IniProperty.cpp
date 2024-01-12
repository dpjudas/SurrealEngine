#include "IniProperty.h"
#include <stdexcept>

template<>
int IniPropertyConverter<int>::FromString(const std::string& valueString)
{
	return std::stoi(valueString);
}

template<>
std::string IniPropertyConverter<int>::ToString(const int& value)
{
	return std::to_string(value);
}

template<>
int IniPropertyConverter<int>::FromIniFile(const IniFile& iniFile, const NameString& section, const NameString& keyName, const int& default_value, const int index)
{
	return std::stoi(iniFile.GetValue(section, keyName, index, ToString(default_value)));
}

//====================================================================

template<>
float IniPropertyConverter<float>::FromString(const std::string& valueString)
{
	return std::stof(valueString);
}

template<>
std::string IniPropertyConverter<float>::ToString(const float& value)
{
	return std::to_string(value);
}

template<>
float IniPropertyConverter<float>::FromIniFile(const IniFile& iniFile, const NameString& section, const NameString& keyName, const float& default_value, const int index)
{
	return std::stof(iniFile.GetValue(section, keyName, index, ToString(default_value)));
}

//====================================================================

template<>
uint8_t IniPropertyConverter<uint8_t>::FromString(const std::string& valueString)
{
	return (uint8_t)std::stoul(valueString);
}

template<>
std::string IniPropertyConverter<uint8_t>::ToString(const uint8_t& value)
{
	return std::to_string(value);
}

template<>
uint8_t IniPropertyConverter<uint8_t>::FromIniFile(const IniFile& iniFile, const NameString& section, const NameString& keyName, const uint8_t& default_value, const int index)
{
	return std::stoul(iniFile.GetValue(section, keyName, index, ToString(default_value)));
}

//====================================================================

template<>
bool IniPropertyConverter<bool>::FromString(const std::string& valueString)
{
	if (valueString == "True" || valueString == "true" || valueString == "1")
		return true;
	else if (valueString == "False" || valueString == "false" || valueString == "0")
		return false;
	else
		throw std::invalid_argument("Encountered a non-boolean value: " + valueString);
}


template<>
std::string IniPropertyConverter<bool>::ToString(const bool& value)
{
	return value ? "True" : "False";
}

template<>
bool IniPropertyConverter<bool>::FromIniFile(const IniFile& iniFile, const NameString& section, const NameString& keyName, const bool& default_value, const int index)
{
	return FromString(iniFile.GetValue(section, keyName, index, ToString(default_value)));
}

//====================================================================

template<>
std::string IniPropertyConverter<std::string>::FromString(const std::string& valueString)
{
	return valueString;
}

template<>
std::string IniPropertyConverter<std::string>::ToString(const std::string& value)
{
	return value;
}

template<>
std::string IniPropertyConverter<std::string>::FromIniFile(const IniFile& iniFile, const NameString& section, const NameString& keyName, const std::string& default_value, const int index)
{
	return iniFile.GetValue(section, keyName, index, default_value);
}

//====================================================================

template<>
AudioFrequency IniPropertyConverter<AudioFrequency>::FromString(const std::string& valueString)
{
	if (auto pos = valueString.find("Hz"); pos != std::string::npos)
	{
		int frequency = std::stoi(valueString.substr(0, pos));
		return AudioFrequency(frequency);
	}
	throw std::invalid_argument("Invalid value '" + valueString + "' for changing frequency. The value must be a number that ends with 'Hz'.");
}

template<>
std::string IniPropertyConverter<AudioFrequency>::ToString(const AudioFrequency& value)
{
	return std::to_string(value.frequency) + "Hz";
}

template<>
AudioFrequency IniPropertyConverter<AudioFrequency>::FromIniFile(const IniFile& iniFile, const NameString& section, const NameString& keyName, const AudioFrequency& default_value, const int index)
{
	return FromString(iniFile.GetValue(section, keyName, index, ToString(default_value)));
}
