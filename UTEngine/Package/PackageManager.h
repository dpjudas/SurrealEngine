#pragma once

#include "Package.h"
#include "IniFile.h"
#include <list>

class PackageStream;
class UObject;
class UClass;

struct IntObject
{
	std::string Name;
	std::string Class;
	std::string MetaClass;
	std::string Description;
};

class PackageManager
{
public:
	PackageManager(const std::string& basepath, int engineVersion);

	bool IsUnreal1() const { return unreal1; }
	int GetEngineVersion() const { return engineVersion; }

	Package *GetPackage(const std::string& name);
	std::vector<std::string> GetPackageNames() const;

	std::shared_ptr<PackageStream> GetStream(Package* package);

	UObject* NewObject(const std::string& name, const std::string& package, const std::string& className);
	UObject* NewObject(const std::string& name, UClass* cls);

	UClass* FindClass(const std::string& name);

	std::string GetIniValue(std::string iniName, const std::string& sectionName, const std::string& keyName);
	std::string Localize(std::string packageName, const std::string& sectionName, const std::string& keyName);

	std::vector<IntObject>& GetIntObjects(const std::string& metaclass);
	const std::vector<std::string>& GetMaps() const { return maps; }

private:
	void LoadIntFiles();
	std::map<std::string, std::string> ParseIntPublicValue(const std::string& value);

	void ScanForMaps();

	void ScanFolder(const std::string& name, const std::string& search);
	std::string GetKey(std::string name);

	void DelayLoadNow();

	std::vector<UObject*> delayLoads;
	int delayLoadActive = 0;

	std::string basepath;
	std::map<std::string, std::string> packageFilenames;
	std::map<std::string, std::unique_ptr<Package>> packages;
	std::map<std::string, std::unique_ptr<IniFile>> iniFiles;
	std::map<std::string, std::unique_ptr<IniFile>> intFiles;

	std::map<std::string, std::vector<IntObject>> IntObjects;

	std::vector<std::string> maps;

	struct OpenStream
	{
		Package* Pkg = nullptr;
		std::shared_ptr<PackageStream> Stream;
	};

	std::list<OpenStream> openStreams;

	bool unreal1 = false;
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
