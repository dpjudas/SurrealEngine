#pragma once

#include <map>

class UProperty;
class UInventory;
class UActor;

// Travel info parser for UInventory and UPlayerPawn-derived classes
struct ObjectTravelInfo
{
	//ObjectTravelInfo(UInventory* inventory);
	ObjectTravelInfo() {}
	ObjectTravelInfo(UActor* travelActor);
	static std::vector<ObjectTravelInfo> Parse(const std::string& text);
	static std::string ToString(const std::vector<ObjectTravelInfo>& objects);

	std::string ClassName; // Full class name of the traveling actor
	bool isPlayerPawn = false; // true = Actor is a PlayerPawn, false = Actor is an Inventory
	std::map<std::string, std::string> Properties; // Properties to travel
};

static ObjectTravelInfo ParseSingleObject(const std::string& singleObjectText);
