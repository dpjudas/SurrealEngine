
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
		PropertyData.Init(Base);
		PropertyData.ReadProperties(stream);
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

std::string UObject::GetUClassName(UObject* obj)
{
	return obj->Base ? obj->Base->Name : std::string("null");
}

/////////////////////////////////////////////////////////////////////////////

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

void PropertyDataBlock::Init(UClass* cls)
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
}

void PropertyDataBlock::ReadProperties(ObjectStream* stream)
{
	while (true)
	{
		std::string name = stream->ReadName();
		if (name == "None")
			break;

		UProperty* prop = Class->Properties[name];
		if (!prop)
			throw std::runtime_error("Unknown property " + name);
		void* data = Ptr(prop);

		uint8_t info = stream->ReadInt8();
		bool infoBit = info & 0x80;

		PropertyHeader header;
		header.type = (UnrealPropertyType)(info & 0x0f);

		if (header.type == UPT_Struct)
			header.structName = stream->ReadName();

		switch ((info & 0x70) >> 4)
		{
		default:
		case 0: header.size = 1; break;
		case 1: header.size = 2; break;
		case 2: header.size = 4; break;
		case 3: header.size = 12; break;
		case 4: header.size = 16; break;
		case 5: header.size = stream->ReadInt8(); break;
		case 6: header.size = stream->ReadInt16(); break;
		case 7: header.size = stream->ReadInt32(); break;
		}

		header.arrayIndex = 0;
		if (infoBit && header.type != UPT_Bool)
		{
			int byte1 = stream->ReadUInt8();
			if ((byte1 & 0xc0) == 0xc0)
			{
				byte1 &= 0x3f;
				int byte2 = stream->ReadUInt8();
				int byte3 = stream->ReadUInt8();
				int byte4 = stream->ReadUInt8();
				header.arrayIndex = (byte1 << 24) | (byte2 << 16) | (byte3 << 8) | byte4;
			}
			else if (byte1 & 0x80)
			{
				byte1 &= 0x7f;
				int byte2 = stream->ReadUInt8();
				header.arrayIndex = (byte1 << 8) | byte2;
			}
			else
			{
				header.arrayIndex = byte1;
			}
		}
		else if (header.type == UPT_Bool)
		{
			header.boolValue = infoBit;
		}

		prop->LoadValue(data, stream, header);

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
