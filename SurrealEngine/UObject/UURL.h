#pragma once

#include <string>

class UnrealURL
{
public:
	UnrealURL() = default;
	UnrealURL(const UnrealURL& base, const std::string& url);
	UnrealURL(const std::string& urlString);

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