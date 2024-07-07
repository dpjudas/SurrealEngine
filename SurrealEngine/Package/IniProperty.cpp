#include "IniProperty.h"
#include "Utils/Exception.h"
#include "Utils/Convert.h"

template<>
int IniPropertyConverter<int>::FromString(const std::string& valueString)
{
	return Convert::to_int32(valueString);
}

template<>
std::string IniPropertyConverter<int>::ToString(const int& value)
{
	return std::to_string(value);
}

template<>
int IniPropertyConverter<int>::FromIniFile(const IniFile& iniFile, const NameString& section, const NameString& keyName, const int& default_value, const int index)
{
	return Convert::to_int32(iniFile.GetValue(section, keyName, ToString(default_value), index));
}

//====================================================================

template<>
float IniPropertyConverter<float>::FromString(const std::string& valueString)
{
	return Convert::to_float(valueString);
}

template<>
std::string IniPropertyConverter<float>::ToString(const float& value)
{
	return std::to_string(value);
}

template<>
float IniPropertyConverter<float>::FromIniFile(const IniFile& iniFile, const NameString& section, const NameString& keyName, const float& default_value, const int index)
{
	return Convert::to_float(iniFile.GetValue(section, keyName, ToString(default_value), index));
}

//====================================================================

template<>
uint8_t IniPropertyConverter<uint8_t>::FromString(const std::string& valueString)
{
	return Convert::to_uint8(valueString);
}

template<>
std::string IniPropertyConverter<uint8_t>::ToString(const uint8_t& value)
{
	return std::to_string(value);
}

template<>
uint8_t IniPropertyConverter<uint8_t>::FromIniFile(const IniFile& iniFile, const NameString& section, const NameString& keyName, const uint8_t& default_value, const int index)
{
	return Convert::to_uint8(iniFile.GetValue(section, keyName, ToString(default_value), index));
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
		Exception::Throw("Encountered a non-boolean value: " + valueString);
}


template<>
std::string IniPropertyConverter<bool>::ToString(const bool& value)
{
	return value ? "True" : "False";
}

template<>
bool IniPropertyConverter<bool>::FromIniFile(const IniFile& iniFile, const NameString& section, const NameString& keyName, const bool& default_value, const int index)
{
	return FromString(iniFile.GetValue(section, keyName, ToString(default_value), index));
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
	return iniFile.GetValue(section, keyName, default_value, index);
}

//====================================================================

template<>
AudioFrequency IniPropertyConverter<AudioFrequency>::FromString(const std::string& valueString)
{
	if (auto pos = valueString.find("Hz"); pos != std::string::npos)
	{
		int frequency = Convert::to_int32(valueString.substr(0, pos));
		return AudioFrequency(frequency);
	}
	Exception::Throw("Invalid value '" + valueString + "' for changing frequency. The value must be a number that ends with 'Hz'.");
}

template<>
std::string IniPropertyConverter<AudioFrequency>::ToString(const AudioFrequency& value)
{
	return std::to_string(value.frequency) + "Hz";
}

template<>
AudioFrequency IniPropertyConverter<AudioFrequency>::FromIniFile(const IniFile& iniFile, const NameString& section, const NameString& keyName, const AudioFrequency& default_value, const int index)
{
	return FromString(iniFile.GetValue(section, keyName, ToString(default_value), index));
}
