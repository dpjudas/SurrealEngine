
#include "Precomp.h"
#include "IniFile.h"
#include "File.h"
#include "MurmurHash3/MurmurHash3.h"
#include <algorithm>

static uint32_t HashIniString(const std::string& str)
{
	std::string strLower;
	strLower.resize(str.size());
	std::transform(str.begin(), str.end(), strLower.begin(), [](unsigned char c) { return tolower(c); });

	uint32_t hash = 0;
	MurmurHash3_x86_32(strLower.c_str(), (int)strLower.length(), 0, &hash);

	return hash;
}

IniFile::IniFile(const std::string& filename)
{
	std::string text = File::read_all_text(filename);
	size_t pos = 0;
	std::string line;
	std::string sectionName;
	while (ReadLine(text, pos, line))
	{
		if (line.empty() || line[0] == ';')
			continue;

		if (line.size() >= 2 && line.front() == '[' && line.back() == ']')
		{
			sectionName = line.substr(1, line.size() - 2);
		}
		else
		{
			size_t equalpos = line.find('=');
			if (equalpos != std::string::npos)
			{
				std::string name = line.substr(0, equalpos);
				while (!name.empty() && (name.back() == ' ' || name.back() == '\t'))
					name.pop_back();

				std::string value;
				size_t valuepos = line.find_first_not_of(" \t", equalpos + 1);
				if (valuepos != std::string::npos)
					value = line.substr(valuepos);

				if (!name.empty())
				{
					size_t bracket = name.find('[');
					int index = -1;
					bool indexed = false;
					if (bracket != std::string::npos)
					{
						size_t rightBracket = name.find(']');
						if (rightBracket == std::string::npos)
							Exception::Throw("malformed INI array index");

						indexed = true;
						index = std::stoi(name.substr(bracket + 1, rightBracket - bracket - 1));
						name = name.substr(0, bracket);
					}

					IniSection& section = AddUniqueSection(sectionName);
					section.SetValue(name, value, index, indexed);
				}
			}
		}
	}

	ini_file_path = filename;
}

IniFile::IniFile(const IniFile& other)
{
	ini_file_path = other.ini_file_path;
	sections = other.sections;
}

bool IniFile::ReadLine(const std::string& text, size_t& pos, std::string& line)
{
	// Skip whitespace
	while (pos < text.size() && (text[pos] == ' ' || text[pos] == '\t' || text[pos] == '\r' || text[pos] == '\n'))
		pos++;
	if (pos >= text.size())
		return false;

	size_t startpos = pos;

	// Read until end of line
	while (pos < text.size() && text[pos] != '\r' && text[pos] != '\n')
		pos++;

	size_t endpos = pos;

	// Skip whitespace at end of line
	while (endpos > startpos && (text[endpos - 1] == ' ' || text[endpos - 1] == '\t' || text[endpos - 1] == '\r' || text[endpos - 1] == '\n'))
		endpos--;

	// Read newline
	while (pos < text.size() && (text[pos] == '\r' || text[pos] == '\n'))
		pos++;

	line = text.substr(startpos, endpos - startpos);
	return true;
}

std::vector<NameString> IniFile::GetKeys(const NameString& sectionName) const
{
	std::vector<NameString> result;

	const IniSection* section = FindSection(sectionName.ToString());
	if (section != nullptr)
	{
		auto& keys = section->GetKeys();
		for (const IniKey& key : keys)
			result.push_back(key.GetName());
	}

	return result;
}

std::string IniFile::GetValue(const NameString& sectionName, const NameString& keyName, const std::string& defaultValue, const int index) const
{
	const IniSection* section = FindSection(sectionName.ToString());
	if (section == nullptr)
		return defaultValue;

	return section->GetValue(keyName, defaultValue, index);
}

std::vector<std::string> IniFile::GetValues(const NameString& sectionName, const NameString& keyName, const std::vector<std::string>& defaultValues) const
{
	const IniSection* section = FindSection(sectionName.ToString());
	if (section == nullptr)
		return defaultValues;

	return section->GetValues(keyName, defaultValues);
}

void IniFile::SetValue(const NameString& sectionName, const NameString& keyName, const std::string& newValue, const int index)
{
	IniSection& section = AddUniqueSection(sectionName.ToString());
	if (section.SetValue(keyName, newValue, index))
		isModified = true;
}

void IniFile::SetValues(const NameString& sectionName, const NameString& keyName, const std::vector<std::string>& newValues)
{
	IniSection& section = AddUniqueSection(sectionName.ToString());
	if (section.SetValues(keyName, newValues))
		isModified = true;
}

void IniFile::SaveTo()
{
	SaveTo(ini_file_path);
}

