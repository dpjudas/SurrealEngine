#include "UnrealURL.h"
#include "Engine.h"
#include "Package/PackageManager.h"

#include <sstream>

UnrealURL::UnrealURL(const UnrealURL& baseURL, const UnrealURL& nextURL)
{
	// To do: this also needs to be able to handle fully qualified URLs for network support

	*this = baseURL;

	// Pass options from the nextURL to the base one
	Map = nextURL.Map;
	Portal = nextURL.Portal;

	for (auto& option : nextURL.Options)
		AddOrReplaceOption(option);

	/*
	// Unreal uses relative urls
	if (Map.size() > 8 && Map.substr(0, 8) == "..\\maps\\")
		Map = Map.substr(8);
	*/
}

UnrealURL::UnrealURL(std::string urlString)
{
	// Expected url format on a local game:
	// mapname[#teleporttag][?key1=value1[?key2=value2]...]
	// Or in case of Klingons Honor Guard
	// mapname[/teleporttag][?key1=value1[?key2=value2]...]

	// Trim the url string of whitespaces
	// Fixes the crash during the transition from Temple of Vandora to The Trench in Unreal
	// Due to the exit teleporter pointing to " trench" (with the whitespace at the beginning)
	urlString.erase(urlString.find_last_not_of(' ') + 1);
	urlString.erase(0, urlString.find_first_not_of(' '));

	std::string mapName = "";
	std::string teleportTag = "";
	std::string options = "";

	size_t teleportTagPos;

	if (engine && engine->packages->IsKlingonHonorGuard())
		// Klingon Honor Guard uses / as the teleport tag delimiter for some reason
		teleportTagPos = urlString.find('/');
	else
		teleportTagPos = urlString.find('#');

	if (teleportTagPos != std::string::npos)
	{
		mapName = urlString.substr(0, teleportTagPos);
		std::string allOptsString = urlString.substr(teleportTagPos + 1);

		size_t optionsTagPos = allOptsString.find_first_of('?');

		if (optionsTagPos != std::string::npos)
		{
			teleportTag = allOptsString.substr(0, optionsTagPos);
			options = allOptsString.substr(optionsTagPos + 1);
		}
		else
			// No options mean that the string consists of only the map name and teleporter tag
			teleportTag = allOptsString;
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

	/*
	// Remove map extension from mapName if it is there
	size_t extPos = mapName.find_last_of(".");
	if (extPos != std::string::npos)
		mapName = mapName.substr(0, extPos);*/

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

std::string UnrealURL::GetOption(std::string name) const
{
	for (char& c : name) c = std::tolower(c);

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
	return Portal;
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

bool UnrealURL::Empty() const
{
	return Map.empty() && Portal.empty() && Options.empty();
}

void UnrealURL::Clear()
{
	Map.clear();
	Portal.clear();
	Options.clear();
}

