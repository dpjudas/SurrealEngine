
#pragma once

#include <exception>
#include <string>

class CompilerException : public std::exception
{
public:
	CompilerException(const std::string &message) : _message(message) { }
	const std::string &message() const { return _message; }

	const char *what() const noexcept override { return _message.c_str(); }

private:
	std::string _message;
};
