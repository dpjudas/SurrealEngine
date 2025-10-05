#pragma once

#include "Package.h"

class NameString;
class UObject;
class PackageStreamWriter;

class PackageWriter
{
public:
	PackageWriter(Package* package);
	void Save(UObject* packageObject, std::string filename = {});

private:
	void WriteHeader(PackageStreamWriter* stream);
	void WriteObjects(UObject* packageObject, PackageStreamWriter* stream);
	void WriteNameTable(PackageStreamWriter* stream);
	void WriteExportTable(PackageStreamWriter* stream);
	void WriteImportTable(PackageStreamWriter* stream);

	int GetVersion() const;
	int GetNameIndex(NameString name);
	int GetObjectReference(UObject* obj);
	int GetPackageReference(NameString packageName);

	Package* Source = nullptr;
	Array<NameTableEntry> NameTable;
	Array<ExportTableEntry> ExportTable;
	Array<ImportTableEntry> ImportTable;

	Array<UObject*> ExportObjects;

	uint32_t NameOffset = 0;
	uint32_t ExportOffset = 0;
	uint32_t ImportOffset = 0;

	std::map<NameString, int> NameHash;
	std::map<UObject*, int> ObjRefHash;
	std::map<NameString, int> PackageReferences;

	friend class PackageStreamWriter;
};
