#pragma once

#include "NameString.h"

class IniFile
{
public:
	IniFile() = default;
	IniFile(const std::string& filename);

	std::string GetValue(NameString sectionName, NameString keyName) const;
	std::vector<std::string> GetValues(NameString sectionName, NameString keyName) const;

private:
	bool ReadLine(const std::string& text, size_t& pos, std::string& line);

	std::map<NameString, std::map<NameString, std::vector<std::string>>> sections;
};
