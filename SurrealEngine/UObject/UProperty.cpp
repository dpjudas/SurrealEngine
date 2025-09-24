
#include "Precomp.h"
#include "UProperty.h"
#include "Package/PackageManager.h"

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
	*reinterpret_cast<int32_t*>(data) = stream->ReadInt32();
}

void UPointerProperty::LoadStructMemberValue(void* data, ObjectStream* stream)
{
	*reinterpret_cast<int32_t*>(data) = stream->ReadInt32();
}

void UPointerProperty::SaveHeader(void* data, PropertyHeader& header)
{
	header.type = UPT_Int;
}

void UPointerProperty::SaveValue(void* data, PackageStreamWriter* stream)
{
	stream->WriteInt32(*reinterpret_cast<int32_t*>(data));
}

/////////////////////////////////////////////////////////////////////////////

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
	*reinterpret_cast<uint8_t*>(data) = stream->ReadUInt8();
}

void UByteProperty::LoadStructMemberValue(void* data, ObjectStream* stream)
{
	*reinterpret_cast<uint8_t*>(data) = stream->ReadUInt8();
}

void UByteProperty::SaveHeader(void* data, PropertyHeader& header)
{
	header.type = UPT_Byte;
}

void UByteProperty::SaveValue(void* data, PackageStreamWriter* stream)
{
	stream->WriteUInt8(*reinterpret_cast<uint8_t*>(data));
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

void UIntProperty::SaveHeader(void* data, PropertyHeader& header)
{
	header.type = UPT_Int;
}

void UIntProperty::SaveValue(void* data, PackageStreamWriter* stream)
{
	stream->WriteInt32(*reinterpret_cast<int32_t*>(data));
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

void UFloatProperty::SaveHeader(void* data, PropertyHeader& header)
{
	header.type = UPT_Float;
}

void UFloatProperty::SaveValue(void* data, PackageStreamWriter* stream)
{
	stream->WriteFloat(*reinterpret_cast<float*>(data));
}

/////////////////////////////////////////////////////////////////////////////

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

/////////////////////////////////////////////////////////////////////////////

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
	*reinterpret_cast<UObject**>(data) = stream->ReadObject<UObject>();
}

void UObjectProperty::LoadStructMemberValue(void* data, ObjectStream* stream)
{
	*reinterpret_cast<UObject**>(data) = stream->ReadObject<UObject>();
}

void UObjectProperty::SaveHeader(void* data, PropertyHeader& header)
{
	header.type = UPT_Object;
}

void UObjectProperty::SaveValue(void* data, PackageStreamWriter* stream)
{
	stream->WriteObject(*reinterpret_cast<UObject**>(data));
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

void UNameProperty::SaveHeader(void* data, PropertyHeader& header)
{
	header.type = UPT_Name;
}

void UNameProperty::SaveValue(void* data, PackageStreamWriter* stream)
{
	stream->WriteName(*reinterpret_cast<NameString*>(data));
}

/////////////////////////////////////////////////////////////////////////////

void UStringProperty::LoadValue(void* data, ObjectStream* stream, const PropertyHeader& header)
{
	ThrowIfTypeMismatch(header, UPT_String);
	int len = header.size;
	Array<char> s;
	s.resize(len);
	stream->ReadBytes(s.data(), (int)s.size());
	s.push_back(0);
	*reinterpret_cast<std::string*>(data) = s.data();
}

void UStringProperty::SaveHeader(void* data, PropertyHeader& header)
{
	header.type = UPT_String;
	header.size = (int)reinterpret_cast<std::string*>(data)->size();
}

void UStringProperty::SaveValue(void* data, PackageStreamWriter* stream)
{
	const std::string& value = *reinterpret_cast<std::string*>(data);
	stream->WriteBytes(value.data(), (uint32_t)value.size());
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
		Array<char> s;
		s.resize(len);
		stream->ReadBytes(s.data(), (int)s.size());
		s.push_back(0);
		*reinterpret_cast<std::string*>(data) = s.data();
	}
	else
	{
		Exception::Throw("Property value does not match property type!");
	}
}

void UStrProperty::LoadStructMemberValue(void* data, ObjectStream* stream)
{
	*reinterpret_cast<std::string*>(data) = stream->ReadString();
}

void UStrProperty::SaveHeader(void* data, PropertyHeader& header)
{
	header.type = UPT_Str;
}

void UStrProperty::SaveValue(void* data, PackageStreamWriter* stream)
{
	stream->WriteString(*reinterpret_cast<std::string*>(data));
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
	header.size = (int)(Inner->Size() * Count);
}

void UFixedArrayProperty::SaveValue(void* data, PackageStreamWriter* stream)
{
	uint8_t* p = static_cast<uint8_t*>(data);
	for (int i = 0; i < Count; i++)
	{
		for (int arrayIndex = 0; arrayIndex < Inner->ArrayDimension; arrayIndex++)
		{
			Inner->SaveValue(p, stream);
			p += Inner->ElementSize();
		}
	}
}

/////////////////////////////////////////////////////////////////////////////

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

	size_t s = (Inner->Size() + 7) / 8;

	Array<void*>& vec = static_cast<Array<void*>*>(data)[header.arrayIndex];
	for (int i = 0; i < arraySize; i++)
	{
		int64_t* d = new int64_t[s];
		Inner->Construct(d);
		Inner->LoadStructMemberValue(d, stream);
		vec.push_back(d);
	}
}

