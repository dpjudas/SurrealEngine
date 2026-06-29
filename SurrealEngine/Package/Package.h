#pragma once

#include "GC/GC.h"
#include "PackageFlags.h"
#include "PackageTables.h"
#include "ObjectFlags.h"
#include "NameString.h"
#include <functional>

class PackageManager;
class PackageStream;
class ObjectStream;
class UObject;
class UClass;

class Package : public GCObject
{
public:
	Package(PackageManager* packageManager, const NameString& name, const std::string& filepath);
	~Package();

	UObject* NewObject(const NameString& objname, UClass* objclass, ObjectFlags flags, bool initProperties = true);

	UObject* GetUObject(int objref);
	UObject* GetUObject(const NameString& className, const NameString& objectName) { return GetUObject(className, objectName, {}, true); }
	UObject* GetUObject(const NameString& className, const NameString& objectName, const NameString& group, bool ignoreGroup = false);

	UClass* GetClass(const NameString& className);

	void LoadAll();
	void Save(UObject* object = nullptr, const std::string& filename = {});

	const NameString& GetName(int index) const;
	int GetVersion() const { return Version; }
	NameString GetPackageName() const { return Name; }
	std::string GetPackageFileName() const { return FileName; }
	std::string GetPackageFilePath() const { return FilePath; }
	std::string GetPackageFileExtension() const { return FileExtension; }

	PackageManager* GetPackageManager() { return Packages; }

	ExportTableEntry* GetExportEntry(int objref);
	ImportTableEntry* GetImportEntry(int objref);
	int FindObjectReference(const NameString& className, const NameString& objectName, const NameString& group, bool ignoreGroup = false);

	std::string GetExportName(int objref);

	template<class T> Array<T*> GetAllObjects();

private:
	GCAllocation* Mark(GCAllocation* marklist) override;

	void ReadTables();
	std::unique_ptr<ObjectStream> OpenObjectStream(int index, const NameString& name, UClass* base);
	void LoadExportObject(int index);

	PackageManager* Packages = nullptr;
	NameString Name;
	std::string FilePath;
	std::string FileName;
	std::string FileExtension;

	int Version = 0;
	int LicenseeMode = 0;
	PackageFlags Flags = PackageFlags::NoFlags;
	Array<NameTableEntry> NameTable;
	Array<ExportTableEntry> ExportTable;
	Array<ImportTableEntry> ImportTable;
	uint8_t Guid[16] = {};

	std::map<NameString, int> NameHash;

	Array<UObject*> ExportObjects;

	Package(const Package&) = delete;
	Package& operator=(const Package&) = delete;

	friend class PackageManager;
	friend class UObject;
	friend class PackageWriter;
};
