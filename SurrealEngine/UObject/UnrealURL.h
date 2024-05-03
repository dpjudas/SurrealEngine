#pragma once

#include <string>

class UnrealURL
{
public:
	UnrealURL() = default;
	// Constructs an UnrealURL by passing the options from nextURL to the baseURL
	UnrealURL(const UnrealURL& baseURL, const UnrealURL& nextURL);
	// Constructs an UnrealURL from a given parse-able string.
	UnrealURL(std::string urlString);

	void AddOrReplaceOption(const std::string& newvalue);
	bool HasOption(const std::string& name) const;
	std::string GetOption(const std::string& name) const;
	std::string GetAddressURL() const;
	std::string GetOptions() const;
	std::string GetPortal() const;
	std::string ToString() const;

	std::string Protocol = "unreal";
	std::string ProtocolDescription = "Unreal Protocol";
	std::string Name = "Player";
	std::string Map = "Index.unr";
	std::string Host;
	std::string Portal;
	std::string MapExt = "unr";
	std::string SaveExt = "usa";
	int Port = 7777;
	std::vector<std::string> Options;
};