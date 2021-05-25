
#include "Precomp.h"
#include "UObject.h"
#include "UClass.h"
#include "UProperty.h"
#include "Package/Package.h"

UObject::UObject(std::string name, UClass* base, ObjectFlags flags) : Name(name), Base(base), Flags(flags)
{
}

void UObject::LoadNow()
{
	if (DelayLoad)
	{
		auto info = std::move(DelayLoad);
		auto stream = info->package->OpenObjectStream(info->Index, info->ObjName, info->Class);
		if (!stream->IsEmptyStream())
			Load(stream.get());
	}
}

void UObject::Load(ObjectStream* stream)
{
	if (Base)
		Base->LoadNow();

	if (AllFlags(stream->GetFlags(), ObjectFlags::HasStack))
	{
		int32_t node = stream->ReadIndex();
		int32_t stateNode = stream->ReadIndex();
		int64_t probeMask = stream->ReadInt64();
		int32_t latentAction = stream->ReadInt32();
		if (node != 0)
		{
			int offset = stream->ReadIndex();
		}
	}

	if (!dynamic_cast<UClass*>(this))
	{
		PropertyData.ReadProperties(stream, Base);
	}
}

const void* UObject::GetProperty(const std::string& name) const
{
	auto it = PropertyData.Class->Properties.find(name);
	if (it != PropertyData.Class->Properties.end())
		return PropertyData.Ptr(it->second);
	else
		throw std::runtime_error("Property '" + name + "' not found");
}

void* UObject::GetProperty(const std::string& name)
{
	auto it = PropertyData.Class->Properties.find(name);
	if (it != PropertyData.Class->Properties.end())
		return PropertyData.Ptr(it->second);
	else
		throw std::runtime_error("Property '" + name + "' not found");
}

bool UObject::HasProperty(const std::string& name) const
{
	auto it = PropertyData.Class->Properties.find(name);
	return it != PropertyData.Class->Properties.end();
}

uint8_t UObject::GetByte(const std::string& name) const
{
	return *static_cast<const uint8_t*>(GetProperty(name));
}

uint32_t UObject::GetInt(const std::string& name) const
{
	return *static_cast<const uint32_t*>(GetProperty(name));
}

bool UObject::GetBool(const std::string& name) const
{
	return *static_cast<const bool*>(GetProperty(name));
}

float UObject::GetFloat(const std::string& name) const
{
	return *static_cast<const float*>(GetProperty(name));
}

vec3 UObject::GetVector(const std::string& name) const
{
	return *static_cast<const vec3*>(GetProperty(name));
}

Rotator UObject::GetRotator(const std::string& name) const
{
	return *static_cast<const Rotator*>(GetProperty(name));
}

const std::string& UObject::GetString(const std::string& name) const
{
	return *static_cast<const std::string*>(GetProperty(name));
}

UObject* UObject::GetUObject(const std::string& name)
{
	return *static_cast<UObject**>(GetProperty(name));
}

void* PropertyDataBlock::Ptr(const UProperty* prop)
{
	return static_cast<uint8_t*>(Data) + prop->DataOffset;
}

const void* PropertyDataBlock::Ptr(const UProperty* prop) const
{
	return static_cast<const uint8_t*>(Data) + prop->DataOffset;
}

void PropertyDataBlock::Reset()
{
	// To do: this crashes as the class might have been destroyed first
	/*if (Data && Class)
	{
		for (auto& it : Class->Properties)
		{
			UProperty* prop = it.second;
			prop->Destruct(Ptr(prop));
		}
	}*/
	delete[](int64_t*)Data;
	Data = nullptr;
	Class = nullptr;
}

