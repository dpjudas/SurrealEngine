#include "ObjectTravelInfo.h"

#include "UProperty.h"
#include "UActor.h"

#include <string>
#include <sstream>

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

std::vector<ObjectTravelInfo> ObjectTravelInfo::Parse(const std::string& text)
{
	std::vector<ObjectTravelInfo> result;

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

	std::string className = singleObjectText.substr(0, colonPos);
	std::string properties = singleObjectText.substr(colonPos + 1);

	ObjectTravelInfo result(nullptr);

	result.ClassName = className;

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

std::string ObjectTravelInfo::ToString(const std::vector<ObjectTravelInfo>& objects)
{
	// Example:
	// Object1:prop1=value1;prop2=value2;prop3=value3...?Object2:prop1=value1;prop2=value2...
	std::string result = "";

	for (auto& object : objects)
	{
		result += object.ClassName + ":";

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
