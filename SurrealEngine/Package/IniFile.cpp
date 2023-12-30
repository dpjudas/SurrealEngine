
#include "Precomp.h"
#include "IniFile.h"
#include "File.h"

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
			for (char& c : sectionName)
				if (c >= 'A' && c <= 'Z')
					c += 'a' - 'A';
		}
		else
		{
			size_t equalpos = line.find('=');
			if (equalpos != std::string::npos)
			{
				std::string name = line.substr(0, equalpos);
				while (!name.empty() && (name.back() == ' ' || name.back() == '\t'))
					name.pop_back();

				for (char& c : name)
					if (c >= 'A' && c <= 'Z')
						c += 'a' - 'A';

				std::string value;
				size_t valuepos = line.find_first_not_of(" \t", equalpos + 1);
				if (valuepos != std::string::npos)
					value = line.substr(valuepos);
				if (!name.empty())
					sections[sectionName][name].push_back(value);
			}
		}
	}
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

std::vector<NameString> IniFile::GetKeys(NameString sectionName) const
{
	std::vector<NameString> result;
	
	auto itSection = sections.find(sectionName);
	if (itSection == sections.end())
		return {};

	const auto& values = itSection->second;

	for (auto& key : values)
	{
		result.push_back(key.first);
	}

	return result;
}

std::string IniFile::GetValue(NameString sectionName, NameString keyName) const
{
	auto itSection = sections.find(sectionName);
	if (itSection == sections.end())
		return {};

	const auto& values = itSection->second;
	auto itValues = values.find(keyName);
	if (itValues == values.end())
		return {};

	if (itValues->second.empty())
		return {};

	return itValues->second.front();
}

std::vector<std::string> IniFile::GetValues(NameString sectionName, NameString keyName) const
{
	auto itSection = sections.find(sectionName);
	if (itSection == sections.end())
		return {};

	const auto& values = itSection->second;
	auto itValues = values.find(keyName);
	if (itValues == values.end())
		return {};

	return itValues->second;
}
