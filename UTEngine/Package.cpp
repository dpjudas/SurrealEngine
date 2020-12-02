
#include "Precomp.h"
#include "Package.h"
#include "PackageManager.h"
#include "BinaryStream.h"
#include "File.h"

Package::Package(PackageManager* packageManager, const std::string& name, const std::string& filename) : Packages(packageManager), Name(name), Filename(filename)
{
	auto file = File::open_existing(filename);
	Data.resize(file->size());
	file->read(Data.data(), Data.size());
	file.reset();

	LoadHeader();
}

std::unique_ptr<BinaryStream> Package::OpenStream()
{
	return std::make_unique<BinaryStream>(Data.data(), Data.size());
}

ExportTableEntry* Package::FindExportObject(int objectIndex)
{
	if (objectIndex > 0)
	{
		return ExportForIndex(objectIndex);
	}
	else if (objectIndex < 0)
	{
		ImportTableEntry* entry = ImportForIndex(objectIndex);
		ImportTableEntry* entrypackage = ImportForIndex(entry->ObjPackage);

		std::string groupName;
		while (entrypackage->ObjPackage)
		{
			groupName = GetName(entrypackage->ObjName);
			entrypackage = ImportForIndex(entrypackage->ObjPackage);
		}

		std::string packageName = GetName(entrypackage->ObjName);
		std::string objectName = GetName(entry->ObjName);
		std::string className = GetName(entry->ClassName);
		// std::string classPackage = GetName(entry->ClassPackage);

		Package* package = Packages->GetPackage(packageName);
		return package->FindExportObject(className, objectName, groupName);
	}
	else
	{
		return nullptr;
	}
}

ExportTableEntry* Package::FindExportObject(std::string className, std::string objectName, std::string groupName)
{
	int classIndex = FindNameIndex(className);
	int objectIndex = FindNameIndex(objectName);
	int groupIndex = FindNameIndex(groupName);

	if (classIndex == -1 || objectIndex == -1)
		return nullptr;

	for (ExportTableEntry& entry : ExportTable)
	{
		if (entry.ObjName != objectIndex)
			continue;

		if (!groupName.empty() && entry.ObjPackage != 0)
		{
			if (entry.ObjPackage < 0)
			{
				auto package = ImportForIndex(entry.ObjPackage);
				if (package && groupIndex != package->ObjName)
					continue;
			}
			else
			{
				auto package = ExportForIndex(entry.ObjPackage);
				if (package && groupIndex != package->ObjName)
					continue;
			}
		}

		if (entry.ObjClass == 0)
		{
			if (!(entry.ObjFlags & RF_Native))
				continue;
			if (className == "Class")
				return &entry;
		}
		else if (entry.ObjClass < 0)
		{
			auto classImport = ImportForIndex(entry.ObjClass);
			if (classImport && classIndex == classImport->ObjName)
				return &entry;
		}
		else
		{
			auto classExport = ExportForIndex(entry.ObjClass);
			if (classExport && classIndex == classExport->ObjName)
				return &entry;
		}
	}

	return nullptr;
}

int Package::FindNameIndex(std::string name)
{
	auto it = NameHash.find(name);
	if (it != NameHash.end())
		return it->second;
	else
		return -1;
}

ExportTableEntry* Package::ExportForIndex(int index)
{
	if (index > 0 && index <= (int)ExportTable.size())
		return &ExportTable[(size_t)index - 1];
	else
		return nullptr;
}

ImportTableEntry* Package::ImportForIndex(int index)
{
	index = -index;
	if (index > 0 && index <= (int)ImportTable.size())
		return &ImportTable[(size_t)index - 1];
	else
		return nullptr;
}

void Package::LoadHeader()
{
	std::unique_ptr<BinaryStream> stream = OpenStream();

	uint32_t signature = stream->ReadInt32();
	if (signature != 0x9E2A83C1)
		throw std::runtime_error("Not an unreal package file");

	PackageVersion = stream->ReadInt16();
	uint16_t licenseeMode = stream->ReadInt16();

	if (PackageVersion < 60 || PackageVersion >= 100)
		throw std::runtime_error("Unsupported unreal package version");

	PackageFlags = stream->ReadInt32();

	uint32_t nameCount = stream->ReadInt32();
	uint32_t nameOffset = stream->ReadInt32();

	uint32_t exportCount = stream->ReadInt32();
	uint32_t exportOffset = stream->ReadInt32();

	uint32_t importCount = stream->ReadInt32();
	uint32_t importOffset = stream->ReadInt32();

	if (PackageVersion < 68)
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
	if (PackageVersion < 68)
	{
		for (uint32_t i = 0; i < nameCount; i++)
		{
			std::string name;
			while (true)
			{
				int8_t c = stream->ReadInt8();
				if (c == 0)
					break;
				name.push_back(c);
			}
			uint32_t flags = stream->ReadInt32();
			NameTable.push_back(name);
			NameHash[name] = i;
		}
	}
	else
	{
		for (uint32_t i = 0; i < nameCount; i++)
		{
			uint8_t len = stream->ReadInt8();
			if (len > 0)
			{
				char buffer[256];
				stream->ReadBytes(buffer, len);
				buffer[len - 1] = 0;
				uint32_t flags = stream->ReadInt32();
				NameTable.push_back(buffer);
				NameHash[buffer] = i;
			}
		}
	}

	stream->Seek(exportOffset);
	for (uint32_t i = 0; i < exportCount; i++)
	{
		ExportTableEntry entry;
		entry.Owner = this;
		entry.ObjClass = stream->ReadIndex();
		entry.ObjBase = stream->ReadIndex();
		entry.ObjPackage = stream->ReadInt32();
		entry.ObjName = stream->ReadIndex();
		entry.ObjFlags = stream->ReadInt32();
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

/////////////////////////////////////////////////////////////////////////////

std::unique_ptr<PackageObject> ExportTableEntry::Open()
{
	return std::make_unique<PackageObject>(Owner, this);
}

std::string ExportTableEntry::GetClsName()
{
	if (ObjClass > 0)
	{
		return Owner->GetName(Owner->ExportForIndex(ObjClass)->ObjName);
	}
	else if (ObjClass < 0)
	{
		return Owner->GetName(Owner->ImportForIndex(ObjClass)->ObjName);
	}
	else
	{
		return {};
	}
}
