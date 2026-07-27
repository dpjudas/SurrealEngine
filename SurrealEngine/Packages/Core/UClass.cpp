
#include "Precomp.h"
#include "UClass.h"
#include "UEnum.h"
#include "Properties/UPointerProperty.h"
#include "Properties/UIntProperty.h"
#include "Properties/UClassProperty.h"
#include "Properties/UNameProperty.h"
#include "Properties/UStringProperty.h"
#include "Properties/UStrProperty.h"
#include "Properties/UBoolProperty.h"
#include "Properties/UByteProperty.h"
#include "Properties/UFloatProperty.h"
#include "VM/Bytecode.h"
#include "VM/NativeFunc.h"
#include "VM/ScriptCall.h"
#include "Package/PackageManager.h"
#include "Engine.h"

UClass::UClass(NameString name, UClass* base, ObjectFlags flags) : UState(std::move(name), nullptr, flags, base)
{
	if (base)
		ClsFlags = base->ClsFlags;

	if (name == "Object")
	{
		// Object is special as its the base class for everything. Described in Core.u with circular references.
		// This creates just enough of the properties to resolve it, hopefully.

		if (engine->LaunchInfo.IsUnrealTournament_469()) // 469 changed ObjectInternal from int to pointer
		{
			auto objInternal = GC::Alloc<UPointerProperty>("ObjectInternal", nullptr, ObjectFlags::Native);
			objInternal->ArrayDimension = 6;
			Properties.push_back(objInternal);
		}
		else if (!engine->LaunchInfo.IsUnreal1_227k()) // Unreal 227k has removed ObjectInternal entirely
		{
			auto objInternal = GC::Alloc<UIntProperty>("ObjectInternal", nullptr, ObjectFlags::Native);
			objInternal->ArrayDimension = 6;
			Properties.push_back(objInternal);
		}

		if (engine->LaunchInfo.IsUnreal1_227k())
			Properties.push_back(GC::Alloc<UIntProperty>("ObjectIndex", nullptr, ObjectFlags::Native));
		Properties.push_back(GC::Alloc<UObjectProperty>("Outer", nullptr, ObjectFlags::Native));
		if (engine->LaunchInfo.IsUnreal1_227k())
			Properties.push_back(GC::Alloc<UObjectProperty>("ObjectArchetype", nullptr, ObjectFlags::Native));
		Properties.push_back(GC::Alloc<UIntProperty>("ObjectFlags", nullptr, ObjectFlags::Native));
		Properties.push_back(GC::Alloc<UNameProperty>("Name", nullptr, ObjectFlags::Native));
		Properties.push_back(GC::Alloc<UClassProperty>("Class", nullptr, ObjectFlags::Native));

		size_t offset = 0;
		size_t structAlignment = 1;
		for (UProperty* prop : Properties)
		{
			size_t alignment = prop->ArrayAlignment();
			size_t size = prop->ArraySize();
			prop->DataOffset.DataOffset = (offset + alignment - 1) / alignment * alignment;
			offset = prop->DataOffset.DataOffset + size;
			structAlignment = std::max(structAlignment, alignment);
		}
		StructAlignment = structAlignment;
		StructSize = offset;
	}
}

void UClass::Load(ObjectStream* stream)
{
	UState::Load(stream);

	if (stream->GetVersion() <= 61)
	{
		OldClassRecordSize = stream->ReadUInt32();
		Flags = Flags | ObjectFlags::Public | ObjectFlags::Standalone;
	}

	ClsFlags = (ClassFlags)stream->ReadUInt32();
	stream->ReadBytes(ClassGuid.Data, 16);

	int NumDependencies = stream->ReadIndex();
	for (int i = 0; i < NumDependencies; i++)
	{
		ClassDependency dep;
		dep.Class = stream->ReadObject<UClass>();
		dep.Deep = stream->ReadUInt32();
		dep.ScriptTextCRC = stream->ReadUInt32();
		Dependencies.push_back(dep);
	}

	int NumPackageImports = stream->ReadIndex();
	for (int i = 0; i < NumPackageImports; i++)
		PackageImports.push_back(stream->ReadIndex());

	if (stream->GetVersion() >= 62)
	{
		ClassWithin = stream->ReadIndex();
		ClassConfigName = stream->ReadName();
	}

	PropertyData.Init(this);
	PropertyData.Load(stream);

	// Copy native UObject properties into the VM
	SetObject("Class", this);
	SetName("Name", Name);
	SetInt("ObjectFlags", (int)Flags);

	LoadProperties(&PropertyData);

	for (UField* child = Children; child; child = child->Next)
	{
		if (auto state = UObject::TryCast<UState>(child))
		{
			States[child->Name] = state;
		}
	}
}

