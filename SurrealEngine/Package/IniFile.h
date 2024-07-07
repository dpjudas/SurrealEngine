#pragma once

#include "NameString.h"
#include <map>

class IniKey
{
public:
	IniKey() = default;
	IniKey(const std::string& _name, const uint32_t _hash);
	IniKey(const IniKey& other);

	uint32_t GetHash() const;
	const std::string& GetName() const;
	const Array<std::string>& GetValues() const;
	bool GetIndexed() const;

	std::string GetValue(const int index = 0) const;

	int SetValue(const std::string& newValue, const int index = 0);
	int SetValues(const Array<std::string>& newValues);

	int SetIndexed(bool newIndexed);

private:
	std::string name;
	uint32_t hash;
	Array<std::string> values;
	bool indexed = false;
};

class IniSection
{
public:
	IniSection() = default;
	IniSection(const std::string& _name, const uint32_t _hash);
	IniSection(const IniSection& other);

	uint32_t GetHash() const;
	const std::string& GetName() const;
	const Array<IniKey>& GetKeys() const;

	std::string GetValue(const NameString& keyName, const std::string& defaultValue, const int index = 0) const;
	Array<std::string> GetValues(const NameString& keyName, const Array<std::string>& defaultValues = {}) const;

	bool SetValue(const NameString& keyName, const std::string& newValue, const int index = 0, const bool indexed = false);
	bool SetValues(const NameString& keyName, const Array<std::string>& newValues, const bool indexed = false);

private:
	std::string name;
	uint32_t hash;
	Array<IniKey> keys;
};

class IniFile
{
public:
	IniFile() = default;
	IniFile(const std::string& filename);
	IniFile(const IniFile& other);

	bool IsModified() const { return isModified; }

	Array<NameString> GetKeys(const NameString& sectionName) const;
	std::string GetValue(const NameString& sectionName, const NameString& keyName, const std::string& defaultValue = "", const int index = 0) const;
	Array<std::string> GetValues(const NameString& sectionName, const NameString& keyName, const Array<std::string>& defaultValues = {}) const;

	void SetValue(const NameString& sectionName, const NameString& keyName, const std::string& newValue, const int index = 0);
	void SetValues(const NameString& sectionName, const NameString& keyName, const Array<std::string>& newValues);

	// Saves values to the ini file the said values are loaded from
	void SaveTo();
	// Saves values to a specified file
	void SaveTo(const std::string& filename);

	// Compares the text written in the ini file with what's in the memory, and updates the changed sections
	// (File is still completely replaced, but this way the previous additional data in it won't be lost)
	void UpdateFile();
	// Updates values in a specified file
	void UpdateFile(const std::string& filename);

	// Updates the file with the changed values if it exists, otherwise saves the values to a new file instead
	void UpdateIfExists(const std::string& filename);

private:
	bool ReadLine(const std::string& text, size_t& pos, std::string& line);

	// Adds a new section, or gets the existing section
	IniSection& AddUniqueSection(const std::string& sectionName);
	const IniSection* FindSection(const std::string& sectionName) const;

	bool isModified = false;
	std::string ini_file_path;
	Array<IniSection> sections;
};