#pragma once

#include <string>

class FileResource
{
public:
	static std::string readAllText(const std::string& filename);
};
