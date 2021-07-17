#pragma once

#include "PackageFlags.h"
#include "ObjectFlags.h"
#include <functional>

class PackageManager;
class PackageStream;
class ObjectStream;
class UObject;
class UClass;

class NameTableEntry
{
public:
	std::string Name;
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
	Package(PackageManager* packageManager, const std::string& name, const std::string& filename);
	~Package();

	UObject* NewObject(const std::string& objname, UClass* objclass, ObjectFlags flags, bool initProperties);

	UObject* GetUObject(int objref);
	UObject* GetUObject(const std::string& className, const std::string& objectName, const std::string& groupName = {});

	const std::string& GetName(int index) const;
	int GetVersion() const { return Version; }
	std::string GetPackageName() const { return Name; }
	std::string GetPackageFilename() const { return Filename; }

	PackageManager* GetPackageManager() { return Packages; }

	ExportTableEntry* GetExportEntry(int objref);
	ImportTableEntry* GetImportEntry(int objref);
	int FindObjectReference(const std::string& className, const std::string& objectName, const std::string& groupName = {});

	std::vector<UClass*> GetAllClasses();

private:
	void ReadTables();
	std::unique_ptr<ObjectStream> OpenObjectStream(int index, const std::string& name, UClass* base);
	void LoadExportObject(int index);

	template<typename T>
	void RegisterNativeClass(bool registerInPackage, const std::string& className, const std::string& baseClass = {})
	{
		std::string classNameKey = GetNameKey(className);

		NativeClasses[classNameKey] = [](const std::string& name, UClass* cls, ObjectFlags flags) -> UObject*
		{
			return new T(name, cls, flags);
		};

		if (registerInPackage)
		{
			int objref = FindObjectReference("Class", className);
			if (objref == 0)
			{
				if (NameHash.find(classNameKey) == NameHash.end())
				{
					NameTableEntry nameentry;
					nameentry.Flags = 0;
					nameentry.Name = className;
					NameTable.push_back(nameentry);
					NameHash[classNameKey] = (int)NameTable.size() - 1;
				}

				ExportTableEntry entry;
				entry.ObjClass = 0;
				entry.ObjBase = baseClass.empty() ? 0 : FindObjectReference("Class", baseClass);
				entry.ObjPackage = 0;
				entry.ObjName = NameHash[classNameKey];
				entry.ObjFlags = ObjectFlags::Native;
				entry.ObjSize = 0;
				entry.ObjOffset = 0;
				ExportTable.push_back(entry);
			}
		}
	}

	static bool CompareNames(const std::string& name1, const std::string& name2)
	{
		if (name1.length() != name2.length())
			return false;

		size_t size = name1.length();
		for (size_t i = 0; i < size; i++)
		{
			if (ToLower(name1[i]) != ToLower(name2[i]))
				return false;
		}
		return true;
	}

	static char ToLower(char c)
	{
		return (c >= 'A' && c <= 'Z') ? c + 'a' - 'A' : c;
	}

	static std::string GetNameKey(std::string name)
	{
		for (char& c : name)
		{
			c = ToLower(c);
		}
		return name;
	}

	PackageManager* Packages = nullptr;
	std::string Name;
	std::string Filename;

	int Version = 0;
	PackageFlags Flags = PackageFlags::NoFlags;
	std::vector<NameTableEntry> NameTable;
	std::vector<ExportTableEntry> ExportTable;
	std::vector<ImportTableEntry> ImportTable;

	std::map<std::string, int> NameHash;

	std::vector<std::unique_ptr<UObject>> Objects;

	std::map<std::string, std::function<UObject*(const std::string& name, UClass* cls, ObjectFlags flags)>> NativeClasses;

	Package(const Package&) = delete;
	Package& operator=(const Package&) = delete;

	friend class PackageManager;
	friend class UObject;
};


inline ObjectFlags operator|(ObjectFlags a, ObjectFlags b) { return (ObjectFlags)((uint32_t)a | (uint32_t)b); }
inline ObjectFlags operator&(ObjectFlags a, ObjectFlags b) { return (ObjectFlags)((uint32_t)a & (uint32_t)b); }
inline bool AllFlags(ObjectFlags value, ObjectFlags flags) { return (value & flags) == flags; }
inline bool AnyFlags(ObjectFlags value, ObjectFlags flags) { return (uint32_t)(value & flags) != 0; }