void IniFile::SaveTo(const std::string& filename)
{
	if (filename == ini_file_path && !isModified)
		return;

	std::string ini_text = "";

	// Start with sections first
	for (auto& section : sections)
	{
		// Section header (i.e. [Engine.Engine])
		std::string section_text = "[" + section.GetName() + "]\n";

		// key=value pairs
		auto& keys = section.GetKeys();
		for (auto& key : keys)
		{
			// a key can hold multiple values, like
			// Paths=path1
			// Paths=path2
			// and so on, hence this loop
			auto& values = key.GetValues();
			for (auto& value : values)
			{
				section_text += key.GetName() + "=" + value + "\n";
			}
		}

		ini_text += section_text + "\n";
	}

	// Overwrite whatever is there
	File::write_all_text(filename, ini_text);

	if (filename != ini_file_path)
		ini_file_path = filename;

	isModified = false;
}

void IniFile::UpdateFile()
{
	UpdateFile(ini_file_path);
}

void IniFile::UpdateFile(const std::string& filename)
{
	if (filename == ini_file_path && !isModified)
		return;

	// The logic is simple: Read the file, check each line one-by-one, modify the differing lines, and write everything back
	// The implementation however, is not :V
	auto lines = File::read_all_lines(filename);

	const IniSection* current_section = nullptr;

	// A key can contain multiple values, so we gotta compare if we're getting the same key again and again
	struct KeyOccurance
	{
		std::string keyName = "";
		unsigned int currentIndex = 0;
	};

	std::vector<KeyOccurance> keyOccurrances;

	bool key_has_brackets = false;
	std::string ini_value;			// Value read from the ini file

	for (auto line_it = lines.begin() ; line_it != lines.end() ; line_it++)
	{
		std::string line = *line_it;

		if (line.empty() || line[0] == ';')
			// Do nothing on empty line
			continue;

		if (line.front() == '[' && line.back() == ']')
		{
			// If we're switching from a different section, check for values not in the file, but in the memory, and add them
			if (current_section)
			{
				auto& section_keys = current_section->GetKeys();

				for (auto& key : section_keys)
				{
					bool key_found = false;
					for (auto& keyOccurrance : keyOccurrances)
					{
						if (key.GetName() == keyOccurrance.keyName)
						{
							key_found = true;
							break;
						}
					}

					if (key_found)
						continue;

					auto& values = key.GetValues();

					for (auto& value : values)
					{
						std::string newline = key.GetName() + "=" + value;

						line_it = lines.insert(line_it - 2, newline);

						line_it += 2;
					}
				}
			}

			// Switch sections
			std::string new_section_name = line.substr(1, line.length() - 2);

			current_section = FindSection(new_section_name);

			keyOccurrances.clear();

			continue;
		}

		// Check stuff out only if we're in a relevant section
		if (current_section)
		{
			size_t equal_pos = line.find('=');

			if (equal_pos != std::string::npos)
			{
				std::string key = line.substr(0, equal_pos);
				std::string key_without_brackets = key; // Will differ from key if the said key contains brackets
				std::string value_from_file = line.substr(equal_pos + 1);

				size_t left_bracket_pos = key.find('['),
					   right_bracket_pos = key.find(']');

				int bracket_index = 0;

				if (left_bracket_pos != std::string::npos && right_bracket_pos != std::string::npos && left_bracket_pos < right_bracket_pos)
				{
					bracket_index = std::stoi(key.substr(left_bracket_pos, right_bracket_pos - left_bracket_pos + 1));
					key_without_brackets = key.substr(0, left_bracket_pos);
					key_has_brackets = true;
				}

				// If the key don't have any brackets, check if we encountered the said key before
				// If we did, then get the next index from it, else add the key we found to the database
				int found_index = 0;
				bool occurance_found = false;

				for (auto& keyOccurrance : keyOccurrances)
				{
					if (keyOccurrance.keyName == key_without_brackets)
					{
						keyOccurrance.currentIndex++;
						occurance_found = true;
						break;
					}

					found_index++;
				}

				if (!occurance_found)
				{
					// Must be a new occurrance
					keyOccurrances.push_back({ key, 0 });
				}

				// Read and compare the value of the current line
				if (key_has_brackets)
					ini_value = current_section->GetValue(key_without_brackets, "", bracket_index);
				else
					ini_value = current_section->GetValue(key, "", occurance_found ? keyOccurrances[found_index].currentIndex : 0);
				
				// Update the current line if the value differs
				if (value_from_file != ini_value)
					*line_it = key + "=" + ini_value;
			}
		}
	}

	// Combine lines back
	std::string final_text = "";

	for (auto& line : lines)
	{
		final_text += line;

		if (line != "\n")
			final_text += "\n";
	}
	
	File::write_all_text(filename, final_text);

	if (filename != ini_file_path)
		ini_file_path = filename;

	isModified = false;
}

