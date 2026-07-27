
#include "Precomp.h"
#include "UBoolProperty.h"

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