void UClass::Save(PackageStreamWriter* stream)
{
	UState::Save(stream);

	if (stream->GetVersion() <= 61)
		stream->WriteUInt32(OldClassRecordSize);

	stream->WriteUInt32((uint32_t)ClsFlags);
	stream->WriteBytes(ClassGuid.Data, 16);

	stream->WriteIndex((int)Dependencies.size());
	for (const ClassDependency& dep : Dependencies)
	{
		stream->WriteObject(dep.Class);
		stream->WriteUInt32(dep.Deep);
		stream->WriteUInt32(dep.ScriptTextCRC);
	}

	stream->WriteIndex((int)PackageImports.size());
	for (int v : PackageImports)
		stream->WriteIndex(v);

	if (stream->GetVersion() >= 62)
	{
		stream->WriteIndex(ClassWithin);
		stream->WriteName(ClassConfigName);
	}

	PropertyData.Save(stream, nullptr);
}

std::map<NameString, std::string> UClass::ParseStructValue(const std::string& text)
{
	// Parse one of the following:
	//
	// Object=(Name=Package.ObjectName,Class=ObjectClass,MetaClass=Package.MetaClassName,Description="descriptive string")
	// Preferences=(Caption="display name",Parent="display name of parent",Class=Package.ClassName,Category=variable group name,Immediate=True)

	if (text.size() < 2 || text.front() != '(' || text.back() != ')')
		return {};

	std::map<NameString, std::string> desc;

	// This would have been so much easier with a regular expression, but we can't use that as we have no idea what character set those .int files might be using
	size_t pos = 1;
	while (pos < text.size() - 1)
	{
		size_t endpos = text.find('=', pos);
		if (endpos == std::string::npos)
			break;
		NameString keyname = text.substr(pos, endpos - pos);
		pos = endpos + 1;

		if (text[pos] == '"')
		{
			pos++;
			endpos = text.find('"', pos);
			if (endpos == std::string::npos)
				break;

			std::string value = text.substr(pos, endpos - pos);
			desc[keyname] = value;
			pos++;

			pos = text.find(',', pos);
			if (pos == std::string::npos)
				break;
			pos++;
		}
		else
		{
			endpos = text.find_first_of(",)", pos);
			if (endpos == std::string::npos)
				break;
			std::string value = text.substr(pos, endpos - pos);
			desc[keyname] = value;
			pos = endpos + 1;
		}
	}

	return desc;
}

UProperty* UClass::GetProperty(const NameString& propName)
{
	for (UProperty* prop : PropertyData.Class->Properties)
	{
		if (prop->Name == propName)
			return prop;
	}
	Exception::Throw("Class Property '" + Name.ToString() + "." + propName.ToString() + "' not found");
}

void UClass::SaveConfig()
{
	// Saves the default object properties to the ini file
	SaveProperties(&PropertyData);
}

