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
	Package(PackageManager* packageManager, const NameString& name, const std::string& filename);
	~Package();

	UObject* NewObject(const NameString& objname, UClass* objclass, ObjectFlags flags, bool initProperties);

	UObject* GetUObject(int objref);
	UObject* GetUObject(const NameString& className, const NameString& objectName, const NameString& groupName = {});

	void LoadAll();
	void Save(UObject* object = nullptr, const std::string& filename = {});

	const NameString& GetName(int index) const;
	int GetVersion() const { return Version; }
	NameString GetPackageName() const { return Name; }
	std::string GetPackageFilename() const { return Filename; }

	PackageManager* GetPackageManager() { return Packages; }

	ExportTableEntry* GetExportEntry(int objref);
	ImportTableEntry* GetImportEntry(int objref);
	int FindObjectReference(const NameString& className, const NameString& objectName, const NameString& groupName = {});

	std::string GetExportName(int objref);

	template<class T> Array<T*> GetAllObjects();

private:
	GCAllocation* Mark(GCAllocation* marklist) override;

	void ReadTables();
	std::unique_ptr<ObjectStream> OpenObjectStream(int index, const NameString& name, UClass* base);
	void LoadExportObject(int index);

	template<typename T>
	void RegisterNativeClass(bool registerInPackage, const NameString& className, const NameString& baseClass = {});

	PackageManager* Packages = nullptr;
	NameString Name;
	std::string Filename;

	int Version = 0;
	int LicenseeMode = 0;
	PackageFlags Flags = PackageFlags::NoFlags;
	Array<NameTableEntry> NameTable;
	Array<ExportTableEntry> ExportTable;
	Array<ImportTableEntry> ImportTable;
	uint8_t Guid[16] = {};

	std::map<NameString, int> NameHash;

	Array<UObject*> ExportObjects;

	std::map<NameString, std::function<UObject*(const NameString& name, UClass* cls, ObjectFlags flags)>> NativeClasses;

	Package(const Package&) = delete;
	Package& operator=(const Package&) = delete;

	friend class PackageManager;
	friend class UObject;
	friend class PackageWriter;
};

template<typename T>
void Package::RegisterNativeClass(bool registerInPackage, const NameString& className, const NameString& baseClass)
{
	NativeClasses[className] = [](const NameString& name, UClass* cls, ObjectFlags flags) -> UObject*
		{
			return GC::Alloc<T>(name, cls, flags);
		};

	if (registerInPackage)
	{
		int objref = FindObjectReference("Class", className);
		if (objref == 0)
		{
			if (NameHash.find(className) == NameHash.end())
			{
				NameTableEntry nameentry;
				nameentry.Flags = 0;
				nameentry.Name = className;
				NameTable.push_back(nameentry);
				NameHash[className] = (int)NameTable.size() - 1;
			}

			ExportTableEntry entry;
			entry.ObjClass = 0;
			entry.ObjBase = baseClass.IsNone() ? 0 : FindObjectReference("Class", baseClass);
			entry.ObjPackage = 0;
			entry.ObjName = NameHash[className];
			entry.ObjFlags = ObjectFlags::Native;
			entry.ObjSize = 0;
			entry.ObjOffset = 0;
			ExportTable.push_back(entry);
		}
	}
}
