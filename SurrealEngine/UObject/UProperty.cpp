
#include "Precomp.h"
#include "UProperty.h"

void UProperty::Load(ObjectStream* stream)
{
	UField::Load(stream);

	ArrayDimension = stream->ReadUInt32();
	PropFlags = (PropertyFlags)stream->ReadUInt32();
	Category = stream->ReadName();
	if (AllFlags(PropFlags, PropertyFlags::Net))
		ReplicationOffset = stream->ReadUInt16();
	if (stream->GetVersion() <= 61)
		PropFlags = (PropertyFlags)((uint32_t)PropFlags & ~0x00080040);
}

void UProperty::LoadValue(void* data, ObjectStream* stream, const PropertyHeader& header)
{
	ThrowIfTypeMismatch(header, UPT_Invalid);
}

void UProperty::LoadStructMemberValue(void* data, ObjectStream* stream)
{
	throw std::runtime_error("Unsupported struct member type");
}

void UProperty::ThrowIfTypeMismatch(const PropertyHeader& header, UnrealPropertyType type)
{
	if (header.type != type)
		throw std::runtime_error("Property value does not match property type!");
}

/////////////////////////////////////////////////////////////////////////////

void UPointerProperty::Load(ObjectStream* stream)
{
	UProperty::Load(stream);
}

void UPointerProperty::LoadValue(void* data, ObjectStream* stream, const PropertyHeader& header)
{
	ThrowIfTypeMismatch(header, UPT_Int);
	*reinterpret_cast<int32_t*>(data) = stream->ReadInt32();
}

void UPointerProperty::LoadStructMemberValue(void* data, ObjectStream* stream)
{
	*reinterpret_cast<int32_t*>(data) = stream->ReadInt32();
}

/////////////////////////////////////////////////////////////////////////////

void UByteProperty::Load(ObjectStream* stream)
{
	UProperty::Load(stream);
	EnumType = stream->ReadObject<UEnum>();
}

void UByteProperty::LoadValue(void* data, ObjectStream* stream, const PropertyHeader& header)
{
	ThrowIfTypeMismatch(header, UPT_Byte);
	*reinterpret_cast<uint8_t*>(data) = stream->ReadInt8();
}

void UByteProperty::LoadStructMemberValue(void* data, ObjectStream* stream)
{
	*reinterpret_cast<uint8_t*>(data) = stream->ReadInt8();
}

/////////////////////////////////////////////////////////////////////////////

void UIntProperty::LoadValue(void* data, ObjectStream* stream, const PropertyHeader& header)
{
	ThrowIfTypeMismatch(header, UPT_Int);
	*reinterpret_cast<int32_t*>(data) = stream->ReadInt32();
}

void UIntProperty::LoadStructMemberValue(void* data, ObjectStream* stream)
{
	*reinterpret_cast<int32_t*>(data) = stream->ReadInt32();
}

/////////////////////////////////////////////////////////////////////////////

void UFloatProperty::LoadValue(void* data, ObjectStream* stream, const PropertyHeader& header)
{
	ThrowIfTypeMismatch(header, UPT_Float);
	*reinterpret_cast<float*>(data) = stream->ReadFloat();
}

void UFloatProperty::LoadStructMemberValue(void* data, ObjectStream* stream)
{
	*reinterpret_cast<float*>(data) = stream->ReadFloat();
}

/////////////////////////////////////////////////////////////////////////////

void UBoolProperty::LoadValue(void* data, ObjectStream* stream, const PropertyHeader& header)
{
	ThrowIfTypeMismatch(header, UPT_Bool);
	*reinterpret_cast<bool*>(data) = header.boolValue;
}

void UBoolProperty::LoadStructMemberValue(void* data, ObjectStream* stream)
{
	*reinterpret_cast<bool*>(data) = stream->ReadUInt8() == 1; // Is this always a byte? Is it aligned? Bitfield stuff?
}

/////////////////////////////////////////////////////////////////////////////

void UObjectProperty::Load(ObjectStream* stream)
{
	UProperty::Load(stream);
	ObjectClass = stream->ReadObject<UClass>();
}

void UObjectProperty::LoadValue(void* data, ObjectStream* stream, const PropertyHeader& header)
{
	ThrowIfTypeMismatch(header, UPT_Object);
	*reinterpret_cast<UObject**>(data) = stream->ReadObject<UObject>();
}

void UObjectProperty::LoadStructMemberValue(void* data, ObjectStream* stream)
{
	*reinterpret_cast<UObject**>(data) = stream->ReadObject<UObject>();
}

/////////////////////////////////////////////////////////////////////////////

void UNameProperty::LoadValue(void* data, ObjectStream* stream, const PropertyHeader& header)
{
	ThrowIfTypeMismatch(header, UPT_Name);
	*reinterpret_cast<NameString*>(data) = stream->ReadName();
}

