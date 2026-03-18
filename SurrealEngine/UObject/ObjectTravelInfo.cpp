
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
	// Collect all actors we need to travel and give them a name each

	std::map<UActor*, std::string> travelActorNames;
	Array<UActor*> processList;

	if (pawn)
	{
		travelActorNames[pawn] = "player";
		processList.push_back(pawn);
	}

	if (transferItems)
	{
		for (size_t i = 0; i < processList.size(); i++)
		{
			UActor* cur = processList[i];
			for (UProperty* property : cur->GetAllTravelProperties())
			{
				if (auto objProperty = UObject::TryCast<UObjectProperty>(property))
				{
					UActor* value = UObject::TryCast<UActor>(*static_cast<UObject**>(cur->GetProperty(objProperty)));
					if (value && travelActorNames.find(value) == travelActorNames.end())
					{
						std::string name = "item" + std::to_string(processList.size());
						travelActorNames[value] = name;
						processList.push_back(value);
					}
				}
			}
		}
	}

	// Save each travel actor
	Array<TravelObject> actorTravelInfo;
	for (UActor* actor : processList)
	{
		actorTravelInfo.emplace_back(CreateObject(actor, travelActorNames[actor], travelActorNames));
	}
	return ToString(actorTravelInfo);
}

std::string ActorTravelInfo::ToString(const Array<TravelObject>& travelActors)
{
	// Example:
	// ClassName#player:prop1=value1;prop2=value2;prop3=value3...?ClassName#item1:prop1=value1;prop2=value2...

	std::string result;
	for (auto& object : travelActors)
	{
		result += object.ClassName + "#" + object.Name + ":";

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

ActorTravelInfo::TravelObject ActorTravelInfo::CreateObject(UActor* travelActor, const std::string& name, const std::map<UActor*, std::string>& travelActors)
{
	TravelObject info;
	info.ClassName = UObject::GetUClassFullName(travelActor).ToString();
	info.Name = name;
	for (UProperty* property : travelActor->GetAllTravelProperties())
	{
		if (auto objProperty = UObject::TryCast<UObjectProperty>(property))
		{
			UActor* value = UObject::TryCast<UActor>(*static_cast<UObject**>(travelActor->GetProperty(objProperty)));
			std::string name;
			if (value)
			{
				auto it = travelActors.find(value);
				if (it != travelActors.end())
					name = it->second;
			}
			else
			{
				name = "None";
			}
			if (!name.empty())
				info.Properties[property->Name.ToString()] = name;
		}
		else
		{
			info.Properties[property->Name.ToString()] = travelActor->GetPropertyAsString(property->Name);
		}
	}
	return info;
}

Array<UActor*> ActorTravelInfo::Accept(UPlayerPawn* pawn, const std::string& travelInfo)
{
	Array<UActor*> acceptedActors;
	Array<const TravelObject*> acceptedTravel;
	std::map<NameString, UObject*> nameToActor;

	Array<TravelObject> items = Parse(travelInfo);

	// Spawn the items
	for (const TravelObject& objInfo : items)
	{
		UActor* actor = nullptr;
		if (objInfo.Name != "player")
		{
			UClass* cls = engine->packages->FindClass(objInfo.ClassName);
			if (cls)
			{
				actor = pawn->Spawn(cls, nullptr, NameString(), nullptr, nullptr);
			}
			else
			{
				LogMessage("Warning: could not spawn travel actor with class name: " + objInfo.ClassName);
			}
		}
		else
		{
			actor = pawn;
		}

		if (actor)
		{
			nameToActor[objInfo.Name] = actor;
			acceptedActors.push_back(actor);
			acceptedTravel.push_back(&objInfo);
		}
	}

	// Set travel properties
	for (size_t i = 0, count = acceptedActors.size(); i < count; i++)
	{
		const TravelObject& objInfo = *acceptedTravel[i];
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
				if (value != "None")
				{
					auto it = nameToActor.find(value);
					if (it != nameToActor.end())
					{
						*obj = nameToActor[value];
					}
					else
					{
						LogMessage("Warning: could not find travel actor: " + value);
					}
				}
				else
				{
					*obj = nullptr;
				}
			}
			else
			{
				acceptedActor->SetPropertyFromString(property->Name, value);
			}
		}
	}

	// Important: we want the actor returned first, followed by inventory
	return acceptedActors;
}

Array<ActorTravelInfo::TravelObject> ActorTravelInfo::Parse(const std::string& text)
{
	Array<TravelObject> result;

	std::stringstream textStream(text);

	std::string propertyString;

	while (getline(textStream, propertyString, '?'))
		result.push_back(ParseSingleObject(propertyString));

	return result;
}

ActorTravelInfo::TravelObject ActorTravelInfo::ParseSingleObject(const std::string& singleObjectText)
{
	auto colonPos = singleObjectText.find(':');

	if (colonPos == std::string::npos)
		Exception::Throw("No Class name found while parsing " + singleObjectText);

	std::string classNameAndType = singleObjectText.substr(0, colonPos);
	std::string properties = singleObjectText.substr(colonPos + 1);

	auto hashtagPos = classNameAndType.find('#');

	if (hashtagPos == std::string::npos)
		Exception::Throw("No # discriminator found while parsing " + singleObjectText);

	TravelObject result;

	std::string className = classNameAndType.substr(0, hashtagPos);

	result.ClassName = className;

	result.Name = classNameAndType.substr(hashtagPos + 1);

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
