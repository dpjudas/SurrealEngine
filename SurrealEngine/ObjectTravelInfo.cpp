
#include "Precomp.h"
#include "ObjectTravelInfo.h"
#include "Packages/Core/UClass.h"
#include "Packages/Core/Properties/UObjectProperty.h"
#include "Packages/Engine/Actors/UActor.h"
#include "Packages/Engine/Actors/Pawn/UPlayerPawn.h"
#include "Engine.h"
#include "Package/PackageManager.h"

#include <string>
#include <sstream>
#include <algorithm>

std::string ActorTravelInfo::Create(UPlayerPawn* pawn, bool transferItems)
{
	// Collect all actors we need to travel and give them a name each

	std::map<UObject*, std::string> travelObjectNames;
	Array<UObject*> processList;

	if (pawn)
	{
		travelObjectNames[pawn] = "player";
		processList.push_back(pawn);
	}

	if (transferItems)
	{
		for (size_t i = 0; i < processList.size(); i++)
		{
			UObject* cur = processList[i];
			for (UProperty* property : cur->GetAllTravelProperties())
			{
				if (auto objProperty = UObject::TryCast<UObjectProperty>(property))
				{
					UObject** values = static_cast<UObject**>(cur->GetProperty(objProperty));
					for (int j = 0; j < objProperty->ArrayDimension; j++)
					{
						UObject* value = values[j];
						if (value && travelObjectNames.find(value) == travelObjectNames.end())
						{
							std::string name = "item" + std::to_string(processList.size());
							travelObjectNames[value] = name;
							processList.push_back(value);
						}
					}
				}
			}
		}
	}

	// Save each travel object
	Array<TravelObject> objectTravelInfo;
	for (UObject* object : processList)
	{
		objectTravelInfo.emplace_back(CreateObject(object, travelObjectNames[object], travelObjectNames));
	}
	return ToString(objectTravelInfo);
}

std::string ActorTravelInfo::ToString(const Array<TravelObject>& travelActors)
{
	// Example:
	// ClassName#player:prop1=value1;prop2=value2;prop3=value3...?ClassName#item1:prop1=value1;prop2=value2...
	//LogMessage("Travel string begin:");
	std::string result;
	for (auto& object : travelActors)
	{
		result += object.ClassName + "#" + object.Name + ":";
		//LogMessage(object.ClassName + "#" + object.Name + ":");

		for (auto it = object.Properties.begin(); it != object.Properties.end(); it++)
		{
			result += it->first + "=" + Escape(it->second);
			//LogMessage("  " + it->first + " = " + Escape(it->second));

			if (it != --object.Properties.end())
				result += ";";
		}

		result += "?";
	}
	//LogMessage("Travel string end");
	return result;
}

ActorTravelInfo::TravelObject ActorTravelInfo::CreateObject(UObject* travelObject, const std::string& name, const std::map<UObject*, std::string>& travelObjects)
{
	TravelObject info;
	info.ClassName = UObject::GetUClassFullName(travelObject).ToString();
	info.Name = name;
	for (UProperty* property : travelObject->GetAllTravelProperties())
	{
		if (auto objProperty = UObject::TryCast<UObjectProperty>(property))
		{
			UObject** values = static_cast<UObject**>(travelObject->GetProperty(objProperty));
			for (int j = 0; j < objProperty->ArrayDimension; j++)
			{
				UObject* value = static_cast<UObject**>(travelObject->GetProperty(objProperty))[j];
				std::string name;
				if (value)
				{
					auto it = travelObjects.find(value);
					if (it != travelObjects.end())
						name = it->second;
				}
				else
				{
					// Seems we are not supposed to travel object properties set to None?
					// 
					// For Deus Ex, AugmentationSystem can be set to none and then is reinitialized in PostBeginPlay.
					// Travel happens after PostBeginPlay, so if we set it back to None Deus Ex breaks.
					// name = "None";
				}
				if (!name.empty())
				{
					if (objProperty->ArrayDimension > 1)
					{
						info.Properties[property->Name.ToString() + "[" + std::to_string(j) + "]"] = name;
					}
					else
					{
						info.Properties[property->Name.ToString()] = name;
					}
				}
			}
		}
		else
		{
			info.Properties[property->Name.ToString()] = travelObject->GetPropertyAsString(property->Name);
		}
	}
	return info;
}

