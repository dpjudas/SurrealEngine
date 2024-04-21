#include "UnrealURL.h"

#include <sstream>

UnrealURL::UnrealURL(const UnrealURL& base, const std::string& url)
{
	// To do: this also needs to be able to handle fully qualified URLs for network support

	*this = base;

	size_t pos = url.find('?');
	if (pos == std::string::npos)
	{
		Map = url;
	}
	else
	{
		Map = url.substr(0, pos);

		pos++;
		while (pos < url.size())
		{
			size_t endpos = url.find('?', pos);
			if (endpos == std::string::npos)
				endpos = url.size();
			AddOrReplaceOption(url.substr(pos, endpos - pos));
			pos = endpos + 1;
		}
	}

	// Unreal uses relative urls
	if (Map.size() > 8 && Map.substr(0, 8) == "..\\maps\\")
		Map = Map.substr(8);
}

UnrealURL::UnrealURL(const std::string& urlString)
{
	// Expected url format on a local game:
	// mapname[#teleporttag][?key1=value1[?key2=value2]...]

	std::string mapName = "";
	std::string teleportTag = "";
	std::string options = "";

	size_t teleportTagPos = urlString.find('#');

	if (teleportTagPos != std::string::npos)
	{
		mapName = urlString.substr(0, teleportTagPos);

		size_t optionsTagPos = urlString.find_first_of('?');

		if (optionsTagPos != std::string::npos)
		{
			teleportTag = urlString.substr(teleportTagPos + 1, optionsTagPos - teleportTagPos + 1);
			options = urlString.substr(optionsTagPos + 1);
		}
		else
			// No options mean that the string consists of only the map name and teleporter tag
			teleportTag = urlString.substr(teleportTagPos + 1);
	}
	else
	{
		// No teleporter tag
		size_t optionsTagPos = urlString.find_first_of('?');

		if (optionsTagPos != std::string::npos)
		{
			mapName = urlString.substr(0, optionsTagPos);
			options = urlString.substr(optionsTagPos + 1);
		}
		else
			// No options either, which means the url is just the name of the map
			mapName = urlString;
	}

	// Remove map extension from mapName if it is there
	size_t extPos = mapName.find(".");
	if (extPos != std::string::npos)
		mapName = mapName.substr(0, extPos);

	Map = mapName;
	Portal = teleportTag;

	// Parse the options
	if (!options.empty())
	{
		std::stringstream ss(options);
		std::string option;

		while (getline(ss, option, '?'))
			AddOrReplaceOption(option); // The key=value parsing is done within AddOrReplaceOption()
	}
}

void UnrealURL::AddOrReplaceOption(const std::string& newvalue)
{
	size_t pos = newvalue.find('=');
	if (pos != std::string::npos)
	{
		std::string name = newvalue.substr(0, pos);
		for (char& c : name) c = std::tolower(c);
		for (std::string& option : Options)
		{
			if (option.size() >= name.size() + 1 && option[name.size()] == '=')
			{
				std::string key = option.substr(0, name.size());
				for (char& c : key) c = std::tolower(c);
				if (key == name)
				{
					option = newvalue;
					return;
				}
			}
		}
		Options.push_back(newvalue);
	}
	else
	{
		std::string name = newvalue;
		for (char& c : name) c = std::tolower(c);
		for (std::string& option : Options)
		{
			if (option.size() == name.size())
			{
				std::string key = option;
				for (char& c : key) c = std::tolower(c);
				if (key == name)
				{
					option = newvalue;
					return;
				}
			}
		}
		Options.push_back(newvalue);
	}
}

bool UnrealURL::HasOption(const std::string& name) const
{
	for (const std::string& option : Options)
	{
		if ((option.size() >= name.size() + 1 && option[name.size()] == '=') || option.size() == name.size())
		{
			std::string key = option.substr(0, name.size());
			for (char& c : key) c = std::tolower(c);
			if (key == name)
				return true;
		}
	}
	return false;
}

std::string UnrealURL::GetOption(const std::string& name) const
{
	for (const std::string& option : Options)
	{
		if (option.size() >= name.size() + 1 && option[name.size()] == '=')
		{
			std::string key = option.substr(0, name.size());
			for (char& c : key) c = std::tolower(c);
			if (key == name)
				return option.substr(name.size() + 1);
		}
	}
	return {};
}

std::string UnrealURL::GetAddressURL() const
{
	return Host + ":" + std::to_string(Port);
}

std::string UnrealURL::GetOptions() const
{
	std::string result;
	for (const std::string& option : Options)
	{
		result += "?";
		result += option;
	}
	return result;
}

std::string UnrealURL::GetPortal() const
{
	if (!Portal.empty())
		return "#" + Portal;
	else
		return std::string();
}

std::string UnrealURL::ToString() const
{
	std::string result;

	if (Protocol != "unreal")
	{
		result += Protocol;
		result += ":";
		if (!Host.empty())
			result += "//";
	}

	if (!Host.empty() || Port != 7777)
	{
		result += Host;
		result += ":";
		result += std::to_string(Port);
		result += "/";
	}

	result += Map;
	result += GetOptions();
	result += GetPortal();

	return result;
}
