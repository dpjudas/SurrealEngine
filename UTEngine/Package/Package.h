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
class UClass;

class Package
{
public:
	Package(PackageManager* packageManager, const std::string& name, const std::string& filename);
	~Package();

	UObject* GetUObject(int objref, std::function<void(UObject*)> validateTypeCast = {});
	UObject* GetUObject(const std::string& className, const std::string& objectName, const std::string& groupName = {}, std::function<void(UObject*)> validateTypeCast = {});

	const std::string& GetName(int index) const;
	int GetVersion() const { return Version; }
	std::string GetPackageName() const { return Name; }

	ExportTableEntry* GetExportEntry(int objref);
	ImportTableEntry* GetImportEntry(int objref);
	int FindObjectReference(const std::string& className, const std::string& objectName, const std::string& groupName = {});

	std::vector<UClass*> GetAllClasses();

private:
	void ReadTables();
	std::unique_ptr<PackageStream> OpenStream();
	std::unique_ptr<ObjectStream> OpenObjectStream(int index, const std::string& name, UClass* base);
	void LoadExportObject(int index);

	void DelayLoadNow();

	std::vector<std::function<void()>> delayLoads;
	std::vector<std::function<void()>> delayLoadTypeValidations;
	int delayLoadActive = 0;

	struct SetDelayLoadActive
	{
		SetDelayLoadActive(Package* p) : p(p) { p->delayLoadActive++; }
		~SetDelayLoadActive() { p->delayLoadActive--; }
		Package* p;
	};

	template<typename T>
	void NewObject(int index, std::string name, UClass* base)
	{
		// We need the pointer for the object before initializing it in order to support circular references.
		ObjectAllocations[index].reset(new uint64_t[sizeof(T)]);
		void* ptr = ObjectAllocations[index].get();
		Objects[index] = (T*)ptr;

		if (typeid(T) != typeid(UClass))
			delayLoads.push_back([=]() { new(ptr) T(OpenObjectStream(index, name, base).get()); });
		else
			new(ptr) T(OpenObjectStream(index, name, base).get());
	}

	template<typename T>
	void RegisterNativeClass(const std::string& name)
	{
		NativeClasses[GetNameKey(name)] = [](Package* package, int index, const std::string& name, UClass* base) { package->NewObject<T>(index, name, base); };
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
	PackageFlags Flags = PackageFlags::None;
	std::vector<NameTableEntry> NameTable;
	std::vector<ExportTableEntry> ExportTable;
	std::vector<ImportTableEntry> ImportTable;

	std::map<std::string, int> NameHash;

	std::vector<std::unique_ptr<uint64_t[]>> ObjectAllocations;
	std::vector<UObject*> Objects;

	std::map<std::string, std::function<void(Package* package, int index, const std::string& name, UClass* base)>> NativeClasses;

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