Array<UObject*> ActorTravelInfo::Accept(UPlayerPawn* pawn, const std::string& travelInfo)
{
	Array<UObject*> acceptedObjects;
	Array<const TravelObject*> acceptedTravel;
	std::map<NameString, UObject*> nameToObject;

	Array<TravelObject> items = Parse(travelInfo);

	// Spawn the items
	for (const TravelObject& objInfo : items)
	{
		UObject* object = nullptr;
		if (objInfo.Name != "player")
		{
			UClass* cls = engine->packages->FindClass(objInfo.ClassName);

			UStruct* actorCls = cls;
			while (actorCls)
			{
				if (actorCls->Name == "Actor")
					break;
				actorCls = actorCls->BaseStruct;
			}

			if (actorCls)
			{
				object = pawn->Spawn(cls, pawn, NameString(), {}, {});
			}
			else if (cls)
			{
				object = engine->packages->GetTransientPackage()->NewObject({}, cls, ObjectFlags::Transient);
			}
			else
			{
				LogMessage("Warning: could not spawn travel object with class name: " + objInfo.ClassName);
			}
		}
		else
		{
			object = pawn;
		}

		if (object)
		{
			nameToObject[objInfo.Name] = object;
			acceptedObjects.push_back(object);
			acceptedTravel.push_back(&objInfo);
		}
	}

	// Set travel properties
	for (size_t i = 0, count = acceptedObjects.size(); i < count; i++)
	{
		const TravelObject& objInfo = *acceptedTravel[i];
		UObject* acceptedObject = acceptedObjects[i];

		for (UProperty* property : acceptedObject->GetAllTravelProperties())
		{
			// GetAllTravelProperties force-includes Inventory so Create() can walk the chain to
			// find what to carry. Outside Deus Ex it is not a real UE1 travel property (Actor.uc:
			// "var Inventory Inventory;", no travel keyword), and it must not be written back here:
			// UE1 rebuilds the chain in Inventory.TravelPreAccept -> GiveTo -> AddInventory, which
			// runs after this. Pre-linking it breaks scripts that ask whether the pawn already owns
			// an item - Translator.TravelPreAccept skips its Super call when
			// FindInventoryType(class) != None, and with the chain already wired it finds *itself*,
			// so it never gets BecomeItem()/GotoState('Idle2') and arrives as a visible world pickup
			// that no longer shows in the item list.
			if (property->Name == "Inventory" && !AllFlags(property->PropFlags, PropertyFlags::Travel))
				continue;

			for (size_t j = 0; j < property->ArrayDimension; j++)
			{
				std::string propertyNameStr;
				if (property->ArrayDimension == 1)
					propertyNameStr = property->Name.ToString();
				else
					propertyNameStr = property->Name.ToString() + "[" + std::to_string(j) + "]";

				auto it = objInfo.Properties.find(propertyNameStr);
				if (it == objInfo.Properties.end())
					continue;
				const std::string& value = it->second;

				if (auto objProperty = UObject::TryCast<UObjectProperty>(property))
				{
					UObject** objects = static_cast<UObject**>(acceptedObject->GetProperty(objProperty));
					if (value != "None")
					{
						auto it = nameToObject.find(value);
						if (it != nameToObject.end())
						{
							objects[j] = nameToObject[value];
						}
						else
						{
							LogMessage("Warning: could not find travel object: " + value);
						}
					}
					else
					{
						objects[j] = nullptr;
					}
				}
				else
				{
					if (j > 0)
					{
						LogMessage("ActorTravelInfo.Accept does not travel arrays correctly");
						break;
					}
					property->SetValueFromString(acceptedObject->GetProperty(property), value);
				}
			}
		}
	}

	// Important: we want the actor returned first, followed by inventory
	return acceptedObjects;
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
		std::string propValue = Unescape(readProperty.substr(equalsPos + 1));

		result.Properties[propName] = propValue;
	}

	return result;
}

std::string ActorTravelInfo::Escape(const std::string& value)
{
	std::string result;
	result.reserve(value.size());
	for (char c : value)
	{
		if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '.' || c == '-' || c == '_')
		{
			result.push_back(c);
		}
		else if (c == ' ')
		{
			result.push_back('+');
		}
		else
		{
			uint8_t v = c;
			uint8_t nibbles[2] = { (uint8_t)(v >> 4), (uint8_t)(v & 15) };
			result.push_back('%');
			for (int j = 0; j < 2; j++)
			{
				if (nibbles[j] < 10)
					result.push_back('0' + (char)nibbles[j]);
				else
					result.push_back('a' + (char)(nibbles[j] - 10));
			}
		}
	}
	return result;
}

std::string ActorTravelInfo::Unescape(const std::string& value)
{
	std::string result;
	result.reserve(value.size());
	for (size_t i = 0, count = value.size(); i < count; i++)
	{
		if (value[i] == '+')
		{
			result.push_back(' ');
		}
		else if (value[i] == '%' && i + 3 <= count)
		{
			uint8_t v = 0;
			for (int j = 0; j < 2; j++)
			{
				i++;
				v <<= 4;
				if (value[i] >= 'A' && value[i] <= 'Z')
					v += 10 + (value[i] - 'A');
				else if (value[i] >= 'a' && value[i] <= 'z')
					v += 10 + (value[i] - 'a');
				else if (value[i] >= '0' && value[i] <= '9')
					v += value[i] - '0';
			}
			result.push_back((char)v);
		}
		else
		{
			result.push_back(value[i]);
		}
	}
	return result;
}
