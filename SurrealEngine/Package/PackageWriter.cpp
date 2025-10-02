
#include "Precomp.h"
#include "PackageWriter.h"
#include "PackageStream.h"
#include "NameString.h"
#include "PackageManager.h"
#include "UObject/UObject.h"
#include "UObject/UClass.h"
#include "Utils/File.h"

PackageWriter::PackageWriter(Package* package) : Source(package)
{
}

void PackageWriter::Save(UObject* packageObject, std::string filename)
{
	// Everything must be loaded
	Source->LoadAll();

	if (filename.empty())
		filename = Source->GetPackageFilename();

	std::string tempFilename = FilePath::remove_extension(filename) + ".tmp";

	try
	{
		GetNameIndex({}); // Add None as first index to name table

		auto stream = std::make_unique<PackageStreamWriter>(this, File::create_always(tempFilename));
		WriteHeader(stream.get());
		WriteObjects(packageObject, stream.get());
		WriteNameTable(stream.get());
		WriteExportTable(stream.get());
		WriteImportTable(stream.get());
		stream->Seek(0);
		WriteHeader(stream.get());
		stream.reset();

		// Close all package file handles
		Source->GetPackageManager()->CloseStreams();

		try
		{
			File::rename(filename, filename + ".old");
		}
		catch (...)
		{
			// No file to rename?
		}

		File::rename(tempFilename, filename);

		if (filename == Source->GetPackageFilename()) // To do: should we search all open packages?
		{
			Source->NameTable = std::move(NameTable);
			Source->NameHash = std::move(NameHash);
			Source->ImportTable = std::move(ImportTable);
			Source->ExportTable = std::move(ExportTable);
			Source->ExportObjects = std::move(ExportObjects);
		}
	}
	catch (...)
	{
		File::try_delete(tempFilename);
		throw;
	}
}

void PackageWriter::WriteHeader(PackageStreamWriter* stream)
{
	stream->WriteInt32(0x9E2A83C1);

	stream->WriteInt16(Source->Version);
	stream->WriteInt16(Source->LicenseeMode);

	stream->WriteUInt32((uint32_t)Source->Flags);

	stream->WriteInt32((int)NameTable.size());
	stream->WriteUInt32(NameOffset);

	stream->WriteInt32((int)ExportTable.size());
	stream->WriteUInt32(ExportOffset);

	stream->WriteInt32((int)ImportTable.size());
	stream->WriteUInt32(ImportOffset);

	if (Source->Version < 68)
	{
		stream->WriteInt32(0); // HeritageCount
		stream->WriteUInt32(0); // HeritageOffset
	}
	else
	{
		stream->WriteBytes(Source->Guid, 16);
		stream->WriteInt32(0); // GenerationCount
		/*for (uint32_t i = 0; i < GenerationCount; i++)
		{
			stream->WriteInt32(genExportCount);
			stream->WriteInt32(genNameCount);
		}*/
	}
}

void PackageWriter::WriteObjects(UObject* packageObject, PackageStreamWriter* stream)
{
	if (packageObject)
	{
		// Save a specific object and all its dependencies
		GetObjectReference(packageObject);
	}
	else
	{
		// Save all ObjectFlags::Standalone objects and their dependencies
		for (GCObject* gcobj : GC::GetObjects())
		{
			if (auto obj = dynamic_cast<UObject*>(gcobj))
			{
				if (obj->package == Source && AllFlags(obj->Flags, ObjectFlags::Standalone))
				{
					GetObjectReference(obj);
				}
			}
		}
	}

	size_t i = 0;
	while (i < ExportTable.size())
	{
		ExportTable[i].ObjOffset = stream->Tell();
		ExportObjects[i]->Save(stream);
		ExportTable[i].ObjSize = stream->Tell() - ExportTable[i].ObjOffset;
		i++;
	}
}

void PackageWriter::WriteNameTable(PackageStreamWriter* stream)
{
	NameOffset = stream->Tell();
	for (const NameTableEntry& entry : NameTable)
	{
		stream->WriteString(entry.Name.ToString());
		stream->WriteInt32(entry.Flags);
	}
}

void PackageWriter::WriteExportTable(PackageStreamWriter* stream)
{
	ExportOffset = stream->Tell();
	for (const ExportTableEntry& entry : ExportTable)
	{
		stream->WriteIndex(entry.ObjClass);
		stream->WriteIndex(entry.ObjBase);
		stream->WriteInt32(entry.ObjPackage);
		stream->WriteIndex(entry.ObjName);
		stream->WriteInt32((int)entry.ObjFlags);
		stream->WriteIndex(entry.ObjSize);
		if (entry.ObjSize > 0)
			stream->WriteIndex(entry.ObjOffset);
	}
}

void PackageWriter::WriteImportTable(PackageStreamWriter* stream)
{
	ImportOffset = stream->Tell();
	for (const ImportTableEntry& entry : ImportTable)
	{
		stream->WriteIndex(entry.ClassPackage);
		stream->WriteIndex(entry.ClassName);
		stream->WriteInt32(entry.ObjPackage);
		stream->WriteIndex(entry.ObjName);
	}
}

int PackageWriter::GetVersion() const
{
	return Source->Version;
}

int PackageWriter::GetNameIndex(NameString name)
{
	auto it = NameHash.find(name);
	if (it != NameHash.end())
		return it->second;

	int index = (int)NameTable.size();
	NameTableEntry entry;
	entry.Name = name;
	entry.Flags = 0;
	NameTable.push_back(entry);
	NameHash[name] = index;
	return index;
}

int PackageWriter::GetObjectReference(UObject* obj)
{
	if (obj == nullptr)
		return 0;

	if (obj != obj->Class)
		GetObjectReference(obj->Class);

	auto it = ObjRefHash.find(obj);
	if (it != ObjRefHash.end())
		return it->second;

	bool isClass = UObject::TryCast<UClass>(obj) != nullptr;
	if (obj->package == Source)
	{
		ExportTableEntry entry = {};
		if (isClass)
			entry.ObjBase = (obj != obj->Class) ? GetObjectReference(obj->Class) : 0;
		else
			entry.ObjClass = GetObjectReference(obj->Class);
		entry.ObjName = GetNameIndex(obj->Name);
		//entry.ObjPackage = GetObjectReference(obj->group);
		entry.ObjFlags = obj->Flags;

		ExportTable.push_back(entry);
		ExportObjects.push_back(obj);

		int ref = (int)ExportTable.size();
		ObjRefHash[obj] = ref;
		return ref;
	}
	else
	{
		// This stuff encodes some group info :(
		// see Package::GetUObject

		ImportTableEntry entry = {};
		entry.ObjName = GetNameIndex(obj->Name);
		entry.ClassName = GetNameIndex(obj->Class->Name);
		//entry.ObjPackage = GetObjectReference(obj->package);
		//entry.ClassPackage = GetObjectReference(obj->Class->package);
		ImportTable.push_back(entry);

		int ref = -(int)ImportTable.size();
		ObjRefHash[obj] = ref;
		return ref;
	}
}
