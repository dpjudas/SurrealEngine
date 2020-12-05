#pragma once

#include "PackageFlags.h"
#include "ObjectFlags.h"
#include <functional>

class PackageManager;
class PackageStream;
class ObjectStream;
class NameTableEntry;
class ImportTableEntry;
class ExportTableEntry;
class UObject;

class Package
{
public:
	Package(PackageManager* packageManager, const std::string& name, const std::string& filename);
	~Package();

	UObject* GetUObject(int objref);
	UObject* GetUObject(const std::string& className, const std::string& objectName, const std::string& groupName = {});

	std::string GetName(int index);
	int GetVersion() const { return Version; }

private:
	void ReadTables();
	std::unique_ptr<PackageStream> OpenStream();
	std::unique_ptr<ObjectStream> OpenObjectStream(int index, std::string classname);
	void LoadExportObject(int index);
	ExportTableEntry* GetExportEntry(int objref);
	ImportTableEntry* GetImportEntry(int objref);
	int FindNameIndex(std::string name);
	int FindObjectReference(const std::string& className, const std::string& objectName, const std::string& groupName = {});

	template<typename T>
	void NewObject(int index, std::string classname)
	{
		// We need the pointer for the object before initializing it in order to support circular references.
		ObjectAllocations[index].reset(new uint64_t[sizeof(T)]);
		Objects[index] = (T*)ObjectAllocations[index].get();
		try
		{
			new(ObjectAllocations[index].get()) T(OpenObjectStream(index, std::move(classname)).get());
		}
		catch (...)
		{
			Objects[index] = nullptr;
			ObjectAllocations[index].reset();
			throw;
		}
	}

	template<typename T>
	void RegisterNativeClass(const std::string& name)
	{
		NativeClasses[GetNameKey(name)] = [](Package* package, int index, std::string classname) { package->NewObject<T>(index, std::move(classname)); };
	}

	static std::string GetNameKey(std::string name)
	{
		for (char& c : name)
		{
			if (c >= 'A' && c <= 'Z') c += 'a' - 'A';
		}
		return name;
	}

	PackageManager* Packages = nullptr;
	std::string Name;
	std::string Filename;

	int Version = 0;
	PackageFlags Flags = PackageFlags::None;
	std::vector<NameTableEntry> NameTable;
	std::vector<ExportTableEntry> ExportTable;
	std::vector<ImportTableEntry> ImportTable;

	std::map<std::string, int> NameHash;

	std::vector<std::unique_ptr<uint64_t[]>> ObjectAllocations;
	std::vector<UObject*> Objects;

	std::map<std::string, std::function<void(Package* package, int index, std::string classname)>> NativeClasses;

	Package(const Package&) = delete;
	Package& operator=(const Package&) = delete;
};

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

inline ObjectFlags operator|(ObjectFlags a, ObjectFlags b) { return (ObjectFlags)((uint32_t)a | (uint32_t)b); }
inline ObjectFlags operator&(ObjectFlags a, ObjectFlags b) { return (ObjectFlags)((uint32_t)a & (uint32_t)b); }
inline bool AllFlags(ObjectFlags value, ObjectFlags flags) { return (value & flags) == flags; }
inline bool AnyFlags(ObjectFlags value, ObjectFlags flags) { return (uint32_t)(value & flags) != 0; }
