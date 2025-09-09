#pragma once

#include <map>

class UProperty;
class UInventory;
class UActor;
class UPlayerPawn;

class ActorTravelInfo
{
public:
	static std::string Create(UPlayerPawn* pawn, bool transferItems);
	static Array<UActor*> Accept(UPlayerPawn* pawn, const std::string& travelInfo);
};

// Travel info parser for UInventory and UPlayerPawn-derived classes
class ObjectTravelInfo
{
public:
	ObjectTravelInfo() = default;
	ObjectTravelInfo(UActor* travelActor);

	static Array<ObjectTravelInfo> Parse(const std::string& text);
	static std::string ToString(const Array<ObjectTravelInfo>& objects);

	// Full class name of the traveling actor
	// [PackageName].[ClassName]
	std::string ClassName;

	// true = Actor is a PlayerPawn, false = Actor is an Inventory
	bool IsPlayerPawn = false;

	// Properties to travel
	std::map<std::string, std::string> Properties;

private:
	static ObjectTravelInfo ParseSingleObject(const std::string& singleObjectText);
};
