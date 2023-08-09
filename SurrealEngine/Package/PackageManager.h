#pragma once

#include "Package.h"
#include "IniFile.h"
#include <list>

class PackageStream;
class UObject;
class UClass;

struct IntObject
{
	NameString Name;
	NameString Class;
	NameString MetaClass;
	std::string Description;
};

class PackageManager
{
public:
	PackageManager(const std::string& basepath, int engineVersion, const std::string& gameName);

	bool IsUnreal1() const { return engineVersion < 300; }
	int GetEngineVersion() const { return engineVersion; }

	Package *GetPackage(const NameString& name);
	std::vector<NameString> GetPackageNames() const;

	void UnloadPackage(const NameString& name);

	std::shared_ptr<PackageStream> GetStream(Package* package);

	UObject* NewObject(const NameString& name, const NameString& package, const NameString& className);
	UObject* NewObject(const NameString& name, UClass* cls);

	UClass* FindClass(const NameString& name);

	std::string GetIniValue(NameString iniName, const NameString& sectionName, const NameString& keyName);
	std::string Localize(NameString packageName, const NameString& sectionName, const NameString& keyName);

	std::vector<IntObject>& GetIntObjects(const NameString& metaclass);
	const std::vector<std::string>& GetMaps() const { return maps; }

private:
	void LoadIntFiles();
	std::map<NameString, std::string> ParseIntPublicValue(const std::string& value);

	void ScanForMaps();

	void ScanFolder(const std::string& name, const std::string& search);

	void DelayLoadNow();

	std::vector<UObject*> delayLoads;
	int delayLoadActive = 0;

	std::string basepath;
	std::map<NameString, std::string> packageFilenames;
	std::map<NameString, std::unique_ptr<Package>> packages;
	std::map<NameString, std::unique_ptr<IniFile>> iniFiles;
	std::map<NameString, std::unique_ptr<IniFile>> intFiles;

	std::map<NameString, std::vector<IntObject>> IntObjects;

	std::vector<std::string> maps;

	struct OpenStream
	{
		Package* Pkg = nullptr;
		std::shared_ptr<PackageStream> Stream;
	};

	std::list<OpenStream> openStreams;

	std::string gameName;
	int engineVersion = 436;

	friend class Package;
	friend struct SetDelayLoadActive;
};

struct SetDelayLoadActive
{
	SetDelayLoadActive(PackageManager* p) : p(p) { p->delayLoadActive++; }
	~SetDelayLoadActive() { p->delayLoadActive--; }
	PackageManager* p;
};
