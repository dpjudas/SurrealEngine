#pragma once

#include "Package.h"

class NameString;
class UObject;
class PackageStreamWriter;

class PackageWriter
{
public:
	PackageWriter(Package* package);
	void Save();

private:
	void WriteHeader(PackageStreamWriter* stream);
	void WriteObjects(PackageStreamWriter* stream);
	void WriteNameTable(PackageStreamWriter* stream);
	void WriteExportTable(PackageStreamWriter* stream);
	void WriteImportTable(PackageStreamWriter* stream);

	int GetVersion() const;
	int GetNameIndex(NameString name);
	int GetObjectReference(UObject* obj);

	Package* Source = nullptr;
	Array<NameTableEntry> NameTable;
	Array<ExportTableEntry> ExportTable;
	Array<ImportTableEntry> ImportTable;

	uint32_t NameOffset = 0;
	uint32_t ExportOffset = 0;
	uint32_t ImportOffset = 0;

	std::map<NameString, int> NameHash;
	std::map<UObject*, int> ObjRefHash;

	friend class PackageStreamWriter;
};
