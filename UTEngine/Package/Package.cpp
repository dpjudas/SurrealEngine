
#include "Precomp.h"
#include "Package.h"
#include "PackageStream.h"
#include "PackageManager.h"
#include "UObject/UObject.h"
#include "UObject/UFont.h"
#include "UObject/ULevel.h"
#include "UObject/UMesh.h"
#include "UObject/UTexture.h"
#include "UObject/UActor.h"
#include "File.h"

Package::Package(PackageManager* packageManager, const std::string& name, const std::string& filename) : Packages(packageManager), Name(name), Filename(filename)
{
	ReadTables();
	ObjectAllocations.resize(ExportTable.size());
	Objects.resize(ExportTable.size());

	RegisterNativeClass<UObject>("Object");
	RegisterNativeClass<UFont>("Font");
	RegisterNativeClass<UModel>("Model");
	RegisterNativeClass<ULevelBase>("LevelBase");
	RegisterNativeClass<ULevel>("Level");
	RegisterNativeClass<UActor>("Actor");
	RegisterNativeClass<UPrimitive>("Primitive");
	RegisterNativeClass<UMesh>("Mesh");
	RegisterNativeClass<ULodMesh>("LodMesh");
	RegisterNativeClass<UTexture>("Texture");
	RegisterNativeClass<UFractalTexture>("FractalTexture");
	RegisterNativeClass<UFireTexture>("FireTexture");
	RegisterNativeClass<UIceTexture>("IceTexture");
	RegisterNativeClass<UWaterTexture>("WaterTexture");
	RegisterNativeClass<UWaveTexture>("WaveTexture");
	RegisterNativeClass<UScriptedTexture>("ScriptedTexture");
	RegisterNativeClass<UPalette>("Palette");
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

	int clsobjref = entry->ObjClass;
	Package* clspackage = this;

	std::string firstclassname;

	while (true)
	{
		// If the class is defined in a different package, find the export entry in that package
		if (clsobjref < 0)
		{
			ImportTableEntry* importentry = clspackage->GetImportEntry(clsobjref);
			std::string clsname = clspackage->GetName(importentry->ObjName);
			if (firstclassname.empty()) firstclassname = clsname;
			auto it = NativeClasses.find(GetNameKey(clsname));
			if (it != NativeClasses.end())
			{
				(it->second)(this, index, firstclassname);
				return;
			}

			if (importentry->ObjPackage > 0)
			{
				ExportTableEntry* pkgentry = clspackage->GetExportEntry(importentry->ObjPackage);
				clspackage = Packages->GetPackage(clspackage->GetName(pkgentry->ObjName));
				clsobjref = clspackage->FindObjectReference("Class", clsname);
			}
			else if (importentry->ObjPackage < 0)
			{
				ImportTableEntry* pkgentry = clspackage->GetImportEntry(importentry->ObjPackage);

				while (pkgentry->ObjPackage != 0) // Groups
					pkgentry = clspackage->GetImportEntry(pkgentry->ObjPackage);

				clspackage = Packages->GetPackage(clspackage->GetName(pkgentry->ObjName));
				clsobjref = clspackage->FindObjectReference("Class", clsname);
			}
			else
			{
				throw std::runtime_error("ObjPackage is null");
			}
		}

		ExportTableEntry* clsentry = clspackage->GetExportEntry(clsobjref);
		std::string clsname = clspackage->GetName(clsentry->ObjName);
		if (firstclassname.empty()) firstclassname = clsname;

		auto it = NativeClasses.find(GetNameKey(clsname));
		if (it != NativeClasses.end())
		{
			(it->second)(this, index, firstclassname);
			return;
		}

		if (clsentry->ObjBase == 0)
			throw std::runtime_error("Could not find the object base class");

		clsobjref = clsentry->ObjBase;
	}
}

UObject* Package::GetUObject(int objref)
{
	if (objref > 0) // Export table object
	{
		int index = objref - 1;
		if (!Objects[index])
			LoadExportObject(index);
		return Objects[index];
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

		return Packages->GetPackage(packageName)->GetUObject(className, objectName, groupName);
	}
	else
	{
		return nullptr;
	}
}

UObject* Package::GetUObject(const std::string& className, const std::string& objectName, const std::string& groupName)
{
	return GetUObject(FindObjectReference(className, objectName, groupName));
}

int Package::FindObjectReference(const std::string& className, const std::string& objectName, const std::string& groupName)
{
	int classIndex = FindNameIndex(className);
	int objectIndex = FindNameIndex(objectName);
	int groupIndex = FindNameIndex(groupName);

	if ((className != "Class" && classIndex == -1) || objectIndex == -1 || (!groupName.empty() && groupIndex == -1))
		return 0;

	size_t count = ExportTable.size();
	for (size_t index = 0; index < count; index++)
	{
		ExportTableEntry& entry = ExportTable[index];
		if (entry.ObjName != objectIndex)
			continue;

		if (!groupName.empty())
		{
			if (entry.ObjPackage > 0)
			{
				auto package = GetExportEntry(entry.ObjPackage);
				if (package && groupIndex != package->ObjName)
					continue;
			}
			else if (entry.ObjPackage < 0)
			{
				auto package = GetImportEntry(entry.ObjPackage);
				if (package && groupIndex != package->ObjName)
					continue;
			}
			else
			{
				continue;
			}
		}

		if (entry.ObjClass == 0)
		{
			if (className == "Class")
				return (int)index + 1;
		}
		else if (entry.ObjClass < 0)
		{
			auto classImport = GetImportEntry(entry.ObjClass);
			if (classImport && classIndex == classImport->ObjName)
				return (int)index + 1;
		}
		else
		{
			auto classExport = &ExportTable[entry.ObjClass + 1];
			if (classExport && classIndex == classExport->ObjName)
				return (int)index + 1;
		}
	}

	return 0;
}

int Package::FindNameIndex(std::string name)
{
	auto it = NameHash.find(GetNameKey(name));
	if (it != NameHash.end())
		return it->second;
	else
		return -1;
}

std::string Package::GetName(int index)
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

std::unique_ptr<ObjectStream> Package::OpenObjectStream(int index, std::string classname)
{
	const auto& entry = ExportTable[index];
	if (entry.ObjSize > 0)
	{
		std::unique_ptr<uint64_t[]> buffer(new uint64_t[(entry.ObjSize + 7) / 8]);
		auto stream = OpenStream();
		stream->Seek(entry.ObjOffset);
		stream->ReadBytes(buffer.get(), entry.ObjSize);
		return std::make_unique<ObjectStream>(this, std::move(buffer), entry.ObjOffset, entry.ObjSize, entry.ObjFlags, classname);
	}
	else
	{
		return std::make_unique<ObjectStream>(this, std::unique_ptr<uint64_t[]>(), 0, 0, entry.ObjFlags, classname);
	}
}
