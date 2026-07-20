
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
#include "Utils/AlignedAlloc.h"

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
			SetObject("Outer", info->Outer);
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
				SetObject("Outer", info->Outer);
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
					if (latentAction >= 0 && (size_t)latentAction < NativeFunctions::LatentActionByIndex.size())
					{
						StateFrame->LatentState = NativeFunctions::LatentActionByIndex[latentAction];
					}
					else
					{
						LogMessage("UObject::Load encountered out of bounds latent action index");
						StateFrame->LatentState = LatentRunState::Continue;
					}
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

		for (int i = 0; i < 64; i++)
		{
			if ((probeMask >> i) & 1)
			{
				DisableEvent(ToNameString((EventName)i));
			}
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
		// func and state: in this engine's VM, a StateFrame can only ever be suspended in
		// state code. Frame::CallScript runs called UFunctions to completion on their own
		// stack-local Frame before returning, so a latent wait (Sleep, MoveTo, ...) can only
		// happen directly in the state's own bytecode. GotoState/GotoLabel always assign
		// StateFrame->Func a UState*, never a plain UFunction. That means func (raw
		// StateFrame->Func) and state (StateFrame->Func cast to UState) always point at the
		// same object here - there is no case, today, where an actor is suspended in a plain
		// function distinct from its current state. Both are still written/read as separate
		// fields to match the on-disk format and because Load requires func && state to be
		// simultaneously non-null to rebuild anything; if that invariant is ever broken by a
		// future VM change (e.g. nested latent calls), this is the code that needs revisiting.
		UStruct* func = StateFrame ? StateFrame->Func : nullptr;
		UState* state = StateFrame ? UObject::TryCast<UState>(StateFrame->Func) : nullptr;
		int32_t latentAction = 0;
		int offset = -1;

		int64_t probeMask = 0;
		for (int i = 0; i < 64; i++)
		{
			if (IsEventDisabled((EventName)i))
			{
				probeMask |= 1ULL << (uint64_t)i;
			}
		}

		// A StateFrame can outlive its state: GotoState("") (what a bTriggerOnceOnly mover does once
		// it has fired, and what any script does to go dormant) keeps the frame but sets Func to
		// null, while LatentState is left at whatever it was - and defaults to Continue, never Stop.
		// So "has a frame and isn't stopped" is not enough to imply there is code to point into;
		// Func must be checked too, or this dereferences null. Func->Code can likewise be null for a
		// state that carries no bytecode. Both cases mean the same thing on disk - no resume point -
		// and func is written as null just below, which is exactly what Load needs to see to bring
		// the actor back with StateFrame == nullptr, i.e. dormant.
		if (StateFrame && StateFrame->Func && StateFrame->LatentState != LatentRunState::Stop)
		{
			auto it = NativeFunctions::IndexForLatentAction.find(StateFrame->LatentState);
			latentAction = (it != NativeFunctions::IndexForLatentAction.end()) ? it->second : 0;
			if (StateFrame->Func->Code)
				offset = StateFrame->Func->Code->FindOffset((int)StateFrame->StatementIndex);
		}

		stream->WriteObject(func);
		stream->WriteObject(state);
		stream->WriteInt64(probeMask);
		stream->WriteInt32(latentAction);
		if (func)
			stream->WriteIndex(offset);
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
		if (!Class)
			return false;
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

bool UObject::IsEventDisabled(EventName name) const
{
	NameString stateName = GetStateName();
	auto it = DisabledEvents.find(stateName);
	return it != DisabledEvents.end() && it->second.find(ToNameString(name)) != it->second.end();
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
	// Only DeusEx marks Inventory as a travel property.

	Array<UProperty*> result;
	for (UProperty* prop : PropertyData.Class->Properties)
	{
		if (AllFlags(prop->PropFlags, PropertyFlags::Travel) || prop->Name == "Inventory")
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
	{
		// HasStack is otherwise only ever inherited from whatever the original package's export
		// table said at load time, never updated at runtime. An actor that starts a level with no
		// baked-in stack (the common case - most actors sit in their default state until something
		// triggers them) but enters a real state during play would otherwise never have that state
		// written by Save() at all, silently losing it on the next save regardless of what state
		// it's actually in. Mark it here, as early as possible, so any later save's export-table
		// capture of Flags (which happens before Save() runs, see PackageWriter::GetObjectReference)
		// sees it.
		Flags |= ObjectFlags::HasStack;
		StateFrame->GotoLabel(labelName);
	}

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
	AlignedFree(Data);
	Data = nullptr;
	Class = nullptr;
}

void PropertyDataBlock::Init(UClass* cls)
{
	Reset();

	bool isDefaultBlock = (&cls->PropertyData == this);

	Class = cls;
	Size = cls->StructSize;
	Data = AlignedAlloc(cls->StructAlignment, cls->StructSize);

	for (UProperty* prop : cls->Properties)
	{
#ifdef _DEBUG
		if (prop->DataOffset.DataOffset + prop->ArraySize() > cls->StructSize)
			Exception::Throw("Memory corruption detected!");
#endif

		if (!isDefaultBlock)
		{
			// This is an object instance.
			// Copy all properties over from the class default block.
			prop->CopyConstructArray(Ptr(prop), cls->PropertyData.Ptr(prop));
		}
		else
		{
			// This is a default block.
			// If the property comes from the base class, copy its value over from its default block.
			// Otherwise this is a new property for this class, which we default (zero) initialize.
			if (cls->BaseStruct && prop->DataOffset.DataOffset < cls->BaseStruct->PropertyData.Size)
				prop->CopyConstructArray(Ptr(prop), cls->BaseStruct->PropertyData.Ptr(prop));
			else
				prop->ConstructArray(Ptr(prop));
		}
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

		if (Class->Name == "ConEventComment" && name == "commentText") // Deus Ex hack. We can't read this string for some weird reason.
		{
			stream->Skip(header.size);
			continue;
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
		prop->LoadValue(static_cast<uint8_t*>(data) + header.arrayIndex * prop->ElementPitch(), stream, header);
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
			size_t offset = prop->ElementPitch() * arrayIndex;
			void* data = static_cast<uint8_t*>(Ptr(prop)) + offset;

			if (defaultBlock)
			{
				void* defaultdata = static_cast<uint8_t*>(defaultBlock->Ptr(prop)) + offset;
				if (prop->CompareElement(data, defaultdata))
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
