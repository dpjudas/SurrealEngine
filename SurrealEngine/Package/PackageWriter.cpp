
#include "Precomp.h"
#include "PackageWriter.h"
#include "PackageStream.h"
#include "NameString.h"
#include "PackageManager.h"
#include "Utils/File.h"

PackageWriter::PackageWriter(Package* package) : Source(package)
{

}

void PackageWriter::Save(std::string savePath = "")
{
	if (savePath.empty())
		savePath = Source->GetPackageFilename();

	std::string tempFilename = FilePath::remove_extension(savePath) + ".tmp";

	try
	{
		auto stream = std::make_unique<PackageStreamWriter>(this, File::create_always(tempFilename));
		WriteHeader(stream.get());
		WriteObjects(stream.get());
		WriteNameTable(stream.get());
		WriteExportTable(stream.get());
		WriteImportTable(stream.get());
		stream->Seek(0);
		WriteHeader(stream.get());
		stream.reset();

		try
		{
			File::rename(savePath, savePath + ".old");
		}
		catch (...)
		{
			// No file to rename?
		}

		File::rename(tempFilename, savePath);

		// TODO: Probably refresh the packages if saving is successful.
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

void PackageWriter::WriteObjects(PackageStreamWriter* stream)
{
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
	return 0;
}
