
#include "Precomp.h"
#include "UStructProperty.h"
#include "Packages/Core/UStruct.h"

UStructProperty::UStructProperty(NameString name, UClass* base, ObjectFlags flags) : UProperty(std::move(name), base, flags)
{
	ValueType = ExpressionValueType::ValueStruct;
}

void UStructProperty::Load(ObjectStream* stream)
{
	UProperty::Load(stream);
	Struct = stream->ReadObject<UStruct>();
	Struct->LoadNow();

	if (Struct->Name == "Vector")
		ValueType = ExpressionValueType::ValueVector;
	else if (Struct->Name == "Rotator")
		ValueType = ExpressionValueType::ValueRotator;
	else if (Struct->Name == "Color")
		ValueType = ExpressionValueType::ValueColor;
	else if (Struct->Name == "Coords")
		ValueType = ExpressionValueType::ValueCoords;
	else if (Struct->Name == "Quat")
		ValueType = ExpressionValueType::ValueQuat;
}

void UStructProperty::Save(PackageStreamWriter* stream)
{
	UProperty::Save(stream);
	stream->WriteObject(Struct);
}

void UStructProperty::LoadValue(void* data, ObjectStream* stream, const PropertyHeader& header)
{
	ThrowIfTypeMismatch(header, UPT_Struct);

	if (Struct->Name != header.structName)
		Exception::Throw("Encountered struct '" + header.structName.ToString() + "' does not match expected struct property '" + Struct->Name.ToString() + "'");

	LoadStructMemberValue(data, stream);
}

void UStructProperty::LoadStructMemberValue(void* data, ObjectStream* stream)
{
	if (Struct->Properties.empty())
		throw std::runtime_error("Struct has no properties");

	for (UProperty* fieldprop : Struct->Properties)
	{
		void* fielddata = (uint8_t*)data + fieldprop->DataOffset.DataOffset;
		fieldprop->LoadStructMemberValue(fielddata, stream);
	}
}

void UStructProperty::SaveHeader(void* data, PropertyHeader& header)
{
	header.type = UPT_Struct;
	header.structName = Struct->Name;
}

void UStructProperty::SaveValue(void* data, PackageStreamWriter* stream)
{
	for (UProperty* fieldprop : Struct->Properties)
	{
		void* fielddata = (uint8_t*)data + fieldprop->DataOffset.DataOffset;
		fieldprop->SaveValue(fielddata, stream);
	}
}

size_t UStructProperty::ElementAlignment()
{
	return Struct ? Struct->StructAlignment : 1;
}

size_t UStructProperty::ElementSize()
{
	return Struct ? Struct->StructSize : 0;
}

void UStructProperty::ConstructElement(void* data)
{
	if (Struct)
	{
		for (UProperty* prop : Struct->Properties)
		{
			prop->ConstructArray(static_cast<uint8_t*>(data) + prop->DataOffset.DataOffset);
		}
	}
}

void UStructProperty::CopyConstructElement(void* data, const void* src)
{
	if (Struct)
	{
		for (UProperty* prop : Struct->Properties)
		{
			prop->CopyConstructArray(
				static_cast<uint8_t*>(data) + prop->DataOffset.DataOffset,
				static_cast<const uint8_t*>(src) + prop->DataOffset.DataOffset);
		}
	}
}

void UStructProperty::CopyElement(void* data, const void* src)
{
	if (Struct)
	{
		for (UProperty* prop : Struct->Properties)
		{
			prop->CopyArray(
				static_cast<uint8_t*>(data) + prop->DataOffset.DataOffset,
				static_cast<const uint8_t*>(src) + prop->DataOffset.DataOffset);
		}
	}
}

void UStructProperty::DestructElement(void* data)
{
	if (Struct)
	{
		for (UProperty* prop : Struct->Properties)
		{
			prop->DestructArray(static_cast<uint8_t*>(data) + prop->DataOffset.DataOffset);
		}
	}
}