void UArrayProperty::SaveHeader(void* data, PropertyHeader& header)
{
	header.type = UPT_Array;
	header.size = 5; // sizeof(Array) from UE1 
}

void UArrayProperty::SaveValue(void* data, PackageStreamWriter* stream)
{
#if 1
	Exception::Throw("UArrayProperty::SaveValue not implemented");
#else
	Array<void*>& vec = static_cast<Array<void*>*>(data)[arrayIndex];
	stream->WriteIndex((int)vec.size());
	for (void* item : vec)
	{
		Inner->SaveValue(item, stream);
	}
#endif
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

/////////////////////////////////////////////////////////////////////////////

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
}

void UStructProperty::Save(PackageStreamWriter* stream)
{
	UProperty::Save(stream);
	stream->WriteObject(Struct);
}

static void* LoadStruct(void* data, ObjectStream* stream, UStruct* Struct)
{
	if (auto base = UObject::TryCast<UStruct>(Struct->BaseField))
	{
		data = LoadStruct(data, stream, base);
	}

	for (UField* field = Struct->Children; field != nullptr; field = field->Next)
	{
		UProperty* fieldprop = UObject::TryCast<UProperty>(field);
		if (fieldprop)
		{
			void* fielddata = (uint8_t*)data + fieldprop->DataOffset.DataOffset;
			fieldprop->LoadStructMemberValue(fielddata, stream);
		}
	}

	return static_cast<uint8_t*>(data) + Struct->StructSize;
}

void UStructProperty::LoadValue(void* data, ObjectStream* stream, const PropertyHeader& header)
{
	ThrowIfTypeMismatch(header, UPT_Struct);

	if (Struct->Name != header.structName)
		Exception::Throw("Encountered struct '" + header.structName.ToString() + "' does not match expected struct property '" + Struct->Name.ToString() + "'");

	LoadStruct(data, stream, Struct);
}

void UStructProperty::LoadStructMemberValue(void* data, ObjectStream* stream)
{
	LoadStruct(data, stream, Struct);
}

void UStructProperty::SaveHeader(void* data, PropertyHeader& header)
{
	Exception::Throw("UStructProperty::SaveHeader not implemented");
}

void UStructProperty::SaveValue(void* data, PackageStreamWriter* stream)
{
	Exception::Throw("UStructProperty::SaveValue not implemented");
}
