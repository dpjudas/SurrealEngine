
#include "Precomp.h"
#include "UObject.h"
#include "UClass.h"
#include "UProperty.h"
#include "Package/Package.h"
#include "Package/PackageManager.h"
#include "VM/ScriptCall.h"
#include "VM/Frame.h"
#include "VM/Bytecode.h"
#include "VM/NativeFunc.h"
#include "Engine.h"
#include "Utils/Exception.h"

UObject::UObject(NameString name, UClass* cls, ObjectFlags flags) : Name(name), Class(cls), Flags(flags)
{
}

UObject::~UObject()
{
}

void UObject::LoadNow()
{
	if (DelayLoad)
	{
		auto info = std::move(DelayLoad);
		auto stream = info->package->OpenObjectStream(info->Index, info->ObjName, info->Class);
		if (!stream->IsEmptyStream())
		{
			Load(stream.get());
		}
		else if (auto s = UObject::TryCast<UStruct>(this))
		{
			if (s->BaseStruct)
			{
				if (s->BaseStruct->Properties.empty())
					s->BaseStruct->LoadNow();
				s->Properties = s->BaseStruct->Properties;
				s->StructSize = s->BaseStruct->StructSize;
			}

			if (auto c = UObject::TryCast<UClass>(this))
			{
				PropertyData.Init(c);
				SetObject("Class", Class);
				SetName("Name", Name);
				SetInt("ObjectFlags", (int)Flags);
			}
		}
	}
}

void UObject::Load(ObjectStream* stream)
{
	if (Class)
		Class->LoadNow();

	if (AllFlags(Flags, ObjectFlags::HasStack))
	{
		UStruct* func = stream->ReadObject<UStruct>();
		UState* state = stream->ReadObject<UState>();
		int64_t probeMask = stream->ReadInt64();
		int32_t latentAction = stream->ReadInt32();

		for (int i = 0; i < 64; i++)
		{
			if (((1ULL >> (uint64_t)i) & 1) == 1)
			{
				DisableEvent(ToNameString((EventName)i));
			}
		}

		int offset = -1;
		if (func)
			offset = stream->ReadIndex();

		if (func && state)
		{
			if (offset != -1)
			{
				func->LoadNow();
				int index = func->Code->FindStatementIndex(offset);
				if (index != -1)
				{
					StateFrame = std::make_shared<Frame>(this, nullptr);
					StateFrame->SetState(func);
					StateFrame->StatementIndex = index;
					StateFrame->LatentState = NativeFunctions::LatentActionByIndex[latentAction];
				}
				else
				{
					LogMessage("UObject::Load could not find statement index");
				}
			}
			else
			{
				state->LoadNow();
				StateFrame = std::make_shared<Frame>(this, nullptr);
				StateFrame->SetState(state);
				StateFrame->LatentState = LatentRunState::Stop;
			}
		}
		else if (state)
		{
			LogMessage("UObject::Load encountered object with a state object but no function");
		}
	}

	if (!UObject::TryCast<UClass>(this))
	{
		PropertyData.Init(Class);
		PropertyData.Load(stream);
		if (Class && !Class->Properties.empty())
		{
			SetObject("Class", Class);
			SetName("Name", Name);
			SetInt("ObjectFlags", (int)Flags);
		}
	}
}

void UObject::Save(PackageStreamWriter* stream)
{
	if (AllFlags(Flags, ObjectFlags::HasStack))
	{
#if 1
		stream->WriteIndex(0);
		stream->WriteIndex(0);
		stream->WriteInt64(0);
		stream->WriteInt32(0);
#else
		// To do: state and func may not always be the same
		UStruct* func = StateFrame ? StateFrame->Func : nullptr;
		UState* state = StateFrame ? UObject::TryCast<UState>(StateFrame->Func) : nullptr;
		int32_t latentAction = 0;
		int offset = -1;

		int64_t probeMask = 0;
		for (int i = 0; i < 64; i++)
		{
			if (IsEventDisabled(ToNameString((EventName)i)))
			{
				probeMask |= 1ULL << (uint64_t)i;
			}
		}

		if (StateFrame && StateFrame->LatentState != LatentRunState::Stop)
		{
			latentAction = NativeFunctions::IndexForLatentAction[StateFrame->LatentState];
			offset = StateFrame->Func->Code->FindOffset(StateFrame->StatementIndex);
		}

		stream->WriteObject(func);
		stream->WriteObject(state);
		stream->WriteInt64(probeMask);
		stream->WriteInt32(latentAction);
		if (func)
			stream->WriteIndex(offset);
#endif
	}

	if (!UObject::TryCast<UClass>(this))
	{
		PropertyData.Save(stream, Class ? &Class->PropertyData : nullptr);
	}
}

