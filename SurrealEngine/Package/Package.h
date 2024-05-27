#pragma once

#include "PackageFlags.h"
#include "ObjectFlags.h"
#include "NameString.h"
#include <functional>

class PackageManager;
class PackageStream;
class ObjectStream;
class UObject;
class UClass;

class NameTableEntry
{
public:
	NameString Name;
	uint32_t Flags;
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
	int32_t ObjClass;
	int32_t ObjBase;
	int32_t ObjPackage;
	int32_t ObjName;
	ObjectFlags ObjFlags;
	int32_t ObjSize;
	int32_t ObjOffset;
};

class Package
{
public:
	Package(PackageManager* packageManager, const NameString& name, const std::string& filename);
	~Package();

	UObject* NewObject(const NameString& objname, UClass* objclass, ObjectFlags flags, bool initProperties);

	UObject* GetUObject(int objref);
	UObject* GetUObject(const NameString& className, const NameString& objectName, const NameString& groupName = {});

	const NameString& GetName(int index) const;
	int GetVersion() const { return Version; }
	NameString GetPackageName() const { return Name; }
	std::string GetPackageFilename() const { return Filename; }

	PackageManager* GetPackageManager() { return Packages; }

	ExportTableEntry* GetExportEntry(int objref);
	ImportTableEntry* GetImportEntry(int objref);
	int FindObjectReference(const NameString& className, const NameString& objectName, const NameString& groupName = {});

	std::string GetExportName(int objref);

	template<class T> std::vector<T*> GetAllObjects()
	{
		std::vector<T*> objects;
		int objref = 1;
		for (ExportTableEntry& e : ExportTable)
		{
			std::string className;
			if (e.ObjClass < 0)
			{
				className = GetName(GetImportEntry(e.ObjClass)->ObjName).ToString();
			}
			else if (e.ObjClass != 0)
			{
				className = GetName(GetExportEntry(e.ObjClass)->ObjName).ToString();
			}

			// ignore "Groups", they're not real objects
			if (className.compare("Package") != 0)
			{
				T* obj = dynamic_cast<T*>(GetUObject(objref));
				if (obj)
				{
					objects.push_back(obj);
				}
			}
			objref++;
		}
		return objects;
	}

private:
	void ReadTables();
	std::unique_ptr<ObjectStream> OpenObjectStream(int index, const NameString& name, UClass* base);
	void LoadExportObject(int index);

	template<typename T>
	void RegisterNativeClass(bool registerInPackage, const NameString& className, const NameString& baseClass = {})
	{
		NativeClasses[className] = [](const NameString& name, UClass* cls, ObjectFlags flags) -> UObject*
		{
			return new T(name, cls, flags);
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

	PackageManager* Packages = nullptr;
	NameString Name;
	std::string Filename;

	int Version = 0;
	PackageFlags Flags = PackageFlags::NoFlags;
	std::vector<NameTableEntry> NameTable;
	std::vector<ExportTableEntry> ExportTable;
	std::vector<ImportTableEntry> ImportTable;

	std::map<NameString, int> NameHash;

	std::vector<std::unique_ptr<UObject>> Objects;

	std::map<NameString, std::function<UObject*(const NameString& name, UClass* cls, ObjectFlags flags)>> NativeClasses;

	Package(const Package&) = delete;
	Package& operator=(const Package&) = delete;

	friend class PackageManager;
	friend class UObject;
};


inline ObjectFlags operator|(ObjectFlags a, ObjectFlags b) { return (ObjectFlags)((uint32_t)a | (uint32_t)b); }
inline ObjectFlags operator|=(ObjectFlags& a, ObjectFlags b) { a = (ObjectFlags)((uint32_t)a | (uint32_t)b); return a; }
inline ObjectFlags operator&(ObjectFlags a, ObjectFlags b) { return (ObjectFlags)((uint32_t)a & (uint32_t)b); }
inline bool AllFlags(ObjectFlags value, ObjectFlags flags) { return (value & flags) == flags; }
inline bool AnyFlags(ObjectFlags value, ObjectFlags flags) { return (uint32_t)(value & flags) != 0; }
