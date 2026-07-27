
#include "Precomp.h"
#include "UProperty.h"
#include "Package/PackageManager.h"
#include "Utils/AlignedAlloc.h"
#include "Utils/Logger.h"
#include <sstream>

void UProperty::Load(ObjectStream* stream)
{
	UField::Load(stream);

	ArrayDimension = stream->ReadInt32();
	PropFlags = (PropertyFlags)stream->ReadUInt32();
	Category = stream->ReadName();
	if (AllFlags(PropFlags, PropertyFlags::Net))
		ReplicationOffset = stream->ReadUInt16();
	if (stream->GetVersion() <= 61)
		PropFlags = (PropertyFlags)((uint32_t)PropFlags & ~0x00080040);
}

void UProperty::Save(PackageStreamWriter* stream)
{
	UField::Save(stream);

	// To do: we clear some old PropFlags during load.
	// Maybe store a copy of the original PropFlags in Load so we can save them here?

	stream->WriteInt32(ArrayDimension);
	stream->WriteUInt32((uint32_t)PropFlags);
	stream->WriteName(Category);
	if (AllFlags(PropFlags, PropertyFlags::Net))
		stream->WriteUInt16(ReplicationOffset);
}

void UProperty::LoadValue(void* data, ObjectStream* stream, const PropertyHeader& header)
{
	ThrowIfTypeMismatch(header, UPT_Invalid);
}

void UProperty::LoadStructMemberValue(void* data, ObjectStream* stream)
{
	Exception::Throw("Unsupported struct member type");
}

void UProperty::SaveHeader(void* data, PropertyHeader& header)
{
	header.type = UPT_Invalid;
}

void UProperty::SaveValue(void* data, PackageStreamWriter* stream)
{
}

void UProperty::ThrowIfTypeMismatch(const PropertyHeader& header, UnrealPropertyType type)
{
	if (header.type != type)
		Exception::Throw("Property value does not match property type!");
}

GCAllocation* UProperty::MarkProperty(GCAllocation* marklist, void* data)
{
	if (ValueType == ExpressionValueType::ValueObject ||
		ValueType == ExpressionValueType::ValueStruct ||
		ValueType == ExpressionValueType::ValueArray)
	{
		for (int i = 0; i < ArrayDimension; i++)
		{
			marklist = MarkPropertyElement(marklist, GetElement(data, i));
		}
	}
	return marklist;
}

void UProperty::ConstructArray(void* data)
{
	for (int i = 0; i < ArrayDimension; i++)
	{
		ConstructElement(GetElement(data, i));
	}
}

void UProperty::CopyConstructArray(void* data, const void* src)
{
	for (int i = 0; i < ArrayDimension; i++)
	{
		CopyConstructElement(GetElement(data, i), GetElement(src, i));
	}
}

void UProperty::DestructArray(void* data)
{
	for (int i = 0; i < ArrayDimension; i++)
	{
		DestructElement(GetElement(data, i));
	}
}

void UProperty::CopyArray(void* data, const void* src)
{
	for (int i = 0; i < ArrayDimension; i++)
	{
		CopyElement(GetElement(data, i), GetElement(src, i));
	}
}

bool UProperty::CompareArray(const void* v1, const void* v2)
{
	for (int i = 0; i < ArrayDimension; i++)
	{
		if (!CompareElement(GetElement(v1, i), GetElement(v2, i)))
			return false;
	}
	return true;
}

bool UProperty::CompareLessArray(const void* v1, const void* v2)
{
	for (int i = 0; i < ArrayDimension; i++)
	{
		if (CompareLessElement(GetElement(v1, i), GetElement(v2, i)))
			return true;
	}
	return false;
}

void UProperty::GetExportText(std::string& buf, const std::string& whitespace, UObject* obj, UObject* defobj, int i)
{
	if (i >= ArrayDimension)
		Exception::Throw("UProperty::GetExportText index out of bounds");

	size_t elementSize = ElementSize();
	int offset = i * (int)elementSize;

	void* objval = static_cast<uint8_t*>(obj->PropertyData.Ptr(this)) + offset;
	void* defval = nullptr;
	try
	{
		defval = (defobj) ? static_cast<uint8_t*>(defobj->GetProperty(Name)) + offset : nullptr;
	}
	catch (...)
	{
		// ignore exceptions, its ok if GetProperty fails here
	}

	std::string innerbuf;
	GetExportText(innerbuf, whitespace, objval, defval, i);
	if (innerbuf.size() > 0)
		innerbuf += "\r\n";

	buf += innerbuf;
}

void UProperty::GetExportText(std::string& buf, const std::string& whitespace, void* objval, void* defval, int i)
{
	if (!defval && IsDefaultValue(objval))
		return;
	else if (defval && CompareElement(objval, defval))
		return;

	buf += whitespace + Name.ToString();
	if (ArrayDimension > 1)
		buf += '(' + std::to_string(i) + ')';
	buf += '=' + PrintValue(objval);
}

void UProperty::SetValueFromString(void* data, const std::string& valueString)
{
	//Exception::Throw("SetValueFromString() is unimplemented on this Property type!");
}

// Parses a single property
std::pair<NameString, std::string> UProperty::ParseSingleProperty(std::string& propString)
{
	propString.erase(propString.find_last_not_of(' ') + 1);
	propString.erase(0, propString.find_first_not_of(' '));

	auto equalsPos = propString.find('=');

	if (equalsPos == std::string::npos)
		Exception::Throw("No = found in the property string: " + propString);

	std::string name = propString.substr(0, equalsPos);
	std::string value = propString.substr(equalsPos + 1);

	return std::make_pair(name, value);
}

// Parses all properties given in the string
std::map<NameString, std::string> UProperty::ParsePropertiesFromString(std::string propertiesString)
{
	std::map<NameString, std::string> properties;

	if (propertiesString.empty())
		return {};

	// Also check for the string being "null", "null struct" or "None"
	if (propertiesString == "null" || propertiesString == "null struct" || propertiesString == "None")
		return {};

	if (propertiesString[0] != '(')
		Exception::Throw("( not found in the property string: " + propertiesString);

	if (propertiesString[propertiesString.size() - 1] != ')')
		Exception::Throw(") not found in the property string: " + propertiesString);

	std::string propsString = propertiesString.substr(1, propertiesString.find(')') - 1);

	std::stringstream propsStream(propsString);
	std::string prop;

	while (getline(propsStream, prop, ','))
	{
		auto currProp = ParseSingleProperty(prop);
		properties[currProp.first] = currProp.second;
	}

	return properties;
}
