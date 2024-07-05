#include "Convert.h"
#include "Exception.h"

#include <string>
#include <algorithm>

int32_t Convert::to_int32(const std::string& str)
{
	try
	{
		return std::stoi(str);
	}
	catch (std::exception& e)
	{
		Exception::Throw(e.what() + std::string(" (value = ") + str + ")");
	}
}

uint32_t Convert::to_uint32(const std::string& str)
{
	try
	{
		return (uint32_t) std::stoi(str);
	}
	catch (std::exception& e)
	{
		Exception::Throw(e.what() + std::string(" (value = ") + str + ")");
	}
}

int16_t Convert::to_int16(const std::string& str)
{
	try
	{
		return (int16_t)std::stoi(str);
	}
	catch (std::exception& e)
	{
		Exception::Throw(e.what() + std::string(" (value = ") + str + ")");
	}
}

uint16_t Convert::to_uint16(const std::string& str)
{
	try
	{
		return (uint16_t)std::stoi(str);
	}
	catch (std::exception& e)
	{
		Exception::Throw(e.what() + std::string(" (value = ") + str + ")");
	}
}

int8_t Convert::to_int8(const std::string& str)
{
	try
	{
		return (int8_t)std::stoi(str);
	}
	catch (std::exception& e)
	{
		Exception::Throw(e.what() + std::string(" (value = ") + str + ")");
	}
}

uint8_t Convert::to_uint8(const std::string& str)
{
	try
	{
		return (uint8_t)std::stoi(str);
	}
	catch (std::exception& e)
	{
		Exception::Throw(e.what() + std::string(" (value = ") + str + ")");
	}
}

float Convert::to_float(const std::string& str)
{
	try
	{
		return std::stof(str);
	}
	catch (std::exception& e)
	{
		Exception::Throw(e.what() + std::string(" (value = ") + str + ")");
	}
}

double Convert::to_double(const std::string& str)
{
	try
	{
		return std::stod(str);
	}
	catch (std::exception& e)
	{
		Exception::Throw(e.what() + std::string(" (value = ") + str + ")");
	}
}
