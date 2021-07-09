
#include "Precomp.h"
#include "PackageManager.h"
#include "Package.h"
#include "PackageStream.h"
#include "IniFile.h"
#include "File.h"
#include "UObject/UObject.h"
#include "UObject/UClass.h"
#include "Native/NActor.h"
#include "Native/NCanvas.h"
#include "Native/NCommandlet.h"
#include "Native/NConsole.h"
#include "Native/NDecal.h"
#include "Native/NGameInfo.h"
#include "Native/NLevelInfo.h"
#include "Native/NNavigationPoint.h"
#include "Native/NObject.h"
#include "Native/NPawn.h"
#include "Native/NPlayerPawn.h"
#include "Native/NScriptedTexture.h"
#include "Native/NStatLog.h"
#include "Native/NStatLogFile.h"
#include "Native/NWarpZoneInfo.h"
#include "Native/NZoneInfo.h"

PackageManager::PackageManager(const std::string& basepath) : basepath(basepath)
{
	NActor::RegisterFunctions();
	NCanvas::RegisterFunctions();
	NCommandlet::RegisterFunctions();
	NConsole::RegisterFunctions();
	NDecal::RegisterFunctions();
	NGameInfo::RegisterFunctions();
	NLevelInfo::RegisterFunctions();
	NNavigationPoint::RegisterFunctions();
	NObject::RegisterFunctions();
	NPawn::RegisterFunctions();
	NPlayerPawn::RegisterFunctions();
	NScriptedTexture::RegisterFunctions();
	NStatLog::RegisterFunctions();
	NStatLogFile::RegisterFunctions();
	NWarpZoneInfo::RegisterFunctions();
	NZoneInfo::RegisterFunctions();

	ScanFolder("Maps", "*.unr");
	ScanFolder("Music", "*.umx");
	ScanFolder("Sounds", "*.uax");
	ScanFolder("System", "*.u");
	ScanFolder("Textures", "*.utx");

	InitPropertyOffsets(this);

	// File::write_all_text("C:\\Development\\UTNativeProps.txt", NativeObjExtractor::Run(this));
	// File::write_all_text("C:\\Development\\UTNativeFuncs.txt", NativeFuncExtractor::Run(this));
}

Package* PackageManager::GetPackage(const std::string& name)
{
	std::string key = GetKey(name);

	auto& package = packages[key];
	if (package)
		return package.get();
	
	auto it = packageFilenames.find(key);
	if (it != packageFilenames.end())
	{
		package = std::make_unique<Package>(this, key, it->second);
	}
	else
	{
		throw std::runtime_error("Could not find package " + name);
	}

	return package.get();
}

void PackageManager::ScanFolder(const std::string& name, const std::string& search)
{
	std::string packagedir = FilePath::combine(basepath, name);
	for (std::string filename : Directory::files(FilePath::combine(packagedir, search)))
	{
		packageFilenames[GetKey(FilePath::remove_extension(filename))] = FilePath::combine(packagedir, filename);
	}
}

std::string PackageManager::GetKey(std::string name)
{
	for (char& c : name)
	{
		if (c >= 'A' && c <= 'Z')
		{
			c = c - 'A' + 'a';
		}
	}
	return name;
}

std::vector<std::string> PackageManager::GetPackageNames() const
{
	std::vector<std::string> names;
	for (auto& it : packageFilenames)
	{
		names.push_back(it.first);
	}
	return names;
}

std::shared_ptr<PackageStream> PackageManager::GetStream(Package* package)
{
	int numStreams = 0;
	for (auto it = openStreams.begin(); it != openStreams.end(); ++it)
	{
		if ((*it).Pkg == package)
		{
			if (it != openStreams.begin())
			{
				OpenStream s = *it;
				openStreams.erase(it);
				openStreams.push_front(s);
			}
			return openStreams.front().Stream;
		}
		numStreams++;
	}

	OpenStream s;
	s.Pkg = package;
	s.Stream = std::make_shared<PackageStream>(package, File::open_existing(package->GetPackageFilename()));
	openStreams.push_front(s);

	if (numStreams == 10)
		openStreams.pop_back();

	return openStreams.front().Stream;
}

void PackageManager::DelayLoadNow()
{
	while (!delayLoads.empty())
	{
		SetDelayLoadActive delayload(this);

		UObject* obj = delayLoads.back();
		delayLoads.pop_back();
		obj->LoadNow();
	}
}

UObject* PackageManager::NewObject(const std::string& name, const std::string& package, const std::string& className)
{
	Package* pkg = GetPackage(package);
	UClass* cls = UObject::Cast<UClass>(pkg->GetUObject("Class", className));
	if (!cls)
		throw std::runtime_error("Could not find class " + className);
	return pkg->NewObject(name, cls, ObjectFlags::None, true);
}

std::string PackageManager::GetIniValue(std::string iniName, const std::string& sectionName, const std::string& keyName)
{
	for (char& c : iniName)
	{
		if (c >= 'A' && c <= 'Z')
			c += 'a' - 'A';
	}

	if (iniName == "system")
		iniName = "unrealtournament";

	auto& ini = iniFiles[iniName];
	if (!ini)
	{
		ini = std::make_unique<IniFile>(FilePath::combine(basepath, "System/" + iniName + ".ini"));
	}

	return ini->GetValue(sectionName, keyName);
}
