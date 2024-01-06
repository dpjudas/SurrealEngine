#pragma once

#include "NameString.h"

class IniFile
{
public:
	IniFile() = default;
	IniFile(const std::string& filename);

	bool IsModified() const { return isModified; }

	std::vector<NameString> GetKeys(NameString sectionName) const;
	std::string GetValue(NameString sectionName, NameString keyName) const;
	std::vector<std::string> GetValues(NameString sectionName, NameString keyName) const;

	void SetValue(NameString sectionName, NameString keyName, const std::string& newValue);
	void SetValues(NameString sectionName, NameString keyName, const std::vector<std::string>& newValues);

	// Saves values to the ini file the said values are loaded from
	void SaveTo();
	// Saves values to a specified file
	void SaveTo(const std::string& filename); 

private:
	bool ReadLine(const std::string& text, size_t& pos, std::string& line);

	bool isModified = false;

	std::string ini_file_path;
	std::map<NameString, std::map<NameString, std::vector<std::string>>> sections;
};
