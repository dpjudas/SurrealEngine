
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

/////////////////////////////////////////////////////////////////////////////

void UIntProperty::LoadValue(void* data, ObjectStream* stream, const PropertyHeader& header)
{
	ThrowIfTypeMismatch(header, UPT_Int);
	*reinterpret_cast<int32_t*>(data) = stream->ReadInt32();
}

/////////////////////////////////////////////////////////////////////////////

void UFloatProperty::LoadValue(void* data, ObjectStream* stream, const PropertyHeader& header)
{
	ThrowIfTypeMismatch(header, UPT_Float);
	*reinterpret_cast<float*>(data) = stream->ReadFloat();
}

/////////////////////////////////////////////////////////////////////////////

void UBoolProperty::LoadValue(void* data, ObjectStream* stream, const PropertyHeader& header)
{
	ThrowIfTypeMismatch(header, UPT_Bool);
	*reinterpret_cast<bool*>(data) = header.boolValue;
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

/////////////////////////////////////////////////////////////////////////////

void UNameProperty::LoadValue(void* data, ObjectStream* stream, const PropertyHeader& header)
{
	ThrowIfTypeMismatch(header, UPT_Name);
	*reinterpret_cast<std::string*>(data) = stream->ReadName();
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
	ThrowIfTypeMismatch(header, UPT_Str);
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
}

void UStructProperty::LoadValue(void* data, ObjectStream* stream, const PropertyHeader& header)
{
	ThrowIfTypeMismatch(header, UPT_Struct);

	if (Struct->Name != header.structName)
		throw std::runtime_error("Encountered struct '" + header.structName + "' does not match expected struct property '" + Struct->Name + "'");

#if 0
	for (UField* field = Struct->Children; field != nullptr; field = field->Next)
	{
		UProperty* fieldprop = dynamic_cast<UProperty*>(field);
		if (fieldprop)
		{

		}
	}
#else
	if (header.structName == "Vector")
	{
		auto& prop = *reinterpret_cast<vec3*>(data);
		prop.x = stream->ReadFloat();
		prop.y = stream->ReadFloat();
		prop.z = stream->ReadFloat();
	}
	else if (header.structName == "Sphere")
	{
		float x = stream->ReadFloat();
		float y = stream->ReadFloat();
		float z = stream->ReadFloat();
	}
	else if (header.structName == "Coords")
	{
		float Originx = stream->ReadFloat();
		float Originy = stream->ReadFloat();
		float Originz = stream->ReadFloat();

		float XAxisx = stream->ReadFloat();
		float XAxisy = stream->ReadFloat();
		float XAxisz = stream->ReadFloat();

		float YAxisx = stream->ReadFloat();
		float YAxisy = stream->ReadFloat();
		float YAxisz = stream->ReadFloat();

		float ZAxisx = stream->ReadFloat();
		float ZAxisy = stream->ReadFloat();
		float ZAxisz = stream->ReadFloat();
	}
	else if (header.structName == "ModelCoords")
	{
		// PointXform, VectorXform
		for (int i = 0; i < 2; i++)
		{
			float Originx = stream->ReadFloat();
			float Originy = stream->ReadFloat();
			float Originz = stream->ReadFloat();

			float XAxisx = stream->ReadFloat();
			float XAxisy = stream->ReadFloat();
			float XAxisz = stream->ReadFloat();

			float YAxisx = stream->ReadFloat();
			float YAxisy = stream->ReadFloat();
			float YAxisz = stream->ReadFloat();

			float ZAxisx = stream->ReadFloat();
			float ZAxisy = stream->ReadFloat();
			float ZAxisz = stream->ReadFloat();
		}
	}
	else if (header.structName == "Rotator")
	{
		auto& prop = *reinterpret_cast<Rotator*>(data);
		prop.Pitch = stream->ReadInt32();
		prop.Yaw = stream->ReadInt32();
		prop.Roll = stream->ReadInt32();
	}
	else if (header.structName == "Box")
	{
		float minx = stream->ReadFloat();
		float miny = stream->ReadFloat();
		float minz = stream->ReadFloat();

		float maxx = stream->ReadFloat();
		float maxy = stream->ReadFloat();
		float maxz = stream->ReadFloat();

		uint8_t isvalid = stream->ReadInt8();
	}
	else if (header.structName == "Color")
	{
		int8_t red = stream->ReadInt8();
		int8_t green = stream->ReadInt8();
		int8_t blue = stream->ReadInt8();
		int8_t alpha = stream->ReadInt8();
	}
	else if (header.structName == "Matrix")
	{
		// XPlane >> YPlane >> ZPlane >> WPlane
		for (int i = 0; i < 4; i++)
		{
			float x = stream->ReadFloat();
			float y = stream->ReadFloat();
			float z = stream->ReadFloat();
			float w = stream->ReadFloat();
		}
	}
	else if (header.structName == "Plane")
	{
		float x = stream->ReadFloat();
		float y = stream->ReadFloat();
		float z = stream->ReadFloat();
		float w = stream->ReadFloat();
	}
	else if (header.structName == "Scale")
	{
		float x = stream->ReadFloat();
		float y = stream->ReadFloat();
		float z = stream->ReadFloat();
		float sheerRate = stream->ReadFloat();
		UnrealSheerAxis sheerAxis = (UnrealSheerAxis)stream->ReadInt8();
	}
	else if (header.structName == "PointRegion")
	{
		int32_t zoneIndex = stream->ReadIndex(); // AZoneInfo
		int32_t bspLeaf = stream->ReadInt32();
		uint8_t zoneNumber = stream->ReadInt8();
	}
	else if (header.structName == "ADrop" || header.structName == "ASpark")
	{
		uint16_t unknown1 = stream->ReadUInt16();
		uint8_t x = stream->ReadUInt8();
		uint8_t y = stream->ReadUInt8();
		uint16_t unknown2 = stream->ReadUInt32();
	}
	else if (header.structName == "Region")
	{
		struct Region { int X, Y, W, H; };
		Region reg;
		reg.X = stream->ReadInt32();
		reg.Y = stream->ReadInt32();
		reg.W = stream->ReadInt32();
		reg.H = stream->ReadInt32();
	}
	else
	{
		throw std::runtime_error("Unimplemented struct '" + header.structName + "' encountered");
	}
#endif
}
