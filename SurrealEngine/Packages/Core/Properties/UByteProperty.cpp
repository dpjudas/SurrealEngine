
#include "Precomp.h"
#include "UByteProperty.h"
#include "Packages/Core/UEnum.h"
#include "Utils/Convert.h"

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
