
#include "Precomp.h"
#include "UStrProperty.h"

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
