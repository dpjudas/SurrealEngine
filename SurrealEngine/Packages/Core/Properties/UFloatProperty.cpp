
#include "Precomp.h"
#include "UFloatProperty.h"
#include "Utils/Convert.h"

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
