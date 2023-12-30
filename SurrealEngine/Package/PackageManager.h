#pragma once

#include "Package.h"
#include "IniFile.h"
#include "GameFolder.h"
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
	PackageManager(const GameLaunchInfo& launchInfo);

	bool IsUnreal1() const { return launchInfo.gameName == "Unreal"; }
	bool IsUnreal1_226() const { return IsUnreal1() && launchInfo.engineVersion == 226; }
	bool IsUnreal1_227() const { return IsUnreal1() && launchInfo.engineVersion == 227; }
	bool IsUnrealTournament() const { return launchInfo.gameName == "UnrealTournament"; }
	bool IsUnrealTournament_469() const { return IsUnrealTournament() && launchInfo.engineVersion == 469; }
	bool IsDeusEx() const { return launchInfo.gameName == "DeusEx"; }

	int GetEngineVersion() const { return launchInfo.engineVersion; }
	int GetEngineSubVersion() const { return launchInfo.engineSubVersion; }

	Package *GetPackage(const NameString& name);
	std::vector<NameString> GetPackageNames() const;

	void UnloadPackage(const NameString& name);

	std::shared_ptr<PackageStream> GetStream(Package* package);

	UObject* NewObject(const NameString& name, const NameString& package, const NameString& className);
	UObject* NewObject(const NameString& name, UClass* cls);

	UClass* FindClass(const NameString& name);

	std::vector<NameString> GetIniKeysFromSection(NameString iniName, const NameString& sectionName);
	std::string GetIniValue(NameString iniName, const NameString& sectionName, const NameString& keyName);
	std::vector<std::string> GetIniValues(NameString iniName, const NameString& sectionName, const NameString& keyName);
	std::string Localize(NameString packageName, const NameString& sectionName, const NameString& keyName);

	std::vector<IntObject>& GetIntObjects(const NameString& metaclass);
	const std::vector<std::string>& GetMaps() const { return maps; }

private:
	void LoadIntFiles();
	void LoadPackageRemaps();
	std::map<NameString, std::string> ParseIntPublicValue(const std::string& value);

	void ScanForMaps();

	void ScanFolder(const std::string& packagedir, const std::string& search);
	void ScanPaths();

	void DelayLoadNow();

	std::vector<UObject*> delayLoads;
	int delayLoadActive = 0;

	std::map<NameString, std::string> packageFilenames;
	std::map<NameString, std::unique_ptr<Package>> packages;
	std::map<NameString, std::unique_ptr<IniFile>> iniFiles;
	std::map<NameString, std::unique_ptr<IniFile>> intFiles;
	std::map<std::string, std::string> packageRemaps;

	std::map<NameString, std::vector<IntObject>> IntObjects;

	std::vector<std::string> maps;

	struct OpenStream
	{
		Package* Pkg = nullptr;
		std::shared_ptr<PackageStream> Stream;
	};

	std::list<OpenStream> openStreams;

	GameLaunchInfo launchInfo;

	friend class Package;
	friend struct SetDelayLoadActive;
};

struct SetDelayLoadActive
{
	SetDelayLoadActive(PackageManager* p) : p(p) { p->delayLoadActive++; }
	~SetDelayLoadActive() { p->delayLoadActive--; }
	PackageManager* p;
};
