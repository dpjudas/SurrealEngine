#pragma once

#include <string>

class FilePath
{
public:
	static bool hasExtension(const std::string& filename, const char* extension);
	static std::string extension(const std::string& filename);
	static std::string removeExtension(const std::string& filename);

	static std::string firstComponent(const std::string& path);
	static std::string removeFirstComponent(const std::string& path);

	static std::string lastComponent(const std::string& path);
	static std::string removeLastComponent(const std::string& path);

	static std::string combine(const std::string& path1, const std::string& path2);
	static std::string combine(std::string path1, const std::initializer_list<std::string>& paths);

	static std::string normalizePathDelimiters(std::string path);
	static std::string forceSlash(std::string path);
	static std::string forceBackslash(std::string path);
};
