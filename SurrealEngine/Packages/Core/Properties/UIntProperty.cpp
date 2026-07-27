
#include "Precomp.h"
#include "UIntProperty.h"
#include "Utils/Convert.h"

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
