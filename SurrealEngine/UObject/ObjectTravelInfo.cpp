#include "ObjectTravelInfo.h"

#include "UProperty.h"
#include "UActor.h"

#include <string>
#include <sstream>
/*
ObjectTravelInfo::ObjectTravelInfo(UInventory* inventory)
{
	if (inventory)
	{
		// Full name, a.k.a. [PackageName].[ClassName]
		ClassName = UObject::GetUClassFullName(inventory).ToString();

		// Only include properties with PropertyFlags::Travel
		auto allProperties = inventory->GetAllTravelProperties();

		for (UProperty* property : allProperties)
			Properties[property->Name.ToString()] = inventory->GetPropertyAsString(property->Name);
	}
}
*/
ObjectTravelInfo::ObjectTravelInfo(UActor* travelActor)
{
	if (travelActor)
	{
		// Full name, a.k.a. [PackageName].[ClassName]
		ClassName = UObject::GetUClassFullName(travelActor).ToString();

		isPlayerPawn = UObject::TryCast<UPlayerPawn>(travelActor);

		// Only include properties with PropertyFlags::Travel
		auto allProperties = travelActor->GetAllTravelProperties();

		for (UProperty* property : allProperties)
			Properties[property->Name.ToString()] = travelActor->GetPropertyAsString(property->Name);
	}

	if (isPlayerPawn)
	{
		// Hacky workaround for selected Weapon and Inventory
		auto playerActor = UObject::Cast<UPlayerPawn>(travelActor);
		auto selectedInventory = UObject::Cast<UInventory>(playerActor->GetUObject("SelectedItem"));
		if (selectedInventory)
			Properties["SelectedItem"] = "{ name=None, class=" + UObject::GetUClassFullName(selectedInventory).ToString() + " }";

		auto selectedWeapon = UObject::Cast<UInventory>(playerActor->GetUObject("Weapon"));
		if (selectedWeapon)
			Properties["Weapon"] = "{ name=None, class=" + UObject::GetUClassFullName(selectedWeapon).ToString() + " }";
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

ObjectTravelInfo ParseSingleObject(const std::string& singleObjectText)
{
	auto colonPos = singleObjectText.find(':');

	if (colonPos == std::string::npos)
		throw std::runtime_error("No Class name found while parsing " + singleObjectText);

	std::string classNameAndType = singleObjectText.substr(0, colonPos);
	std::string properties = singleObjectText.substr(colonPos + 1);

	auto hashtagPos = classNameAndType.find('#');

	if (hashtagPos == std::string::npos)
		throw std::runtime_error("No # discriminator found while parsing " + singleObjectText);

	ObjectTravelInfo result;

	std::string className = classNameAndType.substr(0, hashtagPos);

	result.ClassName = className;

	result.isPlayerPawn = classNameAndType.substr(hashtagPos + 1) == "player";

	std::stringstream propStream(properties);

	std::string readProperty;

	while (getline(propStream, readProperty, ';'))
	{
		auto equalsPos = readProperty.find('=');

		if (equalsPos == std::string::npos)
			throw std::runtime_error("No = found while parsing property text: " + readProperty);

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
		result += object.ClassName + "#" + (object.isPlayerPawn ? "player" : "inv") + ":";

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