PropertyDataOffset UObject::GetPropertyDataOffset(const NameString& name) const
{
	for (UProperty* prop : PropertyData.Class->Properties)
	{
		if (prop->Name == name)
			return prop->DataOffset;
	}
	return PropertyDataOffset();
}

UProperty* UObject::GetMemberProperty(const NameString& propName) const
{
	for (UProperty* prop : PropertyData.Class->Properties)
	{
		if (prop->Name == propName)
			return prop;
	}
	Exception::Throw("Object Property '" + Name.ToString() + "." + propName.ToString() + "' not found");
}

void* UObject::GetProperty(UProperty* prop)
{
	return PropertyData.Ptr(prop);
}

const void* UObject::GetProperty(UProperty* prop) const
{
	return PropertyData.Ptr(prop);
}

const void* UObject::GetProperty(const NameString& propName) const
{
	return GetProperty(GetMemberProperty(propName));
}

void* UObject::GetProperty(const NameString& propName)
{
	return GetProperty(GetMemberProperty(propName));
}

bool UObject::HasProperty(const NameString& name) const
{
	for (UProperty* prop : PropertyData.Class->Properties)
	{
		if (prop->Name == name)
			return true;
	}
	return false;
}

std::string UObject::GetPropertyAsString(const NameString& propName) const
{
	UProperty* prop = GetMemberProperty(propName);
	return prop->PrintValue(PropertyData.Ptr(prop));
}

void UObject::SetPropertyFromString(const NameString& name, const std::string& value)
{
	UProperty* prop = GetMemberProperty(name);
	prop->SetValueFromString(PropertyData.Ptr(prop), value);
}

void UObject::SaveConfig()
{
	// Saves the instance properties the ini file
	Class->SaveProperties(&PropertyData);
}

uint8_t UObject::GetByte(const NameString& name) const
{
	return *static_cast<const uint8_t*>(GetProperty(name));
}

uint32_t UObject::GetInt(const NameString& name) const
{
	return *static_cast<const uint32_t*>(GetProperty(name));
}

bool UObject::GetBool(const NameString& propName) const
{
	UProperty* prop = GetMemberProperty(propName);
	return static_cast<UBoolProperty*>(prop)->GetBool(PropertyData.Ptr(prop));
}

float UObject::GetFloat(const NameString& name) const
{
	return *static_cast<const float*>(GetProperty(name));
}

vec3 UObject::GetVector(const NameString& name) const
{
	return *static_cast<const vec3*>(GetProperty(name));
}

Rotator UObject::GetRotator(const NameString& name) const
{
	return *static_cast<const Rotator*>(GetProperty(name));
}

const std::string& UObject::GetString(const NameString& name) const
{
	return *static_cast<const std::string*>(GetProperty(name));
}

const NameString& UObject::GetName(const NameString& name) const
{
	return *static_cast<const NameString*>(GetProperty(name));
}

UObject* UObject::GetUObject(const NameString& name)
{
	return *static_cast<UObject**>(GetProperty(name));
}

Color UObject::GetColor(const NameString& name)
{
	return *static_cast<Color*>(GetProperty(name));
}

NameString UObject::GetUClassName(UObject* obj)
{
	return obj->Class ? obj->Class->Name : NameString();
}

NameString UObject::GetUClassFullName(UObject* obj)
{
	return obj->Class ? NameString(obj->Class->package->GetPackageName().ToString() + "." + obj->Class->Name.ToString()) : NameString();
}

void UObject::SetByte(const NameString& name, uint8_t value)
{
	*static_cast<uint8_t*>(GetProperty(name)) = value;
}

void UObject::SetInt(const NameString& name, uint32_t value)
{
	*static_cast<int32_t*>(GetProperty(name)) = value;
}

void UObject::SetBool(const NameString& name, bool value)
{
	UProperty* prop = GetMemberProperty(name);
	static_cast<UBoolProperty*>(prop)->SetBool(PropertyData.Ptr(prop), value);
}

void UObject::SetFloat(const NameString& name, float value)
{
	*static_cast<float*>(GetProperty(name)) = value;
}

