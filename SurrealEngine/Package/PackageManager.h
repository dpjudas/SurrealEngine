#pragma once

#include "GC/GC.h"
#include "Package.h"
#include "IniFile.h"
#include "GameFolder.h"
#include <list>
#include <filesystem>

namespace fs = std::filesystem;

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

struct NativeClass
{
	NameString Package;
	NameString Name;
	NameString Base;
	std::function<UObject* (const NameString& name, UClass* cls, ObjectFlags flags)> CreateFunc;
};

class PackageManager
{
public:
	PackageManager(const GameLaunchInfo& launchInfo);

	bool IsUnreal1() const { return launchInfo.IsUnreal1(); }
	bool IsUnreal1_226() const { return launchInfo.IsUnreal1_226(); }
	bool IsUnreal1_227() const { return launchInfo.IsUnreal1_227(); }
	bool IsUnrealTournament() const { return launchInfo.IsUnrealTournament(); }
	bool IsUnrealTournament_469() const { return launchInfo.IsUnrealTournament_469(); }
	bool IsDeusEx() const { return launchInfo.IsDeusEx(); }
	bool IsCliveBarkersUndying() const { return launchInfo.IsCliveBarkersUndying(); }
	bool IsKlingonHonorGuard() const  { return launchInfo.IsKlingonHonorGuard(); }

	int GetEngineVersion() const { return launchInfo.engineVersion; }
	int GetEngineSubVersion() const { return launchInfo.engineSubVersion; }

	fs::path GetRootFolderPath() const { return gameRootFolderPath; }
	fs::path GetSystemFolderPath() const { return gameSystemFolderPath; }

	Package *GetPackage(const NameString& name);
	Array<NameString> GetPackageNames() const;

	Package* LoadMap(const std::string& path);
	void UnloadMap(Package* package);

	void CloseStreams();

	Package* LoadSaveFile(const std::string& path);
	Package* LoadSaveSlot(const uint32_t slotNum);

	std::shared_ptr<PackageStream> GetStream(Package* package);

	UClass* FindClass(const NameString& name);

	std::string GetMapExtension() const { return mapExtension; }
	std::string GetSaveExtension() const { return saveExtension; }

	std::unique_ptr<IniFile> GetIniFile(NameString iniName);
	std::unique_ptr<IniFile> GetUserIniFile();
	std::unique_ptr<IniFile> GetSystemIniFile();
	Array<NameString> GetIniKeysFromSection(NameString iniName, const NameString& sectionName);
	std::string GetIniValue(NameString iniName, const NameString& sectionName, const NameString& keyName, std::string default_value = "", const int index = 0);
	Array<std::string> GetIniValues(NameString iniName, const NameString& sectionName, const NameString& keyName, Array<std::string> default_values = {});
	std::string GetDefaultIniValue(const NameString& sectionName, const NameString& keyName, std::string default_value = "", const int index = 0);
	Array<std::string> GetDefaultIniValues(const NameString& sectionName, const NameString& keyName, Array<std::string> default_values = {});
	std::string GetDefUserIniValue(const NameString& sectionName, const NameString& keyName, std::string default_value = "", const int index = 0);
	Array<std::string> GetDefUserIniValues(const NameString& sectionName, const NameString& keyName, Array<std::string> default_values = {});
	void SetIniValue(NameString iniName, const NameString& sectionName, const NameString& keyName, const std::string& newValue, const int index = 0);
	void SetIniValues(NameString iniName, const NameString& sectionName, const NameString& keyName, const Array<std::string>& newValues);
	void SaveAllIniFiles();

	std::string GetVideoFilename(const std::string& name);

	std::string Localize(NameString packageName, const NameString& sectionName, const NameString& keyName, const int index = 0);

	Array<IntObject>& GetIntObjects(const NameString& metaclass);
	const Array<std::string>& GetMaps() const { return maps; }

	bool MissingSESystemIni() const { return missing_se_system_ini; }

	Package* GetTransientPackage() { return GetPackage("Transient"); }

private:
	void CreateTransientPackage();

	std::unique_ptr<IniFile>& LoadIniFile(NameString iniName);
	std::unique_ptr<IniFile>& LoadUserIniFile();
	std::unique_ptr<IniFile>& LoadSystemIniFile();
	void LoadEngineIniFiles();
	void LoadIntFiles();
	void LoadPackageRemaps();
	std::map<NameString, std::string> ParseIntPublicValue(const std::string& value);

	void ScanForMaps();

	void ScanFolder(const std::string& packagedir, const std::string& search);
	void ScanPaths();

	void DelayLoadNow();
	void RegisterFunctions();

	void RegisterNativeClasses();

	template<typename T>
	void RegisterNativeClass(const NameString& packageName, const NameString& className, const NameString& baseClass = {});

	std::map<NameString, NativeClass> NativeClasses;

	Array<UObject*> delayLoads;
	int delayLoadActive = 0;

	std::map<NameString, std::string> packageFilenames;
	std::map<NameString, GCRoot<Package>> packages;
	std::map<NameString, std::unique_ptr<IniFile>> iniFiles;
	std::map<NameString, std::unique_ptr<IniFile>> intFiles;
	std::map<std::string, std::string> packageRemaps;

	std::map<NameString, std::string> aviFilenames;

	std::map<NameString, Array<IntObject>> IntObjects;

	std::unique_ptr<IniFile> defaultIniFile;  // Holds Default.ini
	std::unique_ptr<IniFile> defaultUserFile; // Holds DefUser.ini

	Array<std::string> mapFolders;
	Array<std::string> maps;

	// So that we don't have to build fs::path()s all over the place
	fs::path gameRootFolderPath;
	fs::path gameSystemFolderPath;

	std::string mapExtension;
	std::string saveExtension;

	bool missing_se_system_ini = false;

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