void IniFile::UpdateIfExists(const std::string& filename)
{
	if (File::try_open_existing(filename))
		UpdateFile(filename);
	else
		SaveTo(filename);
}

IniSection& IniFile::AddUniqueSection(const std::string& sectionName)
{
	uint32_t sectionHash = HashIniString(sectionName);
	for (auto& section : sections)
	{
		if (section.GetHash() == sectionHash)
			return section;
	}

	sections.push_back(std::move(IniSection(sectionName, sectionHash)));
	return sections.back();
}

const IniSection* IniFile::FindSection(const std::string& sectionName) const
{
	uint32_t sectionHash = HashIniString(sectionName);
	for (auto& section : sections)
	{
		if (section.GetHash() == sectionHash)
			return &section;
	}

	return nullptr;
}

//====================================================================

IniSection::IniSection(const std::string& _name, const uint32_t _hash)
{
	name = _name;
	hash = _hash;
}

IniSection::IniSection(const IniSection& other)
{
	name = other.name;
	hash = other.hash;
	keys = other.keys;
}

const std::string& IniSection::GetName() const
{
	return name;
}

uint32_t IniSection::GetHash() const
{
	return hash;
}

const std::vector<IniKey>& IniSection::GetKeys() const
{
	return keys;
}

std::string IniSection::GetValue(const NameString& keyName, const std::string& defaultValue, const int index) const
{
	uint32_t keyHash = HashIniString(keyName.ToString());
	for (auto& key : keys)
	{
		if (key.GetHash() == keyHash)
		{
			std::string value = key.GetValue(index);
			if (value.size() == 0)
				break;

			return value;
		}
	}

	return defaultValue;
}

std::vector<std::string> IniSection::GetValues(const NameString& keyName, const std::vector<std::string>& defaultValues) const
{
	uint32_t keyHash = HashIniString(keyName.ToString());
	for (auto& key : keys)
	{
		if (key.GetHash() == keyHash)
			return key.GetValues();
	}

	return defaultValues;
}

bool IniSection::SetValue(const NameString& keyName, const std::string& newValue, const int index, const bool indexed)
{
	int result = -1;
	uint32_t keyHash = HashIniString(keyName.ToString());
	for (auto& key : keys)
	{
		if (key.GetHash() == keyHash)
		{
			result = key.SetValue(newValue, index);
			result |= key.SetIndexed(indexed);
			break;
		}
	}

	if (result < 0)
	{
		IniKey newKey(keyName.ToString(), keyHash);
		newKey.SetValue(newValue, index);
		newKey.SetIndexed(indexed);
		keys.push_back(std::move(newKey));
		return true;
	}

	return result == 1;
}

bool IniSection::SetValues(const NameString& keyName, const std::vector<std::string>& newValues, const bool indexed)
{
	int result = -1;
	uint32_t keyHash = HashIniString(keyName.ToString());
	for (auto& key : keys)
	{
		if (key.GetHash() == keyHash)
		{
			result = key.SetValues(newValues);
			result |= key.SetIndexed(indexed);
			break;
		}
	}

	if (result < 0)
	{
		IniKey newKey(keyName.ToString(), keyHash);
		newKey.SetValues(newValues);
		newKey.SetIndexed(indexed);
		keys.push_back(std::move(newKey));
		return true;
	}

	return result == 1;
}

//====================================================================

IniKey::IniKey(const std::string& _name, const uint32_t _hash)
{
	name = _name;
	hash = _hash;
}

IniKey::IniKey(const IniKey& other)
{
	name = other.name;
	hash = other.hash;
	values = other.values;
}

uint32_t IniKey::GetHash() const
{
	return hash;
}

const std::string& IniKey::GetName() const
{
	return name;
}

const std::vector<std::string>& IniKey::GetValues() const
{
	return values;
}

bool IniKey::GetIndexed() const
{
	return indexed;
}

std::string IniKey::GetValue(const int index) const
{
	std::string value = "";
	if (index < values.size())
		value = values[index];
	return value;
}

int IniKey::SetValue(const std::string& newValue, const int index)
{
	if (index == -1)
	{
		values.push_back(newValue);
		return 1;
	}
	else if (index < values.size())
	{
		if (values[index] == newValue)
			return 0;
	}
	else
	{
		values.resize(index + 1);
	}

	values[index] = newValue;
	return 1;
}

int IniKey::SetValues(const std::vector<std::string>& newValues)
{
	if (newValues == values)
		return 0;

	values = newValues;
	return 1;
}

int IniKey::SetIndexed(bool newIndexed)
{
	bool result = !((int)indexed ^ (int)newIndexed);
	indexed = newIndexed;
	return result;
}