#include "IniProperty.h"
#include <stdexcept>

template<>
int IniPropertyConverter<int>::FromString(const std::string& valueString)
{
	return std::stoi(valueString);
}

template<>
float IniPropertyConverter<float>::FromString(const std::string& valueString)
{
	return std::stof(valueString);
}

template<>
uint8_t IniPropertyConverter<uint8_t>::FromString(const std::string& valueString)
{
	return (uint8_t)std::stoul(valueString);
}

template<>
bool IniPropertyConverter<bool>::FromString(const std::string& valueString)
{
	if (valueString == "True" || valueString == "true" || valueString == "1")
		return true;
	else if (valueString == "False" || valueString == "false" || valueString == "0")
		return false;
	else
		throw std::runtime_error("Encountered non-boolean value: " + valueString);
}

template<>
std::string IniPropertyConverter<std::string>::FromString(const std::string& valueString)
{
	return valueString;
}

