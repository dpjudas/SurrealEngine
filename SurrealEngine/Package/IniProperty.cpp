#include "IniProperty.h"
#include <stdexcept>

template<typename T> void IniProperty<T>::FromString(const std::string& valueString)
{
}

template<> void IniProperty<float>::FromString(const std::string& valueString)
{
	if (valueString.empty())
		throw std::runtime_error("Empty value received. Expected: float");

	value = std::stof(valueString);
}

template<> void IniProperty<int>::FromString(const std::string& valueString)
{
	if (valueString.empty())
		throw std::runtime_error("Empty value received. Expected: int");

	value = std::stoi(valueString);
}

template<> void IniProperty<uint8_t>::FromString(const std::string& valueString)
{
	if (valueString.empty())
		throw std::runtime_error("Empty value received. Expected: uint8");

	value = std::stoi(valueString);
}

template<> void IniProperty<std::string>::FromString(const std::string& valueString)
{
	// Lmao
	value = valueString;
}

template<> void IniProperty<bool>::FromString(const std::string& valueString)
{
	if (valueString.empty())
		throw std::runtime_error("Empty value received. Expected: boolean");

	if (valueString == "True" || valueString == "true" || valueString == "1")
		value = true;
	else if (valueString == "False" || valueString == "false" || valueString == "0")
		value = false;
	else
		throw std::runtime_error("Received a non-boolean value: " + valueString);
}