void UObject::SetVector(const NameString& name, const vec3& value)
{
	*static_cast<vec3*>(GetProperty(name)) = value;
}

void UObject::SetRotator(const NameString& name, const Rotator& value)
{
	*static_cast<Rotator*>(GetProperty(name)) = value;
}

void UObject::SetString(const NameString& name, const std::string& value)
{
	*static_cast<std::string*>(GetProperty(name)) = value;
}

void UObject::SetName(const NameString& name, const NameString& value)
{
	*static_cast<NameString*>(GetProperty(name)) = value;
}

void UObject::SetObject(const NameString& name, const UObject* value)
{
	*static_cast<const UObject**>(GetProperty(name)) = value;
}

bool UObject::IsA(const NameString& className) const
{
	UStruct* cls = Class;
	while (cls)
	{
		if (cls->Name == className)
			return true;
		cls = cls->BaseStruct;
	}
	return false;
}

bool UObject::IsEventEnabled(const NameString& name) const
{
	EventName eventName = {};
	if (NameStringToEventName(name, eventName))
	{
		return IsEventEnabled(eventName);
	}
	else
	{
		NameString stateName = GetStateName();
		auto it = DisabledEvents.find(stateName);
		return it == DisabledEvents.end() || it->second.find(name) == it->second.end();
	}
}

bool UObject::IsEventEnabled(EventName name) const
{
	int nameIndex = (int)name;
	if (nameIndex < 64) // Probe event
	{
		bool foundProbe = false;
		if (StateFrame && StateFrame->Func)
		{
			UState* state = static_cast<UState*>(StateFrame->Func);

			// Probe is in the ignore list ('ignores' keyword in unrealscript)
			if ((state->IgnoreMask & (1ULL << nameIndex)) == 0)
			{
				return false;
			}

			// We have a function for the probe
			if ((state->ProbeMask & (1ULL << nameIndex)) != 0)
			{
				foundProbe = true;
			}
		}

		// Maybe the class has a function for our probe?
		if ((Class->ProbeMask & (1ULL << nameIndex)) != 0)
		{
			foundProbe = true;
		}

		if (!foundProbe)
			return false;
	}

	NameString stateName = GetStateName();
	auto it = DisabledEvents.find(stateName);
	return it == DisabledEvents.end() || it->second.find(ToNameString(name)) == it->second.end();
}

std::string UObject::PrintProperties()
{
	std::string result;

	for (UProperty* prop : PropertyData.Class->Properties)
	{
		result += prop->Name.ToString();
		result += " = ";
		void* ptr = PropertyData.Ptr(prop);
		result += prop->PrintValue(ptr);
		result += "\n";
	}

	return result;
}

Array<UProperty*> UObject::GetAllProperties()
{
	Array<UProperty*> result;

	for (UProperty* prop : PropertyData.Class->Properties)
	{
		result.push_back(prop);
	}

	return result;
}

Array<UProperty*> UObject::GetAllUserEditableProperties()
{
	Array<UProperty*> result;

	for (UProperty* prop : PropertyData.Class->Properties)
	{
		if (bool(prop->PropFlags & PropertyFlags::Edit))
			result.push_back(prop);
	}

	return result;
}

Array<UProperty*> UObject::GetAllTravelProperties()
{
	Array<UProperty*> result;

	for (UProperty* prop : PropertyData.Class->Properties)
	{
		if (bool(prop->PropFlags & PropertyFlags::Travel))
			result.push_back(prop);
	}

	return result;
}

NameString UObject::GetStateName() const
{
	return StateFrame && StateFrame->Func ? StateFrame->Func->Name : NameString();
}

void UObject::GotoState(NameString stateName, const NameString& labelName)
{
	if (stateName == "Auto")
	{
		bool foundState = false;
		for (UClass* cls = Class; cls != nullptr && !foundState; cls = static_cast<UClass*>(cls->BaseStruct))
		{
			for (auto& it : cls->States)
			{
				UState* state = it.second;
				if ((state->StateFlags & ScriptStateFlags::Auto) == ScriptStateFlags::Auto)
				{
					stateName = state->Name;
					foundState = true;
					break;
				}
			}
		}
	}

	UState* newState = nullptr;

	if (!stateName.IsNone())
	{
		for (UClass* cls = Class; cls != nullptr; cls = static_cast<UClass*>(cls->BaseStruct))
		{
			UState* state = cls->GetState(stateName);
			if (state)
			{
				newState = state;
				break;
			}
		}
	}

	if (!StateFrame)
		StateFrame = std::make_shared<Frame>(this, nullptr);

	UState* oldState = (UState*)StateFrame->Func;

	if (oldState && oldState != newState)
		CallEvent(this, EventName::EndState);

	if (oldState != newState)
		StateFrame->SetState(newState);

	if (newState)
		StateFrame->GotoLabel(labelName);

	if (newState && oldState != newState)
		CallEvent(this, EventName::BeginState);
}

