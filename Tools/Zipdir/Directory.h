#pragma once

#include <vector>
#include <string>

class Directory
{
public:
	static std::vector<std::string> files(const std::string& filename);
	static std::vector<std::string> folders(const std::string& filename);
	static std::string exePath();
	static std::string currentDirectory();
	static std::string localAppData();
	static void create(const std::string& path);
	static bool trySetHidden(const std::string& path);
};
