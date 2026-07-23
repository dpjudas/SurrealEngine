
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

/////////////////////////////////////////////////////////////////////////////

void UAnyProperty::Load(ObjectStream* stream)
{
	UProperty::Load(stream);
	LogUnimplemented("AnyProperty.Load");
}

void UAnyProperty::Save(PackageStreamWriter* stream)
{
	UProperty::Save(stream);
	LogUnimplemented("AnyProperty.Save");
}

/////////////////////////////////////////////////////////////////////////////

void UPointerProperty::Load(ObjectStream* stream)
{
	UProperty::Load(stream);
}

void UPointerProperty::Save(PackageStreamWriter* stream)
{
	UProperty::Save(stream);
}

void UPointerProperty::LoadValue(void* data, ObjectStream* stream, const PropertyHeader& header)
{
	ThrowIfTypeMismatch(header, UPT_Int);
	*static_cast<int32_t*>(data) = stream->ReadInt32();
}

void UPointerProperty::LoadStructMemberValue(void* data, ObjectStream* stream)
{
	*static_cast<int32_t*>(data) = stream->ReadInt32();
}

void UPointerProperty::SaveHeader(void* data, PropertyHeader& header)
{
	header.type = UPT_Int;
}

void UPointerProperty::SaveValue(void* data, PackageStreamWriter* stream)
{
	stream->WriteInt32(*static_cast<int32_t*>(data));
}

/////////////////////////////////////////////////////////////////////////////

UByteProperty::UByteProperty(NameString name, UClass* base, ObjectFlags flags) : UPropertyT(std::move(name), base, flags)
{
	ValueType = ExpressionValueType::ValueByte;
}

void UByteProperty::Load(ObjectStream* stream)
{
	UProperty::Load(stream);
	EnumType = stream->ReadObject<UEnum>();
}

void UByteProperty::Save(PackageStreamWriter* stream)
{
	UProperty::Save(stream);
	stream->WriteObject(EnumType);
}

void UByteProperty::LoadValue(void* data, ObjectStream* stream, const PropertyHeader& header)
{
	ThrowIfTypeMismatch(header, UPT_Byte);
	*static_cast<uint8_t*>(data) = stream->ReadUInt8();
}

void UByteProperty::LoadStructMemberValue(void* data, ObjectStream* stream)
{
	*static_cast<uint8_t*>(data) = stream->ReadUInt8();
}

void UByteProperty::SaveHeader(void* data, PropertyHeader& header)
{
	header.type = UPT_Byte;
}

void UByteProperty::SaveValue(void* data, PackageStreamWriter* stream)
{
	stream->WriteUInt8(*static_cast<uint8_t*>(data));
}

std::string UByteProperty::PrintValue(const void* data)
{
	uint8_t val = *(uint8_t*)data;
	if (EnumType && val < EnumType->ElementNames.size())
		return EnumType->ElementNames[val].ToString();

	return std::to_string(*(uint8_t*)data);
}

bool UByteProperty::IsDefaultValue(void* val)
{
	return *(uint8_t*)val == 0;
}

