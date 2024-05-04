#pragma once

#include <map>

class UProperty;
class UInventory;

// Travel info parser for a single UInventory
struct ObjectTravelInfo
{
	ObjectTravelInfo(UInventory* inventory);
	static std::vector<ObjectTravelInfo> Parse(const std::string& text);
	static std::string ToString(const std::vector<ObjectTravelInfo>& objects);

	std::string ClassName;
	std::map<std::string, std::string> Properties;
};

static ObjectTravelInfo ParseSingleObject(const std::string& singleObjectText);
