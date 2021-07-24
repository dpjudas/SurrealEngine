
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

	try
	{
		File::open_existing(FilePath::combine(basepath, "System/Unreal.ini"));
		unreal1 = true;
	}
	catch (...)
	{
		try
		{
			File::open_existing(FilePath::combine(basepath, "System/UnrealTournament.ini"));
		}
		catch (...)
		{
			throw std::runtime_error("Could not find Unreal Tournament or Unreal at " + basepath);
		}
	}

	LoadIntFiles();
	ScanForMaps();

	ScanFolder("Maps", "*.unr");
	if (IsUnreal1())
		ScanFolder("Maps/UPak", "*.unr");
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

void PackageManager::ScanForMaps()
{
	std::string packagedir = FilePath::combine(basepath, "Maps");
	for (std::string filename : Directory::files(FilePath::combine(packagedir, "*.unr")))
	{
		maps.push_back(filename);
	}
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
	return pkg->NewObject(name, cls, ObjectFlags::NoFlags, true);
}

std::string PackageManager::GetIniValue(std::string iniName, const std::string& sectionName, const std::string& keyName)
{
	/*
	for (char& c : iniName)
	{
		if (c >= 'A' && c <= 'Z')
			c += 'a' - 'A';
	}
	*/

	if (iniName == "system" || iniName == "System")
		iniName = unreal1 ? "Unreal" : "UnrealTournament";
	else if (iniName == "user")
		iniName = "User";

	auto& ini = iniFiles[iniName];
	if (!ini)
	{
		ini = std::make_unique<IniFile>(FilePath::combine(basepath, "System/" + iniName + ".ini"));
	}

	return ini->GetValue(sectionName, keyName);
}

void PackageManager::LoadIntFiles()
{
	std::string systemdir = FilePath::combine(basepath, "System");
	for (std::string filename : Directory::files(FilePath::combine(systemdir, "*.int")))
	{
		try
		{
			auto intFile = std::make_unique<IniFile>(FilePath::combine(systemdir, filename));

			for (const std::string& value : intFile->GetValues("Public", "Object"))
			{
				auto desc = ParseIntPublicValue(value);
				if (!desc["Name"].empty() && !desc["Class"].empty() && !desc["MetaClass"].empty()) // Used by Actor.GetInt
				{
					IntObject obj;
					obj.Name = desc["Name"];
					obj.Class = desc["Class"];
					obj.MetaClass = desc["MetaClass"];
					obj.Description = desc["Description"];

					std::string metaClass = obj.MetaClass;

					size_t pos = metaClass.find_last_of('.');
					if (pos != std::string::npos)
						metaClass = metaClass.substr(pos + 1);

					IntObjects[metaClass].push_back(std::move(obj));
				}
				else if (!desc["Name"].empty() && !desc["Class"].empty()) // Used by Actor.GetNextSkin
				{
					IntObject obj;
					obj.Name = desc["Name"];
					obj.Class = desc["Class"];
					obj.Description = desc["Description"];

					std::string cls = obj.Class;

					size_t pos = cls.find_last_of('.');
					if (pos != std::string::npos)
						cls = cls.substr(pos + 1);

					IntObjects[cls].push_back(std::move(obj));
				}
			}

			intFiles[FilePath::remove_extension(filename)] = std::move(intFile);
		}
		catch (...)
		{
		}
	}
}

std::vector<IntObject>& PackageManager::GetIntObjects(const std::string& metaclass)
{
	size_t pos = metaclass.find_last_of('.');
	if (pos == std::string::npos)
		return IntObjects[metaclass];
	else
		return IntObjects[metaclass.substr(pos + 1)];
}

std::string PackageManager::Localize(std::string packageName, const std::string& sectionName, const std::string& keyName)
{
	/*
	for (char& c : packageName)
	{
		if (c >= 'A' && c <= 'Z')
			c += 'a' - 'A';
	}
	*/

	auto& intFile = intFiles[packageName];
	if (!intFile)
	{
		try
		{
			intFile = std::make_unique<IniFile>(FilePath::combine(basepath, "System/" + packageName + ".int"));
		}
		catch (...)
		{
			intFile = std::make_unique<IniFile>();
		}
	}

	return intFile->GetValue(sectionName, keyName);
}

std::map<std::string, std::string> PackageManager::ParseIntPublicValue(const std::string& text)
{
	// Parse one of the following:
	//
	// Object=(Name=Package.ObjectName,Class=ObjectClass,MetaClass=Package.MetaClassName,Description="descriptive string")
	// Preferences=(Caption="display name",Parent="display name of parent",Class=Package.ClassName,Category=variable group name,Immediate=True)

	if (text.size() < 2 || text.front() != '(' || text.back() != ')')
		return {};

	std::map<std::string, std::string> desc;

	// This would have been so much easier with a regular expression, but we can't use that as we have no idea what character set those .int files might be using
	size_t pos = 1;
	while (pos < text.size() - 1)
	{
		size_t endpos = text.find('=', pos);
		if (endpos == std::string::npos)
			break;
		std::string keyname = text.substr(pos, endpos - pos);
		pos = endpos + 1;

		if (text[pos] == '"')
		{
			pos++;
			endpos = text.find('"', pos);
			if (endpos == std::string::npos)
				break;

			std::string value = text.substr(pos, endpos - pos);
			desc[keyname] = value;
			pos++;

			pos = text.find(',', pos);
			if (pos == std::string::npos)
				break;
			pos++;
		}
		else
		{
			endpos = text.find_first_of(",)", pos);
			if (endpos == std::string::npos)
				break;
			std::string value = text.substr(pos, endpos - pos);
			desc[keyname] = value;
			pos = endpos + 1;
		}
	}

	return desc;
}
