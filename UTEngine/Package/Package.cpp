
#include "Precomp.h"
#include "Package.h"
#include "PackageStream.h"
#include "PackageManager.h"
#include "UObject/UObject.h"
#include "UObject/UClass.h"
#include "UObject/UFont.h"
#include "UObject/ULevel.h"
#include "UObject/UMesh.h"
#include "UObject/UTexture.h"
#include "UObject/UActor.h"
#include "UObject/USound.h"
#include "UObject/UMusic.h"
#include "UObject/UTextBuffer.h"
#include "File.h"

Package::Package(PackageManager* packageManager, const std::string& name, const std::string& filename) : Packages(packageManager), Name(name), Filename(filename)
{
	ReadTables();

	RegisterNativeClass<UObject>("Object");
	RegisterNativeClass<UField>("Field");
	RegisterNativeClass<UConst>("Const");
	RegisterNativeClass<UEnum>("Enum");
	RegisterNativeClass<UProperty>("Property");
	RegisterNativeClass<UByteProperty>("ByteProperty");
	RegisterNativeClass<UObjectProperty>("ObjectProperty");
	RegisterNativeClass<UFixedArrayProperty>("FixedArrayProperty");
	RegisterNativeClass<UArrayProperty>("ArrayProperty");
	RegisterNativeClass<UMapProperty>("MapProperty");
	RegisterNativeClass<UClassProperty>("ClassProperty");
	RegisterNativeClass<UStructProperty>("StructProperty");
	RegisterNativeClass<UIntProperty>("IntProperty");
	RegisterNativeClass<UBoolProperty>("BoolProperty");
	RegisterNativeClass<UFloatProperty>("FloatProperty");
	RegisterNativeClass<UNameProperty>("NameProperty");
	RegisterNativeClass<UStrProperty>("StrProperty");
	RegisterNativeClass<UStringProperty>("StringProperty");
	RegisterNativeClass<UStruct>("Struct");
	RegisterNativeClass<UFunction>("Function");
	RegisterNativeClass<UState>("State");
	RegisterNativeClass<UClass>("Class");
	RegisterNativeClass<UFont>("Font");
	RegisterNativeClass<UModel>("Model");
	RegisterNativeClass<ULevelBase>("LevelBase");
	RegisterNativeClass<ULevel>("Level");
	RegisterNativeClass<UActor>("Actor");
	RegisterNativeClass<UPrimitive>("Primitive");
	RegisterNativeClass<UMesh>("Mesh");
	RegisterNativeClass<ULodMesh>("LodMesh");
	RegisterNativeClass<USkeletalMesh>("SkeletalMesh");
	RegisterNativeClass<UAnimation>("Animation");
	RegisterNativeClass<UTexture>("Texture");
	RegisterNativeClass<UFractalTexture>("FractalTexture");
	RegisterNativeClass<UFireTexture>("FireTexture");
	RegisterNativeClass<UIceTexture>("IceTexture");
	RegisterNativeClass<UWaterTexture>("WaterTexture");
	RegisterNativeClass<UWaveTexture>("WaveTexture");
	RegisterNativeClass<UScriptedTexture>("ScriptedTexture");
	RegisterNativeClass<UPalette>("Palette");
	RegisterNativeClass<USound>("Sound");
	RegisterNativeClass<UMusic>("Music");
	RegisterNativeClass<UTextBuffer>("TextBuffer");

	if (GetNameKey(name) == "engine" || GetNameKey(name) == "core")
	{
		for (auto& it : NativeClasses)
		{
			std::string name = it.first;
			int objref = FindObjectReference("Class", name);
			if (objref == 0)
			{
				if (NameHash.find(GetNameKey(name)) == NameHash.end())
				{
					NameTableEntry nameentry;
					nameentry.Flags = 0;
					nameentry.Name = name;
					NameTable.push_back(nameentry);
					NameHash[GetNameKey(name)] = (int)NameTable.size() - 1;
				}

				ExportTableEntry entry;
				entry.ObjClass = 0;
				entry.ObjBase = 0;
				entry.ObjPackage = 0;
				entry.ObjName = NameHash[GetNameKey(name)];
				entry.ObjFlags = ObjectFlags::Native;
				entry.ObjSize = 0;
				entry.ObjOffset = 0;
				ExportTable.push_back(entry);
			}
		}
	}

	ObjectAllocations.resize(ExportTable.size());
	Objects.resize(ExportTable.size());
}