void PropertyDataBlock::ReadProperties(ObjectStream* stream, UClass* cls)
{
	Reset();

	Class = cls;
	Data = new int64_t[(cls->StructSize + 7) / 8];
	for (auto& it : cls->Properties)
	{
		UProperty* prop = it.second;

#ifdef _DEBUG
		if (prop->DataOffset + prop->Size() > cls->StructSize)
			throw std::runtime_error("Memory corruption detected!");
		memset((uint8_t*)Ptr(prop) - 8, 0xab, 8);
		memset((uint8_t*)Ptr(prop) + prop->Size(), 0xba, 8);
#endif

		if (&cls->PropertyData != this)
			prop->CopyConstruct(Ptr(prop), cls->PropertyData.Ptr(prop));
		else
			prop->Construct(Ptr(prop));

#ifdef _DEBUG
		static uint8_t start[8] = { 0xab, 0xab, 0xab, 0xab, 0xab, 0xab, 0xab, 0xab };
		static uint8_t end[8] = { 0xba, 0xba, 0xba, 0xba, 0xba, 0xba, 0xba, 0xba };
		if (memcmp((uint8_t*)Ptr(prop) - 8, start, 8) != 0)
			throw std::runtime_error("Memory corruption detected!");
		if (memcmp((uint8_t*)Ptr(prop) + prop->Size(), end, 8) != 0)
			throw std::runtime_error("Memory corruption detected!");
#endif
	}

	while (true)
	{
		std::string name = stream->ReadName();
		if (name == "None")
			break;

		UProperty* prop = cls->Properties[name];
		if (!prop)
			throw std::runtime_error("Unknown property " + name);
		void* data = Ptr(prop);

		uint8_t info = stream->ReadInt8();
		bool isArray = info & 0x80;
		UnrealPropertyType type = (UnrealPropertyType)(info & 0x0f);

		std::string structName;
		if (type == UPT_Struct)
			structName = stream->ReadName();

		int size;
		switch ((info & 0x70) >> 4)
		{
		default:
		case 0: size = 1; break;
		case 1: size = 2; break;
		case 2: size = 4; break;
		case 3: size = 12; break;
		case 4: size = 16; break;
		case 5: size = stream->ReadInt8(); break;
		case 6: size = stream->ReadInt16(); break;
		case 7: size = stream->ReadInt32(); break;
		}

		int arrayIndex = 0;
		if (isArray && type != UPT_Bool)
		{
			int byte1 = stream->ReadUInt8();
			if ((byte1 & 0xc0) == 0xc0)
			{
				byte1 &= 0x3f;
				int byte2 = stream->ReadUInt8();
				int byte3 = stream->ReadUInt8();
				int byte4 = stream->ReadUInt8();
				arrayIndex = (byte1 << 24) | (byte2 << 16) | (byte3 << 8) | byte4;
			}
			else if (byte1 & 0x80)
			{
				byte1 &= 0x7f;
				int byte2 = stream->ReadUInt8();
				arrayIndex = (byte1 << 8) | byte2;
			}
			else
			{
				arrayIndex = byte1;
			}
		}

		switch (type)
		{
		default:
		case UPT_Invalid:
			break;
		case UPT_Byte:
			*reinterpret_cast<uint8_t*>(data) = stream->ReadInt8();
			break;
		case UPT_Int:
			*reinterpret_cast<int32_t*>(data) = stream->ReadInt32();
			break;
		case UPT_Bool:
			*reinterpret_cast<bool*>(data) = isArray;
			isArray = false;
			break;
		case UPT_Float:
			*reinterpret_cast<float*>(data) = stream->ReadFloat();
			break;
		case UPT_Object:
			*reinterpret_cast<UObject**>(data) = stream->ReadObject<UObject>();
			break;
		case UPT_Name:
			*reinterpret_cast<std::string*>(data) = stream->ReadName();
			break;
		case UPT_String:
			{
				int len = size;
				std::vector<char> s;
				s.resize(len);
				stream->ReadBytes(s.data(), (int)s.size());
				s.push_back(0);
				*reinterpret_cast<std::string*>(data) = s.data();
			}
			break;
		case UPT_Class:
			stream->Skip(size);
			break;
		case UPT_Array:
			throw std::runtime_error("Array properties not implemented");
			break;
		case UPT_Struct:
			//prop.Scalar.ValueString = structName;
			if (structName == "Vector")
			{
				auto& prop = *reinterpret_cast<vec3*>(data);
				prop.x = stream->ReadFloat();
				prop.y = stream->ReadFloat();
				prop.z = stream->ReadFloat();
			}
			else if (structName == "Sphere")
			{
				float x = stream->ReadFloat();
				float y = stream->ReadFloat();
				float z = stream->ReadFloat();
			}
			else if (structName == "Coords")
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
			else if (structName == "ModelCoords")
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
			else if (structName == "Rotator")
			{
				auto& prop = *reinterpret_cast<Rotator*>(data);
				prop.Pitch = stream->ReadInt32() / 65536.0f * 360.0f;
				prop.Yaw = stream->ReadInt32() / 65536.0f * 360.0f;
				prop.Roll = stream->ReadInt32() / 65536.0f * 360.0f;
			}
			else if (structName == "Box")
			{
				float minx = stream->ReadFloat();
				float miny = stream->ReadFloat();
				float minz = stream->ReadFloat();

				float maxx = stream->ReadFloat();
				float maxy = stream->ReadFloat();
				float maxz = stream->ReadFloat();

				uint8_t isvalid = stream->ReadInt8();
			}
			else if (structName == "Color")
			{
				int8_t red = stream->ReadInt8();
				int8_t green = stream->ReadInt8();
				int8_t blue = stream->ReadInt8();
				int8_t alpha = stream->ReadInt8();
			}
			else if (structName == "Matrix")
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
			else if (structName == "Plane")
			{
				float x = stream->ReadFloat();
				float y = stream->ReadFloat();
				float z = stream->ReadFloat();
				float w = stream->ReadFloat();
			}
			else if (structName == "Scale")
			{
				float x = stream->ReadFloat();
				float y = stream->ReadFloat();
				float z = stream->ReadFloat();
				float sheerRate = stream->ReadFloat();
				UnrealSheerAxis sheerAxis = (UnrealSheerAxis)stream->ReadInt8();
			}
			else if (structName == "PointRegion")
			{
				int32_t zoneIndex = stream->ReadIndex(); // AZoneInfo
				int32_t bspLeaf = stream->ReadInt32();
				uint8_t zoneNumber = stream->ReadInt8();
			}
			else if (structName == "ADrop" || structName == "ASpark")
			{
				uint16_t unknown1 = stream->ReadUInt16();
				uint8_t x = stream->ReadUInt8();
				uint8_t y = stream->ReadUInt8();
				uint16_t unknown2 = stream->ReadUInt32();
			}
			else
			{
				throw std::runtime_error("Unimplemented struct '" + structName + "' encountered");
			}
			break;
		case UPT_Vector:
			{
				auto& prop = *reinterpret_cast<vec3*>(data);
				prop.x = stream->ReadFloat();
				prop.y = stream->ReadFloat();
				prop.z = stream->ReadFloat();
			}
			break;
		case UPT_Rotator:
			{
				auto& prop = *reinterpret_cast<Rotator*>(data);
				prop.Pitch = stream->ReadInt32() / 65536.0f * 360.0f;
				prop.Yaw = stream->ReadInt32() / 65536.0f * 360.0f;
				prop.Roll = stream->ReadInt32() / 65536.0f * 360.0f;
			}
			break;
		case UPT_Str:
			*reinterpret_cast<std::string*>(data) = stream->ReadString();
			break;
		case UPT_Map:
			stream->Skip(size);
			break;
		case UPT_FixedArray:
			stream->Skip(size);
			break;
		}

#ifdef _DEBUG
		static uint8_t start[8] = { 0xab, 0xab, 0xab, 0xab, 0xab, 0xab, 0xab, 0xab };
		static uint8_t end[8] = { 0xba, 0xba, 0xba, 0xba, 0xba, 0xba, 0xba, 0xba };
		if (memcmp((uint8_t*)Ptr(prop) - 8, start, 8) != 0)
			throw std::runtime_error("Memory corruption detected!");
		if (memcmp((uint8_t*)Ptr(prop) + prop->Size(), end, 8) != 0)
			throw std::runtime_error("Memory corruption detected!");
#endif
	}
}

std::string UObject::GetUClassName(UObject* obj)
{
	return obj->Base ? obj->Base->Name : std::string("null");
}
