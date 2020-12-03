#pragma once

#include "PackageObject.h"

class Package;
class PackageManager;
class BinaryStream;
class ExportTableEntry;
class ImportTableEntry;

class Package
{
public:
	Package(PackageManager *packageManager, const std::string& name, const std::string &filename);

	ExportTableEntry* FindExportObject(int objectIndex);
	ExportTableEntry* FindExportObject(std::string className, std::string objectName, std::string groupName = {});

	int GetVersion() const { return PackageVersion; }

	std::string GetName(int index) { return NameTable[index]; }

private:
	void LoadHeader();

	std::unique_ptr<BinaryStream> OpenStream();

	ExportTableEntry* ExportForIndex(int index);
	ImportTableEntry* ImportForIndex(int index);

	int FindNameIndex(std::string name);

	PackageManager* Packages;
	std::string Name;
	std::string Filename;
	std::vector<uint8_t> Data;
	int PackageVersion = 0;
	uint32_t PackageFlags = 0;
	std::vector<std::string> NameTable;
	std::map<std::string, int> NameHash;
	std::vector<ExportTableEntry> ExportTable;
	std::vector<ImportTableEntry> ImportTable;

	friend class ExportTableEntry;
	friend class PackageObject;
};

class ImportTableEntry
{
public:
	int32_t ClassPackage;
	int32_t ClassName;
	int32_t ObjPackage;
	int32_t ObjName;
};

class ExportTableEntry
{
public:
	std::unique_ptr<PackageObject> Open();
	std::string GetClsName();

	Package* Owner;

	int32_t ObjClass;
	int32_t ObjBase;
	int32_t ObjPackage;
	int32_t ObjName;
	uint32_t ObjFlags;
	int32_t ObjSize;
	int32_t ObjOffset;
};