Package::~Package()
{
	for (UObject* obj : Objects)
	{
		if (obj)
		{
			obj->~UObject();
			obj = nullptr;
		}
	}
}

void Package::LoadExportObject(int index)
{
	const ExportTableEntry* entry = &ExportTable[index];

	SetDelayLoadActive delayload(this);

	std::string objname = GetName(entry->ObjName);

	UClass* objclass = UObject::Cast<UClass>(GetUObject(entry->ObjClass));
	if (objclass)
	{
		for (UClass* cur = objclass; cur != nullptr; cur = cur->Base)
		{
			auto it = NativeClasses.find(GetNameKey(cur->Name));
			if (it != NativeClasses.end())
			{
				it->second(this, index, objname, objclass);
				return;
			}
		}

		throw std::runtime_error("Could not find the object class for " + objname);
	}
	else
	{
		UClass* objbase = UObject::Cast<UClass>(GetUObject(entry->ObjBase));
		NativeClasses[GetNameKey("Class")](this, index, objname, objbase);
	}
}

void Package::DelayLoadNow()
{
	while (!delayLoads.empty())
	{
		SetDelayLoadActive delayload(this);

		auto func = delayLoads.back();
		delayLoads.pop_back();
		func();
	}

	while (!delayLoadTypeValidations.empty())
	{
		auto func = delayLoadTypeValidations.back();
		delayLoadTypeValidations.pop_back();
		func();
	}
}

UObject* Package::GetUObject(int objref, std::function<void(UObject*)> validateTypeCast)
{
	if (objref > 0) // Export table object
	{
		int index = objref - 1;
		if (!Objects[index])
			LoadExportObject(index);

		UObject* obj = Objects[index];
		if (validateTypeCast)
			delayLoadTypeValidations.push_back([=]() { validateTypeCast(obj); });
		if (delayLoadActive == 0)
			DelayLoadNow();
		return obj;
	}
	else if (objref < 0) // Import table object
	{
		ImportTableEntry* entry = GetImportEntry(objref);
		ImportTableEntry* entrypackage = GetImportEntry(entry->ObjPackage);

		std::string groupName;
		while (entrypackage->ObjPackage != 0)
		{
			groupName = GetName(entrypackage->ObjName);
			entrypackage = GetImportEntry(entrypackage->ObjPackage);
		}

		std::string packageName = GetName(entrypackage->ObjName);
		std::string objectName = GetName(entry->ObjName);
		std::string className = GetName(entry->ClassName);
		// std::string classPackage = GetName(entry->ClassPackage);

		return Packages->GetPackage(packageName)->GetUObject(className, objectName, groupName, validateTypeCast);
	}
	else
	{
		return nullptr;
	}
}

UObject* Package::GetUObject(const std::string& className, const std::string& objectName, const std::string& groupName, std::function<void(UObject*)> validateTypeCast)
{
	return GetUObject(FindObjectReference(className, objectName, groupName), validateTypeCast);
}

int Package::FindObjectReference(const std::string& className, const std::string& objectName, const std::string& groupName)
{
	size_t count = ExportTable.size();
	for (size_t index = 0; index < count; index++)
	{
		ExportTableEntry& entry = ExportTable[index];
		if (!CompareNames(GetName(entry.ObjName), objectName))
			continue;

		if (!groupName.empty())
		{
			if (entry.ObjPackage > 0)
			{
				auto package = GetExportEntry(entry.ObjPackage);
				if (package && !CompareNames(groupName, GetName(package->ObjName)))
					continue;
			}
			else if (entry.ObjPackage < 0)
			{
				auto package = GetImportEntry(entry.ObjPackage);
				if (package && !CompareNames(groupName, GetName(package->ObjName)))
					continue;
			}
			else
			{
				continue;
			}
		}

		if (entry.ObjClass == 0)
		{
			if (CompareNames(className, "Class"))
				return (int)index + 1;
		}
		else if (entry.ObjClass < 0)
		{
			auto classImport = GetImportEntry(entry.ObjClass);
			if (classImport && CompareNames(className, GetName(classImport->ObjName)))
				return (int)index + 1;
		}
		else
		{
			auto classExport = &ExportTable[entry.ObjClass + 1];
			if (classExport && CompareNames(className, GetName(classExport->ObjName)))
				return (int)index + 1;
		}
	}

	return 0;
}

const std::string& Package::GetName(int index) const
{
	if (index >= 0 && (size_t)index < NameTable.size())
		return NameTable[index].Name;
	else
		throw std::runtime_error("Name index out of bounds!");
}

