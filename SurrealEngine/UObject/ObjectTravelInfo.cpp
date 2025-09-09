
#include "Precomp.h"
#include "ObjectTravelInfo.h"
#include "UProperty.h"
#include "UActor.h"
#include "Engine.h"
#include "Package/PackageManager.h"

#include <string>
#include <sstream>
#include <algorithm>

std::string ActorTravelInfo::Create(UPlayerPawn* pawn, bool transferItems)
{
	Array<ObjectTravelInfo> actorTravelInfo;

	// Add inventory items
	if (transferItems)
	{
		for (UInventory* item = pawn->Inventory(); item != nullptr; item = item->Inventory())
		{
			actorTravelInfo.emplace_back(item);
		}
	}

	// Add the pawn itself last
	actorTravelInfo.emplace_back(pawn);

	return ObjectTravelInfo::ToString(actorTravelInfo);
}

Array<UActor*> ActorTravelInfo::Accept(UPlayerPawn* pawn, const std::string& travelInfo)
{
	Array<UActor*> acceptedActors;
	Array<const ObjectTravelInfo*> acceptedTravel;
	std::map<NameString, UObject*> classToActor;

	auto items = ObjectTravelInfo::Parse(travelInfo);

	// Spawn the inventory items
	for (const ObjectTravelInfo& objInfo : items)
	{
		if (!objInfo.IsPlayerPawn)
		{
			UClass* cls = engine->packages->FindClass(objInfo.ClassName);
			if (cls)
			{
				UActor* actor = pawn->Spawn(cls, nullptr, NameString(), nullptr, nullptr);
				if (actor)
				{
					classToActor[UObject::GetUClassFullName(actor)] = actor;
					acceptedActors.push_back(actor);
					acceptedTravel.push_back(&objInfo);
				}
			}
		}
		else
		{
			classToActor[UObject::GetUClassFullName(pawn)] = pawn;
			acceptedActors.push_back(pawn);
			acceptedTravel.push_back(&objInfo);
		}
	}

	// Set travel properties
	for (size_t i = 0, count = acceptedActors.size(); i < count; i++)
	{
		const ObjectTravelInfo& objInfo = *acceptedTravel[i];
		UActor* acceptedActor = acceptedActors[i];

		for (UProperty* property : acceptedActor->GetAllTravelProperties())
		{
			auto it = objInfo.Properties.find(property->Name.ToString());
			if (it == objInfo.Properties.end())
				continue;
			const std::string& value = it->second;

			if (auto objProperty = UObject::TryCast<UObjectProperty>(property))
			{
				auto obj = static_cast<UObject**>(acceptedActor->GetProperty(objProperty));
				*obj = classToActor[value];
			}
			else
			{
				acceptedActor->SetPropertyFromString(property->Name, value);
			}
		}
	}

	// We want the actor returned first, followed by inventory
	std::reverse(acceptedActors.begin(), acceptedActors.end());

	return acceptedActors;
}

/////////////////////////////////////////////////////////////////////////////

ObjectTravelInfo::ObjectTravelInfo(UActor* travelActor)
{
	if (travelActor)
	{
		ClassName = UObject::GetUClassFullName(travelActor).ToString();
		IsPlayerPawn = UObject::TryCast<UPlayerPawn>(travelActor);

		for (UProperty* property : travelActor->GetAllTravelProperties())
		{
			if (auto objProperty = UObject::TryCast<UObjectProperty>(property))
			{
				auto obj = static_cast<UObject**>(travelActor->GetProperty(objProperty));
				Properties[property->Name.ToString()] = *obj ? UObject::GetUClassFullName(*obj).ToString() : "None";
			}
			else
			{
				Properties[property->Name.ToString()] = travelActor->GetPropertyAsString(property->Name);
			}
		}
	}
}

Array<ObjectTravelInfo> ObjectTravelInfo::Parse(const std::string& text)
{
	Array<ObjectTravelInfo> result;

	std::stringstream textStream(text);

	std::string propertyString;

	while (getline(textStream, propertyString, '?'))
		result.push_back(ParseSingleObject(propertyString));

	return result;
}

ObjectTravelInfo ObjectTravelInfo::ParseSingleObject(const std::string& singleObjectText)
{
	auto colonPos = singleObjectText.find(':');

	if (colonPos == std::string::npos)
		Exception::Throw("No Class name found while parsing " + singleObjectText);

	std::string classNameAndType = singleObjectText.substr(0, colonPos);
	std::string properties = singleObjectText.substr(colonPos + 1);

	auto hashtagPos = classNameAndType.find('#');

	if (hashtagPos == std::string::npos)
		Exception::Throw("No # discriminator found while parsing " + singleObjectText);

	ObjectTravelInfo result;

	std::string className = classNameAndType.substr(0, hashtagPos);

	result.ClassName = className;

	result.IsPlayerPawn = classNameAndType.substr(hashtagPos + 1) == "player";

	std::stringstream propStream(properties);

	std::string readProperty;

	while (getline(propStream, readProperty, ';'))
	{
		auto equalsPos = readProperty.find('=');

		if (equalsPos == std::string::npos)
			Exception::Throw("No = found while parsing property text: " + readProperty);

		std::string propName = readProperty.substr(0, equalsPos);
		std::string propValue = readProperty.substr(equalsPos + 1);

		result.Properties[propName] = propValue;
	}

	return result;
}

std::string ObjectTravelInfo::ToString(const Array<ObjectTravelInfo>& objects)
{
	// Example:
	// Object1#player:prop1=value1;prop2=value2;prop3=value3...?Object2#inv:prop1=value1;prop2=value2...
	std::string result = "";

	for (auto& object : objects)
	{
		result += object.ClassName + "#" + (object.IsPlayerPawn ? "player" : "inv") + ":";

		for (auto it = object.Properties.begin(); it != object.Properties.end(); it++)
		{
			result += it->first + "=" + it->second;

			if (it != --object.Properties.end())
				result += ";";
		}

		result += "?";
	}
	
	return result;
}