void UClass::LoadProperties(PropertyDataBlock* propertyBlock)
{
	NameString sectionName = package->GetPackageName().ToString() + "." + Name.ToString();
	NameString configName = ClassConfigName;
	if (configName.IsNone()) configName = "system";
	for (UProperty* prop : Properties)
	{
		if (AnyFlags(prop->PropFlags, PropertyFlags::Config | PropertyFlags::GlobalConfig | PropertyFlags::Localized))
		{
			void* ptr = propertyBlock->Ptr(prop);
			for (int arrayIndex = 0; arrayIndex < prop->ArrayDimension; arrayIndex++)
			{
				NameString name = prop->Name;
				if (prop->ArrayDimension > 1)
					name = NameString(name.ToString() + "[" + std::to_string(arrayIndex) + "]");

				std::string value;
				if (AllFlags(prop->PropFlags, PropertyFlags::GlobalConfig))
				{
					if (UClass* outer = UObject::TryCast<UClass>(prop->Outer()))
					{
						NameString outerSectionName = outer->package->GetPackageName().ToString() + "." + outer->Name.ToString();
						NameString outerConfigName = outer->ClassConfigName;
						if (outerConfigName.IsNone()) outerConfigName = "system";
						value = package->GetPackageManager()->GetIniValue(outerConfigName, outerSectionName, name);
					}
				}
				else if (AllFlags(prop->PropFlags, PropertyFlags::Config))
				{
					value = package->GetPackageManager()->GetIniValue(configName, sectionName, name);
				}
				else if (AllFlags(prop->PropFlags, PropertyFlags::Localized))
				{
					value = package->GetPackageManager()->Localize(package->GetPackageName(), Name, name);
				}

				if (!value.empty())
				{
					if (auto byteprop = UObject::TryCast<UByteProperty>(prop))
					{
						if (value.front() >= '0' && value.front() <= '9')
						{
							*static_cast<uint8_t*>(ptr) = (uint8_t)std::atoi(value.c_str());
						}
						else if (byteprop->EnumType)
						{
							int index = 0;
							for (const NameString& elementName : byteprop->EnumType->ElementNames)
							{
								if (elementName == value)
								{
									*static_cast<uint8_t*>(ptr) = (uint8_t)index;
									break;
								}
								index++;
							}
						}
					}
					else if (UObject::IsType<UIntProperty>(prop)) *static_cast<int32_t*>(ptr) = (int32_t)std::atoi(value.c_str());
					else if (UObject::IsType<UFloatProperty>(prop)) *static_cast<float*>(ptr) = (float)std::atof(value.c_str());
					else if (UObject::IsType<UNameProperty>(prop)) *static_cast<NameString*>(ptr) = value;
					else if (UObject::IsType<UStrProperty>(prop)) *static_cast<std::string*>(ptr) = value;
					else if (UObject::IsType<UStringProperty>(prop)) *static_cast<std::string*>(ptr) = value;
					else if (auto boolprop = UObject::TryCast<UBoolProperty>(prop))
					{
						for (char& c : value)
							if (c >= 'A' && c <= 'Z')
								c += 'a' - 'A';
						boolprop->SetBool(ptr, value == "1" || value == "true" || value == "yes");
					}
					else if (UObject::IsType<UClassProperty>(prop))
					{
						try
						{
							size_t pos = value.find_first_of('.');
							if (pos != std::string::npos)
							{
								NameString packageName = value.substr(0, pos);
								NameString className = value.substr(pos + 1);
								Package* pkg = package->GetPackageManager()->GetPackage(packageName);
								*static_cast<UObject**>(ptr) = pkg->GetUObject("Class", className);
							}
						}
						catch (...)
						{
							// To do: is this actually a fatal error?
						}
					}
					else if (auto structprop = UObject::TryCast<UStructProperty>(prop))
					{
						// Yes, this is total spaghetti code at this point. No, I don't care anymore. ;)
						auto values = ParseStructValue(value);
						for (UProperty* member : structprop->Struct->Properties)
						{
							std::string membervalue = values[member->Name];
							void* memberptr = static_cast<uint8_t*>(ptr) + member->DataOffset.DataOffset;
							if (auto byteprop = UObject::TryCast<UByteProperty>(member))
							{
								if (!membervalue.empty() && membervalue.front() >= '0' && membervalue.front() <= '9')
								{
									*static_cast<uint8_t*>(memberptr) = (uint8_t)std::atoi(membervalue.c_str());
								}
								else if (byteprop->EnumType)
								{
									int index = 0;
									for (const NameString& elementName : byteprop->EnumType->ElementNames)
									{
										if (elementName == membervalue)
										{
											*static_cast<uint8_t*>(memberptr) = (uint8_t)index;
											break;
										}
										index++;
									}
								}
							}
							else if (UObject::IsType<UIntProperty>(member)) *static_cast<int32_t*>(memberptr) = (int32_t)std::atoi(membervalue.c_str());
							else if (UObject::IsType<UFloatProperty>(member)) *static_cast<float*>(memberptr) = (float)std::atof(membervalue.c_str());
							else if (UObject::IsType<UNameProperty>(member)) *static_cast<NameString*>(memberptr) = membervalue;
							else if (UObject::IsType<UStrProperty>(member)) *static_cast<std::string*>(memberptr) = membervalue;
							else if (UObject::IsType<UStringProperty>(member)) *static_cast<std::string*>(memberptr) = membervalue;
							else if (auto boolprop = UObject::TryCast<UBoolProperty>(member))
							{
								for (char& c : membervalue)
									if (c >= 'A' && c <= 'Z')
										c += 'a' - 'A';
								boolprop->SetBool(memberptr, membervalue == "1" || membervalue == "true" || membervalue == "yes");
							}
							else if (UObject::IsType<UClassProperty>(member))
							{
								try
								{
									size_t pos = membervalue.find_first_of('.');
									if (pos != std::string::npos)
									{
										NameString packageName = membervalue.substr(0, pos);
										NameString className = membervalue.substr(pos + 1);
										Package* pkg = package->GetPackageManager()->GetPackage(packageName);
										*static_cast<UObject**>(memberptr) = pkg->GetUObject("Class", className);
									}
								}
								catch (...)
								{
									// To do: is this actually a fatal error?
								}
							}
							else if (UObject::IsType<UObjectProperty>(member))
							{
								// This happens for Deus Ex
							}
							else
							{
								Exception::Throw("localize keyword used on unsupported struct member property type");
							}
						}
					}
					else
					{
						Exception::Throw("localize keyword used on unsupported property type");
					}
				}

				ptr = static_cast<uint8_t*>(ptr) + prop->ElementPitch();
			}
		}
	}
}