void UNameProperty::LoadStructMemberValue(void* data, ObjectStream* stream)
{
	*reinterpret_cast<NameString*>(data) = stream->ReadName();
}

/////////////////////////////////////////////////////////////////////////////

void UStringProperty::LoadValue(void* data, ObjectStream* stream, const PropertyHeader& header)
{
	ThrowIfTypeMismatch(header, UPT_String);
	int len = header.size;
	std::vector<char> s;
	s.resize(len);
	stream->ReadBytes(s.data(), (int)s.size());
	s.push_back(0);
	*reinterpret_cast<std::string*>(data) = s.data();
}

/////////////////////////////////////////////////////////////////////////////

void UStrProperty::LoadValue(void* data, ObjectStream* stream, const PropertyHeader& header)
{
	if (header.type == UPT_Str)
	{
		*reinterpret_cast<std::string*>(data) = stream->ReadString();
	}
	else if (header.type == UPT_String)
	{
		int len = header.size;
		std::vector<char> s;
		s.resize(len);
		stream->ReadBytes(s.data(), (int)s.size());
		s.push_back(0);
		*reinterpret_cast<std::string*>(data) = s.data();
	}
	else
	{
		throw std::runtime_error("Property value does not match property type!");
	}
}

void UStrProperty::LoadStructMemberValue(void* data, ObjectStream* stream)
{
	*reinterpret_cast<std::string*>(data) = stream->ReadString();
}

/////////////////////////////////////////////////////////////////////////////

void UFixedArrayProperty::Load(ObjectStream* stream)
{
	UProperty::Load(stream);
	Inner = stream->ReadObject<UProperty>();
	Count = stream->ReadInt32();
}

void UFixedArrayProperty::LoadValue(void* data, ObjectStream* stream, const PropertyHeader& header)
{
	ThrowIfTypeMismatch(header, UPT_FixedArray);
	stream->Skip(header.size);
}

/////////////////////////////////////////////////////////////////////////////

void UArrayProperty::Load(ObjectStream* stream)
{
	UProperty::Load(stream);
	Inner = stream->ReadObject<UProperty>();
}

void UArrayProperty::LoadValue(void* data, ObjectStream* stream, const PropertyHeader& header)
{
	ThrowIfTypeMismatch(header, UPT_Array);
	throw std::runtime_error("Array properties not implemented");
}

/////////////////////////////////////////////////////////////////////////////

void UMapProperty::Load(ObjectStream* stream)
{
	UProperty::Load(stream);
	Key = stream->ReadObject<UProperty>();
	Value = stream->ReadObject<UProperty>();
}

void UMapProperty::LoadValue(void* data, ObjectStream* stream, const PropertyHeader& header)
{
	ThrowIfTypeMismatch(header, UPT_Map);
	stream->Skip(header.size);
}

/////////////////////////////////////////////////////////////////////////////

void UClassProperty::Load(ObjectStream* stream)
{
	UObjectProperty::Load(stream);
	MetaClass = stream->ReadObject<UClass>();
}

/////////////////////////////////////////////////////////////////////////////

void UStructProperty::Load(ObjectStream* stream)
{
	UProperty::Load(stream);
	Struct = stream->ReadObject<UStruct>();
	if (Struct)
		Struct->LoadNow();

	if (Struct->Name == "Vector")
		ValueType = ExpressionValueType::ValueVector;
	else if (Struct->Name == "Rotator")
		ValueType = ExpressionValueType::ValueRotator;
	else if (Struct->Name == "Color")
		ValueType = ExpressionValueType::ValueColor;
}

void UStructProperty::LoadValue(void* data, ObjectStream* stream, const PropertyHeader& header)
{
	ThrowIfTypeMismatch(header, UPT_Struct);

	if (Struct->Name != header.structName)
		throw std::runtime_error("Encountered struct '" + header.structName.ToString() + "' does not match expected struct property '" + Struct->Name.ToString() + "'");

	for (UField* field = Struct->Children; field != nullptr; field = field->Next)
	{
		UProperty* fieldprop = dynamic_cast<UProperty*>(field);
		if (fieldprop)
		{
			void* fielddata = (uint8_t*)data + fieldprop->DataOffset;
			fieldprop->LoadStructMemberValue(fielddata, stream);
		}
	}
}

void UStructProperty::LoadStructMemberValue(void* data, ObjectStream* stream)
{
	for (UField* field = Struct->Children; field != nullptr; field = field->Next)
	{
		UProperty* fieldprop = dynamic_cast<UProperty*>(field);
		if (fieldprop)
		{
			void* fielddata = (uint8_t*)data + fieldprop->DataOffset;
			fieldprop->LoadStructMemberValue(fielddata, stream);
		}
	}
}
