
#include "Precomp.h"
#include "UStringProperty.h"

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
