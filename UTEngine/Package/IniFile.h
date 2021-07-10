#pragma once

class IniFile
{
public:
	IniFile() = default;
	IniFile(const std::string& filename);

	std::string GetValue(std::string sectionName, std::string keyName) const;

private:
	bool ReadLine(const std::string& text, size_t& pos, std::string& line);

	std::map<std::string, std::map<std::string, std::string>> sections;
};