void UClass::SaveProperties(PropertyDataBlock* propertyBlock)
{
	if (!(ClsFlags & ClassFlags::Config))
		return;

	NameString sectionName = package->GetPackageName().ToString() + "." + Name.ToString();
	NameString configName = ClassConfigName;
	if (configName.IsNone()) configName = "system";

	for (UProperty* prop : Properties)
	{
		if (AnyFlags(prop->PropFlags, PropertyFlags::Config | PropertyFlags::GlobalConfig))
		{
			auto ptr = propertyBlock->Ptr(prop);
			for (int arrayIndex = 0; arrayIndex < prop->ArrayDimension; arrayIndex++)
			{
				NameString name = prop->Name;
				if (prop->ArrayDimension > 1)
					name = NameString(name.ToString() + "[" + std::to_string(arrayIndex) + "]");

				bool unsupported = false;
				std::string value;
				if (UObject::IsType<UByteProperty>(prop)) value = std::to_string(*static_cast<uint8_t*>(ptr));
				else if (UObject::IsType<UIntProperty>(prop)) value = std::to_string(*static_cast<int32_t*>(ptr));
				else if (UObject::IsType<UFloatProperty>(prop)) value = std::to_string(*static_cast<float*>(ptr));
				else if (UObject::IsType<UNameProperty>(prop)) value = (*static_cast<NameString*>(ptr)).ToString();
				else if (UObject::IsType<UStrProperty>(prop)) value = *static_cast<std::string*>(ptr);
				else if (UObject::IsType<UStringProperty>(prop)) value = *static_cast<std::string*>(ptr);
				else if (auto boolprop = UObject::TryCast<UBoolProperty>(prop)) value = boolprop->GetBool(ptr) ? "True" : "False";
				else unsupported = true;

				if (!unsupported)
				{
					if (AnyFlags(prop->PropFlags, PropertyFlags::GlobalConfig))
					{
						if (UClass* outer = UObject::TryCast<UClass>(prop->Outer()))
						{
							NameString outerSectionName = outer->package->GetPackageName().ToString() + "." + outer->Name.ToString();
							NameString outerConfigName = outer->ClassConfigName;
							if (outerConfigName.IsNone()) outerConfigName = "system";
							package->GetPackageManager()->SetIniValue(outerConfigName, outerSectionName, name, value);
						}
					}
					else if (AnyFlags(prop->PropFlags, PropertyFlags::Config))
					{
						package->GetPackageManager()->SetIniValue(configName, sectionName, name, value);
					}
				}
				ptr = static_cast<uint8_t*>(ptr) + prop->ElementPitch();
			}
		}
	}

	if (propertyBlock != &PropertyData)
	{
		// If its not our own block getting saved we need to load them into our default block
		LoadProperties(&PropertyData);
	}

	// GlobalConfig properties are shared across the whole class hierarchy that
	// declares them, so a change must propagate to every class deriving from the
	// *declaring* class - not just classes deriving from `this`. Otherwise sibling
	// classes (e.g. UMenuLoadGameClientWindow vs UMenuSaveGameClientWindow, which
	// both inherit globalconfig SlotNames[] from UMenuSlotClientWindow) keep a stale
	// default until the next restart re-reads the ini.
	Array<UStruct*> propagationRoots;
	propagationRoots.push_back(this);
	for (UProperty* prop : Properties)
	{
		if (AnyFlags(prop->PropFlags, PropertyFlags::GlobalConfig))
		{
			if (UClass* declaring = UObject::TryCast<UClass>(prop->Outer()))
			{
				bool known = false;
				for (UStruct* root : propagationRoots)
					if (root == declaring) { known = true; break; }
				if (!known)
					propagationRoots.push_back(declaring);
			}
		}
	}

	for (GCObject* gcObj : GC::GetObjects())
	{
		if (UClass* cls = dynamic_cast<UClass*>(gcObj))
		{
			bool propagate = false;
			for (UStruct* base = cls; base && !propagate; base = base->BaseStruct)
			{
				for (UStruct* root : propagationRoots)
				{
					if (base == root)
					{
						propagate = true;
						break;
					}
				}
			}
			// Reload through cls's own property list (not this->Properties), so the
			// property offsets match cls's data block. this->Properties can be a
			// superset of an ancestor/sibling's layout and would overrun its block.
			if (propagate && cls != this)
				cls->LoadProperties(&cls->PropertyData);
		}
	}
}
