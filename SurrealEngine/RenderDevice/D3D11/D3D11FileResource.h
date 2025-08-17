#pragma once

#include <string>

class D3D11FileResource
{
public:
	static std::string readAllText(const std::string& filename);
};