bool UStructProperty::CompareElement(const void* v1, const void* v2)
{
	return Struct ? Struct->IsEqual(v1, v2) : true;
}

bool UStructProperty::CompareLessElement(const void* v1, const void* v2)
{
	if (Struct)
	{
		for (UProperty* prop : Struct->Properties)
		{
			if (prop->CompareLessArray(
				static_cast<const uint8_t*>(v1) + prop->DataOffset.DataOffset,
				static_cast<const uint8_t*>(v2) + prop->DataOffset.DataOffset))
			{
				return true;
			}
		}
	}
	return false;
}

GCAllocation* UStructProperty::MarkPropertyElement(GCAllocation* marklist, void* data)
{
	if (Struct)
	{
		for (UProperty* prop : Struct->Properties)
		{
			marklist = prop->MarkPropertyElement(marklist, static_cast<uint8_t*>(data) + prop->DataOffset.DataOffset);
		}
	}
	return marklist;
}

void UStructProperty::GetExportText(std::string& buf, const std::string& whitespace, UObject* obj, UObject* defobj, int i)
{
	if (!Struct)
	{
		buf += whitespace + Name.ToString() + '=' + "null struct";
		return;
	}

	if (i >= ArrayDimension)
		Exception::Throw("UStructProperty::GetExportText index out of bounds");

	uint8_t* objval = static_cast<uint8_t*>(obj->PropertyData.Ptr(this)) + (i * ElementPitch());
	uint8_t* defval = nullptr;
	try
	{
		if (defobj)
			defval = static_cast<uint8_t*>(defobj->GetProperty(Name)) + (i * ElementPitch());
	}
	catch (...)
	{
	}

	std::string structbuf = "(";
	std::string innerbuf;

	for (UField* field = Struct->Children; field != nullptr; field = field->Next)
	{
		UProperty* fieldprop = UObject::TryCast<UProperty>(field);

		if (fieldprop == nullptr)
			continue;

		size_t offset = fieldprop->DataOffset.DataOffset;

		for (int k = 0; k < fieldprop->ArrayDimension; k++)
		{
			uint8_t* objsubval = objval + offset;
			uint8_t* defsubval = (defval) ? defval + offset : nullptr;
			fieldprop->GetExportText(innerbuf, "", objsubval, defsubval, k);

			if (innerbuf.length() > 0)
			{
				structbuf += innerbuf + ',';
				innerbuf.clear();
			}
		}
	}

	if (structbuf.length() > 1)
	{
		structbuf.pop_back();
		buf += whitespace + Name.ToString() + "=" + structbuf + ")\r\n";
	}
}

std::string UStructProperty::PrintValue(const void* data)
{
	if (Struct)
	{
		std::string print;
		uint8_t* d = (uint8_t*)data;
		for (UField* field = Struct->Children; field != nullptr; field = field->Next)
		{
			UProperty* fieldprop = UObject::TryCast<UProperty>(field);
			if (fieldprop)
			{
				if (!print.empty())
					print += ", ";
				print += fieldprop->Name.ToString();
				print += "=";
				print += fieldprop->PrintValue(d + fieldprop->DataOffset.DataOffset);
			}
		}
		return "(" + print + ")";
	}
	else
	{
		return "null struct";
	}
}

void UStructProperty::SetValueFromString(void* data, const std::string& valueString)
{
	if (valueString.empty())
		return;

	auto properties = ParsePropertiesFromString(valueString);

	if (Struct)
	{
		for (UField* field = Struct->Children; field != nullptr; field = field->Next)
		{
			UProperty* fieldprop = UObject::TryCast<UProperty>(field);
			if (fieldprop)
			{
				auto it = properties.find(fieldprop->Name);

				if (it != properties.end())
				{
					fieldprop->SetValueFromString(data, it->second);
				}
			}
		}
	}
}