ExportTableEntry* Package::GetExportEntry(int objref)
{
	if (objref == 0)
		return nullptr;
	else if (objref < 0)
		throw std::runtime_error("Expected an export table entry");

	int index = objref - 1;
	if ((size_t)index >= ExportTable.size())
		throw std::runtime_error("Export table entry out of bounds!");

	return ExportTable.data() + index;
}

ImportTableEntry* Package::GetImportEntry(int objref)
{
	if (objref == 0)
		return nullptr;
	else if (objref > 0)
		throw std::runtime_error("Expected an import table entry");

	int index = -objref - 1;
	if ((size_t)index >= ImportTable.size())
		throw std::runtime_error("Import table entry out of bounds!");

	return ImportTable.data() + index;
}

void Package::ReadTables()
{
	auto stream = OpenStream();

	uint32_t signature = stream->ReadInt32();
	if (signature != 0x9E2A83C1)
		throw std::runtime_error("Not an unreal package file");

	Version = stream->ReadInt16();
	uint16_t licenseeMode = stream->ReadInt16();

	if (Version < 60 || Version >= 100)
		throw std::runtime_error("Unsupported unreal package version");

	Flags = (PackageFlags)stream->ReadUInt32();

	uint32_t nameCount = stream->ReadInt32();
	uint32_t nameOffset = stream->ReadInt32();

	uint32_t exportCount = stream->ReadInt32();
	uint32_t exportOffset = stream->ReadInt32();

	uint32_t importCount = stream->ReadInt32();
	uint32_t importOffset = stream->ReadInt32();

	if (Version < 68)
	{
		uint32_t heritageCount = stream->ReadInt32();
		uint32_t heritageOffset = stream->ReadInt32();
	}
	else
	{
		uint8_t guid[16];
		stream->ReadBytes(guid, 16);
		uint32_t generationCount = stream->ReadInt32();
		for (uint32_t i = 0; i < generationCount; i++)
		{
			uint32_t genExportCount = stream->ReadInt32();
			uint32_t genNameCount = stream->ReadInt32();
		}
	}

	stream->Seek(nameOffset);
	for (uint32_t i = 0; i < nameCount; i++)
	{
		NameTableEntry entry;
		entry.Name = stream->ReadString();
		entry.Flags = stream->ReadInt32();
		NameTable.push_back(entry);
		NameHash[GetNameKey(entry.Name)] = i;
	}

	stream->Seek(exportOffset);
	for (uint32_t i = 0; i < exportCount; i++)
	{
		ExportTableEntry entry;
		entry.ObjClass = stream->ReadIndex();
		entry.ObjBase = stream->ReadIndex();
		entry.ObjPackage = stream->ReadInt32();
		entry.ObjName = stream->ReadIndex();
		entry.ObjFlags = (ObjectFlags)stream->ReadInt32();
		entry.ObjSize = stream->ReadIndex();
		entry.ObjOffset = (entry.ObjSize > 0) ? stream->ReadIndex() : -1;
		ExportTable.push_back(entry);
	}

	stream->Seek(importOffset);
	for (uint32_t i = 0; i < importCount; i++)
	{
		ImportTableEntry entry;
		entry.ClassPackage = stream->ReadIndex();
		entry.ClassName = stream->ReadIndex();
		entry.ObjPackage = stream->ReadInt32();
		entry.ObjName = stream->ReadIndex();
		ImportTable.push_back(entry);
	}
}

std::unique_ptr<PackageStream> Package::OpenStream()
{
	return std::make_unique<PackageStream>(this, File::open_existing(Filename));
}

std::unique_ptr<ObjectStream> Package::OpenObjectStream(int index, const std::string& name, UClass* base)
{
	const auto& entry = ExportTable[index];
	if (entry.ObjSize > 0)
	{
		std::unique_ptr<uint64_t[]> buffer(new uint64_t[(entry.ObjSize + 7) / 8]);
		auto stream = OpenStream();
		stream->Seek(entry.ObjOffset);
		stream->ReadBytes(buffer.get(), entry.ObjSize);
		return std::make_unique<ObjectStream>(this, std::move(buffer), entry.ObjOffset, entry.ObjSize, entry.ObjFlags, name, base);
	}
	else
	{
		return std::make_unique<ObjectStream>(this, std::unique_ptr<uint64_t[]>(), 0, 0, entry.ObjFlags, name, base);
	}
}