void UByteProperty::SetValueFromString(void* data, const std::string& valueString)
{
	if (!valueString.empty() && valueString.front() >= '0' && valueString.front() <= '9')
	{
		*(uint8_t*)data = Convert::to_uint8(valueString);
	}
	else if (EnumType)
	{
		int index = 0;
		for (const NameString& elementName : EnumType->ElementNames)
		{
			if (elementName == valueString)
			{
				*(uint8_t*)data = index;
				break;
			}
			index++;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////

UIntProperty::UIntProperty(NameString name, UClass* base, ObjectFlags flags) : UPropertyT(std::move(name), base, flags)
{
	ValueType = ExpressionValueType::ValueInt;
}

void UIntProperty::LoadValue(void* data, ObjectStream* stream, const PropertyHeader& header)
{
	ThrowIfTypeMismatch(header, UPT_Int);
	*static_cast<int32_t*>(data) = stream->ReadInt32();
}

void UIntProperty::LoadStructMemberValue(void* data, ObjectStream* stream)
{
	*static_cast<int32_t*>(data) = stream->ReadInt32();
}

void UIntProperty::SaveHeader(void* data, PropertyHeader& header)
{
	header.type = UPT_Int;
}

void UIntProperty::SaveValue(void* data, PackageStreamWriter* stream)
{
	stream->WriteInt32(*static_cast<int32_t*>(data));
}

std::string UIntProperty::PrintValue(const void* data)
{
	return std::to_string(*(int32_t*)data);
}

bool UIntProperty::IsDefaultValue(void* val)
{
	return *(int*)val == 0;
}

void UIntProperty::SetValueFromString(void* data, const std::string& valueString)
{
	*(int32_t*)data = Convert::to_int32(valueString);
}

/////////////////////////////////////////////////////////////////////////////

UFloatProperty::UFloatProperty(NameString name, UClass* base, ObjectFlags flags) : UPropertyT(std::move(name), base, flags)
{
	ValueType = ExpressionValueType::ValueFloat;
}

void UFloatProperty::LoadValue(void* data, ObjectStream* stream, const PropertyHeader& header)
{
	ThrowIfTypeMismatch(header, UPT_Float);
	*static_cast<float*>(data) = stream->ReadFloat();
}

void UFloatProperty::LoadStructMemberValue(void* data, ObjectStream* stream)
{
	*static_cast<float*>(data) = stream->ReadFloat();
}

void UFloatProperty::SaveHeader(void* data, PropertyHeader& header)
{
	header.type = UPT_Float;
}

void UFloatProperty::SaveValue(void* data, PackageStreamWriter* stream)
{
	stream->WriteFloat(*static_cast<float*>(data));
}

std::string UFloatProperty::PrintValue(const void* data) 
{
	return std::to_string(*(float*)data);
}

bool UFloatProperty::IsDefaultValue(void* val)
{
	return *(float*)val == 0.0f;
}

void UFloatProperty::SetValueFromString(void* data, const std::string& valueString)
{
	*(float*)data = Convert::to_float(valueString);
}

/////////////////////////////////////////////////////////////////////////////

UBoolProperty::UBoolProperty(NameString name, UClass* base, ObjectFlags flags) : UProperty(std::move(name), base, flags)
{
	ValueType = ExpressionValueType::ValueBool;
}

void UBoolProperty::LoadValue(void* data, ObjectStream* stream, const PropertyHeader& header)
{
	ThrowIfTypeMismatch(header, UPT_Bool);
	SetBool(data, header.boolValue);
}

void UBoolProperty::LoadStructMemberValue(void* data, ObjectStream* stream)
{
	SetBool(data, stream->ReadUInt8() == 1); // Is this always a byte? Is it aligned? Bitfield stuff?
}

void UBoolProperty::SaveHeader(void* data, PropertyHeader& header)
{
	header.type = UPT_Bool;
	header.boolValue = GetBool(data);
}

void UBoolProperty::SaveValue(void* data, PackageStreamWriter* stream)
{
}

size_t UBoolProperty::ElementAlignment()
{
	return alignof(uint32_t);
}

size_t UBoolProperty::ElementSize()
{
	return sizeof(uint32_t);
}

void UBoolProperty::ConstructElement(void* data)
{
	SetBool(data, false);
}

void UBoolProperty::CopyConstructElement(void* data, const void* src)
{
	SetBool(data, GetBool(src));
}

void UBoolProperty::DestructElement(void* /*data*/)
{
	// Nothing to do here
}

void UBoolProperty::CopyElement(void* data, const void* src)
{
	SetBool(data, GetBool(src));
}

bool UBoolProperty::CompareElement(const void* v1, const void* v2)
{
	bool b1 = GetBool(v1);
	bool b2 = GetBool(v2);
	return b1 == b2;
}

bool UBoolProperty::CompareLessElement(const void* v1, const void* v2)
{
	bool b1 = GetBool(v1);
	bool b2 = GetBool(v2);
	return b1 < b2;
}

bool UBoolProperty::GetBool(const void* data) const
{
	uint32_t v = *static_cast<const uint32_t*>(data);
	return (v & DataOffset.BitfieldMask) != 0;
}

void UBoolProperty::SetBool(void* data, bool value)
{
	uint32_t& v = *static_cast<uint32_t*>(data);
	if (value)
		v = v | DataOffset.BitfieldMask;
	else
		v = v & ~DataOffset.BitfieldMask;
}

void UBoolProperty::SetValueFromString(void* data, const std::string& valueString)
{
	NameString valueName(valueString);

	if (valueName != "true" && valueName != "false" && valueName != "0" && valueName != "1")
		Exception::Throw("Invalid bool value given to SetValueFromString(): " + valueString);

	bool value = (valueName == "true" || valueName == "1") ? true : false;

	SetBool(data, value);
}

bool UBoolProperty::IsDefaultValue(void* val)
{
	return GetBool(val) == false;
}

std::string UBoolProperty::PrintValue(const void* data)
{
	return GetBool(data) ? "True" : "False";
}

/////////////////////////////////////////////////////////////////////////////

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
		return obj->Class->Name.ToString() + '\'' + obj->Name.ToString()/*obj->package->GetExportName(obj->exportIndex)*/+ '\'';
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

/////////////////////////////////////////////////////////////////////////////

UNameProperty::UNameProperty(NameString name, UClass* base, ObjectFlags flags) : UPropertyT(std::move(name), base, flags)
{
	ValueType = ExpressionValueType::ValueName;
}

void UNameProperty::LoadValue(void* data, ObjectStream* stream, const PropertyHeader& header)
{
	ThrowIfTypeMismatch(header, UPT_Name);
	*static_cast<NameString*>(data) = stream->ReadName();
}

void UNameProperty::LoadStructMemberValue(void* data, ObjectStream* stream)
{
	*static_cast<NameString*>(data) = stream->ReadName();
}

void UNameProperty::SaveHeader(void* data, PropertyHeader& header)
{
	header.type = UPT_Name;
}

void UNameProperty::SaveValue(void* data, PackageStreamWriter* stream)
{
	stream->WriteName(*static_cast<NameString*>(data));
}

void UNameProperty::SetValueFromString(void* data, const std::string& valueString)
{
	*(NameString*)data = NameString(valueString);
}

std::string UNameProperty::PrintValue(const void* data)
{
	return ((NameString*)data)->ToString();
}

bool UNameProperty::IsDefaultValue(void* val)
{
	return static_cast<NameString*>(val)->IsNone();
}

/////////////////////////////////////////////////////////////////////////////

UStringProperty::UStringProperty(NameString name, UClass* base, ObjectFlags flags) : UPropertyT(std::move(name), base, flags)
{
	ValueType = ExpressionValueType::ValueString;
}

void UStringProperty::LoadValue(void* data, ObjectStream* stream, const PropertyHeader& header)
{
	ThrowIfTypeMismatch(header, UPT_String);
	int len = header.size;
	Array<char> s;
	s.resize(len);
	stream->ReadBytes(s.data(), (int)s.size());
	s.push_back(0);
	*static_cast<std::string*>(data) = s.data();
}

void UStringProperty::SaveHeader(void* data, PropertyHeader& header)
{
	header.type = UPT_String;
}

void UStringProperty::SaveValue(void* data, PackageStreamWriter* stream)
{
	const std::string& value = *static_cast<std::string*>(data);
	stream->WriteBytes(value.c_str(), (uint32_t)value.size() + 1);
}

void UStringProperty::SetValueFromString(void* data, const std::string& valueString)
{
	*(std::string*)data = valueString;
}

std::string UStringProperty::PrintValue(const void* data)
{
	return '"' + *(std::string*)data + '"';
}

bool UStringProperty::IsDefaultValue(void* val)
{
	return ((std::string*)val)->length() == 0;
}

/////////////////////////////////////////////////////////////////////////////

UStrProperty::UStrProperty(NameString name, UClass* base, ObjectFlags flags) : UPropertyT(std::move(name), base, flags)
{
	ValueType = ExpressionValueType::ValueString;
}

void UStrProperty::LoadValue(void* data, ObjectStream* stream, const PropertyHeader& header)
{
	if (header.type == UPT_Str)
	{
		*static_cast<std::string*>(data) = stream->ReadString();
	}
	else if (header.type == UPT_String)
	{
		int len = header.size;
		Array<char> s;
		s.resize(len);
		stream->ReadBytes(s.data(), (int)s.size());
		s.push_back(0);
		*static_cast<std::string*>(data) = s.data();
	}
	else
	{
		Exception::Throw("Property value does not match property type!");
	}
}

void UStrProperty::LoadStructMemberValue(void* data, ObjectStream* stream)
{
	*static_cast<std::string*>(data) = stream->ReadString();
}

void UStrProperty::SaveHeader(void* data, PropertyHeader& header)
{
	header.type = UPT_Str;
}

void UStrProperty::SaveValue(void* data, PackageStreamWriter* stream)
{
	stream->WriteString(*static_cast<std::string*>(data));
}

void UStrProperty::SetValueFromString(void* data, const std::string& valueString)
{
	*(std::string*)data = valueString;
}

std::string UStrProperty::PrintValue(const void* data)
{
	return '"' + *(std::string*)data + '"';
}

bool UStrProperty::IsDefaultValue(void* val)
{
	return ((std::string*)val)->length() == 0;
}

/////////////////////////////////////////////////////////////////////////////

void UFixedArrayProperty::Load(ObjectStream* stream)
{
	UProperty::Load(stream);
	Inner = stream->ReadObject<UProperty>();
	Count = stream->ReadInt32();
}

void UFixedArrayProperty::Save(PackageStreamWriter* stream)
{
	UProperty::Save(stream);
	stream->WriteObject(Inner);
	stream->WriteInt32(Count);
}

void UFixedArrayProperty::LoadValue(void* data, ObjectStream* stream, const PropertyHeader& header)
{
	ThrowIfTypeMismatch(header, UPT_FixedArray);
	stream->Skip(header.size);
}

void UFixedArrayProperty::SaveHeader(void* data, PropertyHeader& header)
{
	header.type = UPT_FixedArray;
}

void UFixedArrayProperty::SaveValue(void* data, PackageStreamWriter* stream)
{
	uint8_t* p = static_cast<uint8_t*>(data);
	for (int i = 0; i < Count; i++)
	{
		for (int arrayIndex = 0; arrayIndex < Inner->ArrayDimension; arrayIndex++)
		{
			Inner->SaveValue(p, stream);
			p += Inner->ElementPitch();
		}
	}
}

size_t UFixedArrayProperty::ElementAlignment()
{
	return Inner->ElementAlignment();
}

size_t UFixedArrayProperty::ElementSize()
{
	return Inner->ElementPitch() * Inner->ArrayDimension * Count;
}

void UFixedArrayProperty::ConstructElement(void* data)
{
	size_t pitch = Inner->ElementPitch();
	for (int i = 0; i < Count; i++)
	{
		Inner->ConstructElement(static_cast<uint8_t*>(data) + i * pitch);
	}
}

void UFixedArrayProperty::CopyConstructElement(void* data, const void* src)
{
	size_t pitch = Inner->ElementPitch();
	for (int i = 0; i < Count; i++)
	{
		Inner->CopyConstructElement(static_cast<uint8_t*>(data) + i * pitch, static_cast<const uint8_t*>(src) + i * pitch);
	}
}

void UFixedArrayProperty::DestructElement(void* data)
{
	size_t pitch = Inner->ElementPitch();
	for (int i = 0; i < Count; i++)
	{
		Inner->DestructElement(static_cast<uint8_t*>(data) + i * pitch);
	}
}

void UFixedArrayProperty::CopyElement(void* data, const void* src)
{
	size_t pitch = Inner->ElementPitch();
	for (int i = 0; i < Count; i++)
	{
		Inner->CopyElement(static_cast<uint8_t*>(data) + i * pitch, static_cast<const uint8_t*>(src) + i * pitch);
	}
}

bool UFixedArrayProperty::CompareElement(const void* v1, const void* v2)
{
	size_t pitch = Inner->ElementPitch();
	for (int i = 0; i < Count; i++)
	{
		if (!Inner->CompareElement(static_cast<const uint8_t*>(v1) + i * pitch, static_cast<const uint8_t*>(v2) + i * pitch))
			return false;
	}
	return true;
}

bool UFixedArrayProperty::CompareLessElement(const void* v1, const void* v2)
{
	size_t pitch = Inner->ElementPitch();
	for (int i = 0; i < Count; i++)
	{
		if (Inner->CompareLessElement(static_cast<const uint8_t*>(v1) + i * pitch, static_cast<const uint8_t*>(v2) + i * pitch))
			return true;
	}
	return false;
}

std::string UFixedArrayProperty::PrintValue(const void* data)
{
	return "fixed array";
}

GCAllocation* UFixedArrayProperty::MarkPropertyElement(GCAllocation* marklist, void* data)
{
	size_t pitch = Inner->ElementPitch();
	for (int i = 0; i < Count; i++)
	{
		marklist = Inner->MarkPropertyElement(marklist, static_cast<uint8_t*>(data) + i * pitch);
	}
	return marklist;
}

/////////////////////////////////////////////////////////////////////////////

UArrayProperty::UArrayProperty(NameString name, UClass* base, ObjectFlags flags) : UProperty(std::move(name), base, flags)
{
	ValueType = ExpressionValueType::ValueArray;
}

void UArrayProperty::Load(ObjectStream* stream)
{
	UProperty::Load(stream);
	Inner = stream->ReadObject<UProperty>();
}

void UArrayProperty::Save(PackageStreamWriter* stream)
{
	UProperty::Save(stream);
	stream->WriteObject(Inner);
}

void UArrayProperty::LoadValue(void* data, ObjectStream* stream, const PropertyHeader& header)
{
	ThrowIfTypeMismatch(header, UPT_Array);

	int arraySize = stream->ReadIndex();
	ScriptArray& vec = static_cast<ScriptArray*>(data)[header.arrayIndex];
	vec.Resize(arraySize);
	for (int i = 0; i < arraySize; i++)
	{
		Inner->LoadStructMemberValue(vec.GetItem(i), stream);
	}
}

void UArrayProperty::SaveHeader(void* data, PropertyHeader& header)
{
	header.type = UPT_Array;
}

void UArrayProperty::SaveValue(void* data, PackageStreamWriter* stream)
{
	ScriptArray& vec = *static_cast<ScriptArray*>(data);
	stream->WriteIndex((int)vec.GetSize());
	for (size_t i = 0, count = vec.GetSize(); i < count; i++)
	{
		Inner->SaveValue(vec.GetItem(i), stream);
	}
}

size_t UArrayProperty::ElementAlignment()
{
	return alignof(ScriptArray);
}

size_t UArrayProperty::ElementSize()
{
	return sizeof(ScriptArray);
}

void UArrayProperty::ConstructElement(void* data)
{
	new ((char*)data) ScriptArray(Inner);
}

void UArrayProperty::CopyConstructElement(void* data, const void* src)
{
	ConstructElement(data);
	CopyElement(data, src);
}

void UArrayProperty::DestructElement(void* data)
{
	auto vec = static_cast<ScriptArray*>(data);
	vec->~ScriptArray();
}

void UArrayProperty::CopyElement(void* data, const void* src)
{
	auto& vec = *static_cast<ScriptArray*>(data);
	auto& srcvec = *static_cast<const ScriptArray*>(src);
	vec = srcvec;
}

bool UArrayProperty::CompareElement(const void* a, const void* b)
{
	auto& avec = *static_cast<const ScriptArray*>(a);
	auto& bvec = *static_cast<const ScriptArray*>(b);
	return avec == bvec;
}

bool UArrayProperty::CompareLessElement(const void* a, const void* b)
{
	auto& avec = *static_cast<const ScriptArray*>(a);
	auto& bvec = *static_cast<const ScriptArray*>(b);
	return avec.GetSize() < bvec.GetSize();
}

void UArrayProperty::GetExportText(std::string& buf, const std::string& whitespace, UObject* obj, UObject* defobj, int i)
{
	if (i >= ArrayDimension)
		Exception::Throw("UArrayProperty::GetExportText index out of bounds");

	size_t elementPitch = ElementPitch();
	int offset = i * (int)elementPitch;

	ScriptArray* objarray = static_cast<ScriptArray*>(obj->GetProperty(Name)) + offset;
	ScriptArray* defarray = (defobj) ? static_cast<ScriptArray*>(defobj->GetProperty(Name)) + offset : nullptr;

	for (int k = 0; k < objarray->GetSize(); k++)
	{
		void* objval = objarray->GetItem(k);
		void* defval = (defarray && k < defarray->GetSize()) ? defarray->GetItem(k) : nullptr;

		Inner->GetExportText(buf, whitespace, objval, defval, i);
	}
}

std::string UArrayProperty::PrintValue(const void* data)
{
	return "array";
}

GCAllocation* UArrayProperty::MarkPropertyElement(GCAllocation* marklist, void* data)
{
	ScriptArray* array = static_cast<ScriptArray*>(data);
	size_t count = array->GetSize();
	for (size_t i = 0; i < count; i++)
	{
		marklist = Inner->MarkPropertyElement(marklist, array->GetItem(i));
	}
	return marklist;
}

/////////////////////////////////////////////////////////////////////////////

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

void UClassProperty::Load(ObjectStream* stream)
{
	UObjectProperty::Load(stream);
	MetaClass = stream->ReadObject<UClass>();
}

void UClassProperty::Save(PackageStreamWriter* stream)
{
	UObjectProperty::Save(stream);
	stream->WriteObject(MetaClass);
}

std::string UClassProperty::PrintValue(const void* data)
{
	UObject* obj = *(UObject**)data;
	if (obj)
		return "Class'" + obj->package->GetPackageName().ToString() + '.' + obj->Name.ToString() + '\'';
	else
		return "None";
}

/////////////////////////////////////////////////////////////////////////////

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

/////////////////////////////////////////////////////////////////////////////

ScriptArray::ScriptArray(UProperty* type) : Type(type)
{
}

ScriptArray::ScriptArray(const ScriptArray& other)
{
	Reserve(other.Size);
	try
	{
		auto pitch = Type->ElementPitch();
		for (size_t i = 0, count = other.Size; i < count; i++)
		{
			Type->CopyConstructElement(Data + Size * pitch, other.Data + i * pitch);
			Size++;
		}
	}
	catch (...)
	{
		Clear();
		AlignedFree(Data);
		throw;
	}
}

ScriptArray::ScriptArray(ScriptArray&& other)
{
	Data = other.Data;
	Size = other.Size;
	Capacity = other.Capacity;
	other.Data = nullptr;
	other.Size = 0;
	other.Capacity = 0;
}

ScriptArray::~ScriptArray()
{
	if (Type)
	{
		auto pitch = Type->ElementPitch();
		while (Size > 0)
		{
			Size--;
			Type->DestructElement(Data + pitch * Size);
		}
	}
	AlignedFree(Data);
	Data = nullptr;
}

ScriptArray& ScriptArray::operator=(ScriptArray&& other) noexcept
{
	if (&other != this)
	{
		Clear();
		ShrinkToFit();
		Data = other.Data;
		Size = other.Size;
		Capacity = other.Capacity;
		other.Data = nullptr;
		other.Size = 0;
		other.Capacity = 0;
	}
	return *this;
}

void ScriptArray::ShrinkToFit()
{
	if (Size != Capacity)
	{
		ScriptArray copy(Type);
		copy.Reserve(Size);

		auto pitch = Type->ElementPitch();
		for (size_t i = 0, count = Size; i < count; i++)
		{
			Type->CopyConstructElement(copy.Data + i * pitch, Data + i * pitch);
			copy.Size++;
		}

		Swap(copy);
	}
}

void ScriptArray::Clear()
{
	auto pitch = Type->ElementPitch();
	while (Size > 0)
	{
		Type->DestructElement(Data + pitch * (Size - 1));
		Size--;
	}
}

void ScriptArray::SetValue(size_t index, const void* src)
{
	auto pitch = Type->ElementPitch();
	Type->CopyElement(Data + index * pitch, src);
}

void ScriptArray::Insert(size_t insertPos, size_t insertCount)
{
	if (insertPos == Size)
	{
		Resize(Size + insertCount);
	}
	else
	{
		size_t new_cap = Size + insertCount;
		auto pitch = Type->ElementPitch();
		ScriptArray copy(Type);
		copy.Data = (uint8_t*)AlignedAlloc(Type->ElementAlignment(), pitch * new_cap);
		if (!copy.Data) throw std::bad_alloc();
		copy.Capacity = new_cap;

		// Copy before insertion block
		for (size_t i = 0, count = insertPos; i < count; i++)
		{
			Type->CopyConstructElement(copy.Data + copy.Size * pitch, Data + i * pitch);
			copy.Size++;
		}

		// Default initialize insertion block
		for (size_t i = 0, count = insertCount; i < count; i++)
		{
			Type->ConstructElement(copy.Data + copy.Size * pitch);
			copy.Size++;
		}

		// Copy after insertion block
		for (size_t i = insertPos, count = Size; i < count; i++)
		{
			Type->CopyConstructElement(copy.Data + copy.Size * pitch, Data + i * pitch);
			copy.Size++;
		}

		Swap(copy);

	}
}

void ScriptArray::Remove(size_t removePos, size_t removeCount)
{
	auto pitch = Type->ElementPitch();
	for (size_t i = removePos, end = Size - removeCount; i < end; i++)
	{
		Type->CopyElement(Data + i * pitch, Data + (i + removeCount) * pitch);
	}
	Resize(GetSize() - removeCount);
}

void* ScriptArray::GetItem(size_t index)
{
	return Data + Type->ElementPitch() * index;
}

void ScriptArray::Reserve(size_t new_cap)
{
	if (new_cap > Capacity)
	{
		auto pitch = Type->ElementPitch();
		ScriptArray copy(Type);
		copy.Data = (uint8_t*)AlignedAlloc(Type->ElementAlignment(), pitch * new_cap);
		if (!copy.Data) throw std::bad_alloc();
		copy.Capacity = new_cap;

		for (size_t i = 0, count = Size; i < count; i++)
		{
			Type->CopyConstructElement(copy.Data + i * pitch, Data + i * pitch);
			copy.Size++;
		}

		Swap(copy);
	}
}

void ScriptArray::Swap(ScriptArray& other) noexcept
{
	std::swap(Data, other.Data);
	std::swap(Size, other.Size);
	std::swap(Capacity, other.Capacity);
}

void ScriptArray::Resize(size_t count)
{
	if (count > Size)
	{
		Reserve(count);
		auto pitch = Type->ElementPitch();
		while (Size < count)
		{
			Type->ConstructElement(Data + Size * pitch);
			Size++;
		}
	}
	else if (count < Size)
	{
		auto pitch = Type->ElementPitch();
		while (Size > count)
		{
			Type->DestructElement(Data + (Size - 1) * pitch);
			Size--;
		}
	}
}

ScriptArray& ScriptArray::operator=(const ScriptArray& other)
{
	if (&other != this)
	{
		Clear();
		Reserve(other.GetSize());
		auto pitch = Type->ElementPitch();
		for (size_t i = 0, count = other.Size; i < count; i++)
		{
			Type->CopyConstructElement(Data + Size * pitch, other.Data + i * pitch);
			Size++;
		}
	}
	return *this;
}

bool ScriptArray::operator==(const ScriptArray& rhs) const
{
	if (GetSize() != rhs.GetSize())
		return false;

	auto count = GetSize();
	auto pitch = Type->ElementPitch();
	for (size_t i = 0; i < count; i++)
	{
		uint8_t* lhsitem = Data + pitch * i;
		uint8_t* rhsitem = rhs.Data + pitch * i;
		if (!Type->CompareElement(lhsitem, rhsitem))
			return false;
	}
	return true;
}

bool ScriptArray::operator!=(const ScriptArray& rhs) const
{
	if (GetSize() != rhs.GetSize())
		return true;
	size_t count = GetSize();
	auto pitch = Type->ElementPitch();
	for (size_t i = 0; i < count; i++)
	{
		uint8_t* lhsitem = Data + pitch * i;
		uint8_t* rhsitem = rhs.Data + pitch * i;
		if (!Type->CompareElement(lhsitem, rhsitem))
			return true;
	}
	return false;
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
