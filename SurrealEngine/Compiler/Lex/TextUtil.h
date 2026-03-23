
#pragma once

#include <string>
#include "Compiler/Frontend/CompilerException.h"

class TextException : public CompilerException
{
public:
	TextException(const std::string &message) : CompilerException(message) { }
};

class TextUtil
{
public:
	static std::u32string utf8_to_utf32(const std::string &utf8);
	static std::string utf32_to_utf8(const std::u32string &utf32);

private:
	static const char trailing_bytes_for_utf8[256];
	static const unsigned char bitmask_leadbyte_for_utf8[6];
};
