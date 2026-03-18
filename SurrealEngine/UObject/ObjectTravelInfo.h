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

private:
	// Each object in the travel info string
	struct TravelObject
	{
		// Full class name of the traveling actor
		// [PackageName].[ClassName]
		std::string ClassName;

		// "player" if it is the PlayerPawn, otherwise it needs to be spawned
		std::string Name;

		// Properties to travel
		std::map<std::string, std::string> Properties;
	};

	static TravelObject CreateObject(UActor* travelActor, const std::string& name, const std::map<UActor*, std::string>& travelActors);
	static std::string ToString(const Array<TravelObject>& travelActors);
	static Array<TravelObject> Parse(const std::string& text);
	static TravelObject ParseSingleObject(const std::string& singleObjectText);
};
