
#pragma once

#include <string>

class CompilerMessage
{
public:
	enum Type
	{
		error,
		warning,
		info
	};

	CompilerMessage(Type type, const std::string &text, const std::string &filename = std::string(), int line = 0) : type(type), text(text), filename(filename), line(line) { }

	Type type;
	std::string text;
	std::string filename;
	int line;

	std::string to_string() const
	{
		std::string str = filename;
		if (line != 0)
		{
			str += "(";
			str += std::to_string(line);
			str += ")";
		}
		if (!str.empty())
			str += ": ";
		switch (type)
		{
		case error:
			str += "error: ";
			break;
		case warning:
			str += "warning: ";
			break;
		default:
			break;
		}
		str += text;
		return str;
	}
};
