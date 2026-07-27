
#include "Precomp.h"
#include "UObjectProperty.h"
#include "Package/PackageManager.h"
#include "Packages/Core/UClass.h"

UObjectProperty::UObjectProperty(NameString name, UClass* base, ObjectFlags flags) : UPropertyT(std::move(name), base, flags)
{
	ValueType = ExpressionValueType::ValueObject;
}

void UObjectProperty::Load(ObjectStream* stream)
{
	UProperty::Load(stream);
	ObjectClass = stream->ReadObject<UClass>();
}

void UObjectProperty::Save(PackageStreamWriter* stream)
{
	UProperty::Save(stream);
	stream->WriteObject(ObjectClass);
}

void UObjectProperty::LoadValue(void* data, ObjectStream* stream, const PropertyHeader& header)
{
	ThrowIfTypeMismatch(header, UPT_Object);
	*static_cast<UObject**>(data) = stream->ReadObject<UObject>();
}

void UObjectProperty::LoadStructMemberValue(void* data, ObjectStream* stream)
{
	*static_cast<UObject**>(data) = stream->ReadObject<UObject>();
}

void UObjectProperty::SaveHeader(void* data, PropertyHeader& header)
{
	header.type = UPT_Object;
}

void UObjectProperty::SaveValue(void* data, PackageStreamWriter* stream)
{
	stream->WriteObject(*static_cast<UObject**>(data));
}

std::string UObjectProperty::PrintValue(const void* data)
{
	UObject* obj = *(UObject**)data;
	if (obj)
		return obj->Class->Name.ToString() + '\'' + obj->Name.ToString()/*obj->package->GetExportName(obj->exportIndex)*/ + '\'';
	else
		return "None";
}

bool UObjectProperty::IsDefaultValue(void* val)
{
	return *(UObject**)val == nullptr;
}

void UObjectProperty::SetValueFromString(void* data, const std::string& valueString)
{
	if (valueString.empty())
		return;

	UObject** propertyValue = (UObject**)data;

	if (valueString.substr(0, 6) == "Class\'" || valueString.substr(0, 6) == "class\'")
	{
		*propertyValue = package->GetPackageManager()->FindClass(valueString.substr(6, valueString.length() - 7));
	}
	else
	{
		// This code is trying to set properties recursively into objects that already exists.
		// Is this something UE1 actually does?

		UObject* obj = *propertyValue;
		if (obj)
		{
			auto parsedProperties = ParsePropertiesFromString(valueString);

			for (auto& prop : parsedProperties)
				obj->SetPropertyFromString(prop.first, prop.second);
		}
	}
}

GCAllocation* UObjectProperty::MarkPropertyElement(GCAllocation* marklist, void* data)
{
	GC::MarkObject(marklist, static_cast<UObject*>(data));
	return marklist;
}
