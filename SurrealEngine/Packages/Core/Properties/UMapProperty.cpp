
#include "Precomp.h"
#include "UMapProperty.h"
#include "Utils/AlignedAlloc.h"

void UMapProperty::Load(ObjectStream* stream)
{
	UProperty::Load(stream);
	Key = stream->ReadObject<UProperty>();
	Value = stream->ReadObject<UProperty>();
}

void UMapProperty::Save(PackageStreamWriter* stream)
{
	UProperty::Save(stream);
	stream->WriteObject(Key);
	stream->WriteObject(Value);
}

void UMapProperty::LoadValue(void* data, ObjectStream* stream, const PropertyHeader& header)
{
	ThrowIfTypeMismatch(header, UPT_Map);
	stream->Skip(header.size);
}

void UMapProperty::SaveHeader(void* data, PropertyHeader& header)
{
	Exception::Throw("UMapProperty::SaveHeader not implemented");
}

void UMapProperty::SaveValue(void* data, PackageStreamWriter* stream)
{
	Exception::Throw("UMapProperty::SaveValue not implemented");
}

size_t UMapProperty::ElementAlignment()
{
	return alignof(Map);
}

size_t UMapProperty::ElementSize()
{
	return sizeof(Map);
}

void UMapProperty::ConstructElement(void* data)
{
	new ((char*)data) Map();
}

void UMapProperty::CopyConstructElement(void* data, const void* src)
{
	ConstructElement(data);
	CopyElement(data, src);
}

void UMapProperty::DestructElement(void* data)
{
	auto map = static_cast<Map*>(data);
	map->~map();
}

void UMapProperty::CopyElement(void* data, const void* src)
{
	auto& map = *static_cast<Map*>(data);
	auto& srcmap = *static_cast<const Map*>(src);
	map = srcmap;
}

bool UMapProperty::CompareElement(const void* a, const void* b)
{
	auto& map = *static_cast<const Map*>(a);
	auto& srcmap = *static_cast<const Map*>(b);
	return map == srcmap;
}

bool UMapProperty::CompareLessElement(const void* a, const void* b)
{
	auto& map = *static_cast<const Map*>(a);
	auto& srcmap = *static_cast<const Map*>(b);
	return map.size() < srcmap.size();
}

std::string UMapProperty::PrintValue(const void* data)
{
	return "map";
}

GCAllocation* UMapProperty::MarkPropertyElement(GCAllocation* marklist, void* data)
{
	Map* map = static_cast<Map*>(data);
	for (auto& it : *map)
	{
		marklist = it.second.Property->MarkPropertyElement(marklist, it.second.Data);
	}
	return marklist;
}

/////////////////////////////////////////////////////////////////////////////

MapPropertyValue::MapPropertyValue(const MapPropertyValue& other)
{
	Create(other);
}

MapPropertyValue::MapPropertyValue(UProperty* prop, const void* data)
{
	Create(prop, data);
}

MapPropertyValue::~MapPropertyValue()
{
	Destroy();
}

MapPropertyValue& MapPropertyValue::operator=(const MapPropertyValue& other)
{
	if (&other != this)
	{
		Destroy();
		Create(other);
	}
	return *this;
}

bool MapPropertyValue::operator==(const MapPropertyValue& other) const
{
	if (Data && other.Data)
	{
		return Property->CompareElement(Data, other.Data);
	}
	else
	{
		return Data == other.Data;
	}
}

bool MapPropertyValue::operator<(const MapPropertyValue& other) const
{
	if (Data && other.Data)
	{
		return Property->CompareLessElement(Data, other.Data);
	}
	else
	{
		return Data < other.Data;
	}
}

void MapPropertyValue::Create(const MapPropertyValue& other)
{
	Create(other.Property, other.Data);
}

void MapPropertyValue::Create(UProperty* prop, const void* data)
{
	Property = prop;
	if (prop)
	{
		Data = AlignedAlloc(prop->ElementAlignment(), prop->ElementSize());
		if (data)
			prop->CopyConstructElement(Data, data);
		else
			prop->ConstructElement(Data);
	}
}

void MapPropertyValue::Destroy()
{
	if (Data)
	{
		Property->DestructElement(Data);
		AlignedFree(Data);
		Property = nullptr;
		Data = nullptr;
	}
}
