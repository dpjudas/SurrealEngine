#pragma once

#include <cstdint>

class Convert
{
public:
	// String-to-number
	static int32_t to_int32(const std::string& str);
	static uint32_t to_uint32(const std::string& str);
	static int16_t to_int16(const std::string& str);
	static uint16_t to_uint16(const std::string& str);
	static int8_t to_int8(const std::string& str);
	static uint8_t to_uint8(const std::string& str);

	static float to_float(const std::string& str);
	static double to_double(const std::string& str);

	// String manipulations
	// These make a copy of the string with the desired operation applied
	std::string as_lowercase(const std::string& str);
	std::string as_uppercase(const std::string& str);
};