GCAllocation* UObject::Mark(GCAllocation* marklist)
{
	//for (UProperty* prop : Class->Properties)
	//	marklist = prop->MarkProperty(marklist, PropertyData);
	return marklist;
}

/////////////////////////////////////////////////////////////////////////////

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
	Size = (cls->StructSize + 7) / 8;
	Data = new int64_t[Size];
	Size *= 8;
	for (UProperty* prop : cls->Properties)
	{
#ifdef _DEBUG
		if (prop->DataOffset.DataOffset + prop->Size() > cls->StructSize)
			Exception::Throw("Memory corruption detected!");
#endif

		if (&cls->PropertyData != this)
			prop->CopyConstruct(Ptr(prop), cls->PropertyData.Ptr(prop));
		else if (cls->BaseStruct && prop->DataOffset.DataOffset < cls->BaseStruct->PropertyData.Size) // inherit from base default object
			prop->CopyConstruct(Ptr(prop), cls->BaseStruct->PropertyData.Ptr(prop));
		else
			prop->Construct(Ptr(prop));
	}
}

void PropertyDataBlock::Load(ObjectStream* stream)
{
	while (true)
	{
		NameString name = stream->ReadName();
		if (name == "None")
			break;

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
		case 5: header.size = stream->ReadUInt8(); break;
		case 6: header.size = stream->ReadUInt16(); break;
		case 7: header.size = stream->ReadUInt32(); break;
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

		UProperty* prop = nullptr;
		for (auto it = Class->Properties.rbegin(); it != Class->Properties.rend(); ++it)
		{
			if ((*it)->Name == name)
			{
				// This is a hack. GameReplicationInfo and its base class Actor both have a "Region" property.
				// How is the property loading code supposed to know is being targeted?
				// Only other information we have is the property type.
				if (header.type != UPT_Struct || UObject::TryCast<UStructProperty>(*it))
				{
					prop = *it;
					break;
				}
			}
		}

		if (!prop)
		{
#if 0
			Exception::Throw("Unknown property " + name);
#else
			LogMessage("Skipping unknown property " + name.ToString());
			if (header.type != UPT_Bool)
				stream->Skip(header.size);
			continue;
#endif
		}

		void* data = Ptr(prop);

		if (header.arrayIndex < 0 || header.arrayIndex >= prop->ArrayDimension)
			Exception::Throw("Array property is out of bounds!");

		prop->Flags |= ObjectFlags::TagExp;
		prop->LoadValue(static_cast<uint8_t*>(data) + header.arrayIndex * prop->ElementSize(), stream, header);
	}
}

void PropertyDataBlock::Save(PackageStreamWriter* stream, PropertyDataBlock* defaultBlock)
{
	for (UProperty* prop : Class->Properties)
	{
		// Native properties are saved by the objects themselves
		// Transient properties don't get saved
		if (AnyFlags(prop->PropFlags, PropertyFlags::Native | PropertyFlags::Transient))
			continue;

		for (int arrayIndex = 0; arrayIndex < prop->ArrayDimension; arrayIndex++)
		{
			size_t offset = prop->ElementSize() * arrayIndex;
			void* data = static_cast<uint8_t*>(Ptr(prop)) + offset;

			if (defaultBlock)
			{
				void* defaultdata = static_cast<uint8_t*>(defaultBlock->Ptr(prop)) + offset;
				if (prop->Compare(data, defaultdata))
					continue;
			}

			PropertyHeader header = {};
			header.arrayIndex = arrayIndex;
			prop->SaveHeader(data, header);
			if (header.type == UPT_Invalid)
			{
				LogMessage("Skipping saving invalid property " + prop->Name.ToString());
				continue;
			}

			stream->BeginProperty(prop->Name);
			prop->SaveValue(data, stream);
			stream->EndProperty(header);
		}
	}

	stream->WriteName(NameString());
}
