#include "IniProperty.h"
#include "Utils/Exception.h"
#include "Utils/Convert.h"
#include "UObject/UObject.h"

#include <sstream>

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

//====================================================================

template<>
Color IniPropertyConverter<Color>::FromString(const std::string& valueString)
{
	if (valueString.size() <= 2 || valueString[0] != '(' || valueString[valueString.size() - 1] != ')')
		Exception::Throw("Invalid Color String: " + valueString);

	// Get rid of ()s
	auto subString = valueString.substr(1, valueString.size() - 2);

	uint8_t r = 0, g = 0, b = 0, a = 0;

	std::stringstream ss(subString);

	std::string t;

	while (getline(ss, t, ','))
	{
		size_t equals = t.find('=');

		if (equals == -1)
			Exception::Throw("Malformed Color String: " + valueString);

		auto lhs = t.substr(0, equals);

		auto rhs = Convert::to_uint8(t.substr(equals + 1));

		if (lhs == "R")
			r = rhs;
		else if (lhs == "G")
			g = rhs;
		else if (lhs == "B")
			b = rhs;
		else if (lhs == "A")
			a = rhs;
		else
			Exception::Throw("Unexpected Color Property: " + lhs);
	}

	return Color {
		.R = r,
		.G = g,
		.B = b,
		.A = a
	};
}

template <>
std::string IniPropertyConverter<Color>::ToString(const Color& value)
{
	return "(R=" + std::to_string(value.R)
		   + ",G=" + std::to_string(value.G)
		   + ",B=" + std::to_string(value.B)
	       + ",A=" + std::to_string(value.A) + ")";
}

template<>
Color IniPropertyConverter<Color>::FromIniFile(const IniFile& iniFile, const NameString& section, const NameString& keyName, const Color& default_value, const int index)
{
	return FromString(iniFile.GetValue(section, keyName, ToString(default_value), index));
}
