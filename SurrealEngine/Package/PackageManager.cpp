
#include "Precomp.h"
#include "PackageManager.h"
#include "Package.h"
#include "PackageStream.h"
#include "IniFile.h"
#include "Utils/File.h"
#include "Utils/StrCompare.h"
#include "UObject/UObject.h"
#include "UObject/UClass.h"
#include "VM/NativeFunc.h"
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
#include "Native/NInternetLink.h"
#include "Native/NTcpLink.h"
#include "Native/NUdpLink.h"
#include "Native/NDebugInfo.h"
#include "Native/NDeusExDecoration.h"
#include "Native/NDeusExPlayer.h"
#include "Native/NDeusExSaveInfo.h"
#include "Native/NDumpLocation.h"
#include "Native/NGameDirectory.h"
#include "Native/NParticleIterator.h"
#include "Native/NScriptedPawn.h"
#include "Native/NPlayerPawnExt.h"

PackageManager::PackageManager(const GameLaunchInfo& launchInfo) : launchInfo(launchInfo)
{
	RegisterFunctions();
	LoadEngineIniFiles();
	LoadIntFiles();
	LoadPackageRemaps();
	ScanPaths();
	ScanForMaps();

	InitPropertyOffsets(this);

	// File::write_all_text("C:\\Development\\UTNativeProps.txt", NativeObjExtractor::Run(this));
	// File::write_all_text("C:\\Development\\UTNativeFuncs.txt", NativeFuncExtractor::Run(this));
}

Package* PackageManager::GetPackage(const NameString& name)
{
	auto& package = packages[name];
	if (package)
		return package.get();
	
	auto it = packageFilenames.find(name);
	if (it != packageFilenames.end())
	{
		package.set(GC::Alloc<Package>(this, name, it->second));
	}
	else
	{
		Exception::Throw("Could not find package " + name.ToString());
	}

	return package.get();
}

Package* PackageManager::LoadMap(const std::string& path)
{
	std::string map = FilePath::last_component(path);

	if (!FilePath::has_extension(map, GetMapExtension().c_str()))
		map += "." + GetMapExtension();

	// Check if the map is in the map list
	for (auto& mapName : maps)
	{
		if (StrCompare::equals_ignore_case(mapName, map))
			map = mapName; // Workaround against case sensitivity problems under Linux
	}

	// Check the map name against the map folders we know
	for (auto& folder : mapFolders)
	{
		std::string finalPath = FilePath::combine(folder, map);

		if (FilePath::exists(finalPath))
		{
			return GC::Alloc<Package>(this, FilePath::remove_extension(map), finalPath);
		}
	}

	// Path is relative to the Maps folder?
	// Or is it relative to the package requesting the map load?
	// Or is it relative to the previous map?
	// 
	// Only one of the above is most likely true. Lets begin with assuming its relative to the Maps folder.
	std::string absolute_path = FilePath::relative_to_absolute_from_system(FilePath::combine(launchInfo.gameRootFolder, "Maps"), path);
	// Add the file extension if it is missing
	if (!FilePath::has_extension(absolute_path, GetMapExtension().c_str()))
		absolute_path += "." + GetMapExtension();
	return GC::Alloc<Package>(this, FilePath::remove_extension(map), absolute_path);
}

void PackageManager::UnloadMap(Package* package)
{
	// Remove package from open streams cache:
	auto streamit = openStreams.begin();
	while (streamit != openStreams.end())
	{
		if (streamit->Pkg == package)
		{
			streamit = openStreams.erase(streamit);
		}
		else
		{
			++streamit;
		}
	}
}

void PackageManager::ScanForMaps()
{
	for (auto& mapFolderPath : mapFolders)
	{
		for (std::string filename : Directory::files(FilePath::combine(mapFolderPath, "*." + mapExtension)))
		{
			maps.push_back(filename);
		}
	}
}

void PackageManager::ScanFolder(const std::string& packagedir, const std::string& search)
{
	for (std::string filename : Directory::files(FilePath::combine(packagedir, search)))
	{
		// Do not add the package again if it exists
		// This is useful for example when you have HD textures installed in a different folder
		// And you wish to load them instead of the original ones
		auto it = packageFilenames.find(NameString(FilePath::remove_extension(filename)));
		if (it == packageFilenames.end())
			packageFilenames[NameString(FilePath::remove_extension(filename))] = FilePath::combine(packagedir, filename);
	}
}

void PackageManager::ScanPaths()
{
	auto paths = GetIniValues("system", "Core.System", "Paths");
	mapExtension = GetIniValue("System", "URL", "MapExt");
	saveExtension = GetIniValue("System", "URL", "SaveExt");

	for (auto& current_path : paths)
	{
		// Get the filename
		std::string filename = FilePath::last_component(current_path);

		// Calculate the final, absolute path
		auto final_path = FilePath::relative_to_absolute_from_system(FilePath::combine(launchInfo.gameRootFolder, "System"), current_path);
		final_path = FilePath::remove_last_component(final_path);

		// Add map folders in a separate list, so ScanForMaps() can use them
		if (filename == "*." + mapExtension)
		{
			mapFolders.push_back(final_path);
		}

		ScanFolder(final_path, filename);
	}
}

Array<NameString> PackageManager::GetPackageNames() const
{
	Array<NameString> names;
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

UObject* PackageManager::NewObject(const NameString& name, const NameString& package, const NameString& className)
{
	Package* pkg = GetPackage(package);
	UClass* cls = UObject::Cast<UClass>(pkg->GetUObject("Class", className));
	if (!cls)
		Exception::Throw("Could not find class " + className.ToString());
	return pkg->NewObject(name, cls, ObjectFlags::NoFlags, true);
}

UObject* PackageManager::NewObject(const NameString& name, UClass* cls)
{
	// To do: package needs to be grabbed from outer, or the "transient package" if it is None, a virtual package for runtime objects
	return GetPackage("Engine")->NewObject(name, cls, ObjectFlags::NoFlags, true);
}

UClass* PackageManager::FindClass(const NameString& name)
{
	std::string value = name.ToString();
	size_t pos = value.find('.');
	if (pos == 0 || pos == std::string::npos || pos + 1 == value.size())
		return nullptr;

	NameString packageName = value.substr(0, pos);
	NameString className = value.substr(pos + 1);

	try
	{
		return UObject::Cast<UClass>(GetPackage(packageName)->GetUObject("Class", className));
	}
	catch (...)
	{
		return nullptr;
	}
}

std::unique_ptr<IniFile> PackageManager::GetIniFile(NameString iniName)
{
	bool userIni = (iniName == "user");
	if (userIni && launchInfo.engineVersion > 219)
		iniName = "User";
	if (iniName == "system" || iniName == "System" || (userIni && launchInfo.engineVersion == 219))
	{
		// Clive Barker's Undying uses System.ini instead of ExeName.ini
		if (IsCliveBarkersUndying())
			iniName = "System";
		else
			iniName = launchInfo.gameExecutableName;
	}

	return std::make_unique<IniFile>(*iniFiles[iniName]);
}

std::unique_ptr<IniFile>& PackageManager::GetSystemIniFile(NameString iniName)
{
	bool userIni = (iniName == "user");
	if (userIni && launchInfo.engineVersion > 219)
		iniName = "User";
	if (iniName == "system" || iniName == "System" || (userIni && launchInfo.engineVersion == 219))
	{
		// Clive Barker's Undying uses System.ini instead of ExeName.ini
		if (IsCliveBarkersUndying())
			iniName = "System";
		else
			iniName = launchInfo.gameExecutableName;
	}
	
	auto& ini = iniFiles[iniName];
	if (!ini)
	{
		ini = std::make_unique<IniFile>(FilePath::combine(launchInfo.gameRootFolder, "System/" + iniName.ToString() + ".ini"));
	}

	return ini;
}

Array<NameString> PackageManager::GetIniKeysFromSection(NameString iniName, const NameString& sectionName)
{
	return GetSystemIniFile(iniName)->GetKeys(sectionName);
}

std::string PackageManager::GetIniValue(NameString iniName, const NameString& sectionName, const NameString& keyName, std::string default_value, const int index)
{
	return GetSystemIniFile(iniName)->GetValue(sectionName, keyName, default_value, index);
}

Array<std::string> PackageManager::GetIniValues(NameString iniName, const NameString& sectionName, const NameString& keyName, Array<std::string> default_values)
{
	return GetSystemIniFile(iniName)->GetValues(sectionName, keyName, default_values);
}

void PackageManager::SetIniValue(NameString iniName, const NameString& sectionName, const NameString& keyName, const std::string& newValue, const int index)
{
	GetSystemIniFile(iniName)->SetValue(sectionName, keyName, newValue, index);
}

void PackageManager::SetIniValues(NameString iniName, const NameString& sectionName, const NameString& keyName, const Array<std::string>& newValues)
{
	GetSystemIniFile(iniName)->SetValues(sectionName, keyName, newValues);
}

void PackageManager::SaveAllIniFiles()
{
	const std::string system_folder = FilePath::combine(launchInfo.gameRootFolder, "System");

	for (auto& iniFile : iniFiles)
	{
		if (iniFile.first == launchInfo.gameExecutableName)
		{
			const std::string engine_ini_name = "SE-" + launchInfo.gameExecutableName + ".ini";
			iniFile.second->UpdateIfExists(FilePath::combine(system_folder, engine_ini_name));
		}
		else if (iniFile.first == "User")
			iniFile.second->UpdateIfExists(FilePath::combine(system_folder, "SE-User.ini"));
		else
			iniFile.second->UpdateFile();
	}
}

void PackageManager::LoadPackageRemaps()
{
	auto remap_keys = GetIniKeysFromSection("system", "PackageRemap");

	for (auto& key : remap_keys)
	{
		packageRemaps[key.ToString()] = GetIniValue("system", "PackageRemap", key);
	}
}

void PackageManager::LoadEngineIniFiles()
{
	// Load SE-[GameName].ini and SE-User.ini from the appropriate places
	// If they do not exist, import the appropriate [GameName].ini and User.ini files

	std::string engine_ini_name = "SE-" + launchInfo.gameExecutableName + ".ini";
	std::string user_ini_name = "SE-User.ini";

	const std::string system_folder = FilePath::combine(launchInfo.gameRootFolder, "System");

	if (!File::try_open_existing(FilePath::combine(system_folder, engine_ini_name)))
	{
		missing_se_system_ini = true;
		engine_ini_name = engine_ini_name.substr(3); // Trim off the "SE-" part
	}
	
	if (IsCliveBarkersUndying())
		iniFiles["System"] = std::make_unique<IniFile>(FilePath::combine(system_folder, "System.ini"));
	else
		iniFiles[launchInfo.gameExecutableName] = std::make_unique<IniFile>(FilePath::combine(system_folder, engine_ini_name));

	if (launchInfo.engineVersion > 209)
	{
		if (!File::try_open_existing(FilePath::combine(system_folder, user_ini_name)))
			user_ini_name = user_ini_name.substr(3); // Trim off the "SE-" part

		iniFiles["User"] = std::make_unique<IniFile>(FilePath::combine(system_folder, user_ini_name));
	}
}

void PackageManager::LoadIntFiles()
{
	std::string systemdir = FilePath::combine(launchInfo.gameRootFolder, "System");
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

					NameString metaClass = obj.MetaClass;

					size_t pos = metaClass.ToString().find_last_of('.');
					if (pos != std::string::npos)
						metaClass = NameString(metaClass.ToString().substr(pos + 1));

					IntObjects[metaClass].push_back(std::move(obj));
				}
				else if (!desc["Name"].empty() && !desc["Class"].empty()) // Used by Actor.GetNextSkin
				{
					IntObject obj;
					obj.Name = desc["Name"];
					obj.Class = desc["Class"];
					obj.Description = desc["Description"];

					NameString cls = obj.Class;

					size_t pos = cls.ToString().find_last_of('.');
					if (pos != std::string::npos)
						cls = NameString(cls.ToString().substr(pos + 1));

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

Array<IntObject>& PackageManager::GetIntObjects(const NameString& metaclass)
{
	size_t pos = metaclass.ToString().find_last_of('.');
	if (pos == std::string::npos)
		return IntObjects[metaclass];
	else
		return IntObjects[NameString(metaclass.ToString().substr(pos + 1))];
}

std::string PackageManager::Localize(NameString packageName, const NameString& sectionName, const NameString& keyName)
{
	if (packageName == "Engine" && keyName == "ClassCaption")
	{
		if (sectionName == "SurrealRenderDevice") return "Surreal Graphics";
		else if (sectionName == "SurrealAudioDevice") return "Surreal Audio";
		else if (sectionName == "SurrealNetworkDevice") return "Surreal Network";
	}

	auto& intFile = intFiles[packageName];
	if (!intFile)
	{
		try
		{
			intFile = std::make_unique<IniFile>(FilePath::combine(launchInfo.gameRootFolder, "System/" + packageName.ToString() + ".int"));
		}
		catch (...)
		{
			intFile = std::make_unique<IniFile>();
		}
	}

	std::string value = intFile->GetValue(sectionName, keyName);
	if (!value.empty() && value.front() == '"' && value.back() == '"')
	{
		value.erase(value.begin());
		value.erase(value.end()-1);
	}
	return value;
}

std::map<NameString, std::string> PackageManager::ParseIntPublicValue(const std::string& text)
{
	// Parse one of the following:
	//
	// Object=(Name=Package.ObjectName,Class=ObjectClass,MetaClass=Package.MetaClassName,Description="descriptive string")
	// Preferences=(Caption="display name",Parent="display name of parent",Class=Package.ClassName,Category=variable group name,Immediate=True)

	if (text.size() < 2 || text.front() != '(' || text.back() != ')')
		return {};

	std::map<NameString, std::string> desc;

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

void PackageManager::RegisterFunctions()
{
	if (launchInfo.gameExecutableName == "UnrealTournament" && launchInfo.gameVersionString == "436")
	{
		//{AUTOGENERATED(Register,UnrealTournament-436)
		//}AUTOGENERATED
	}
	else if (launchInfo.gameExecutableName == "Unreal" && launchInfo.gameVersionString == "226b")
	{
		//{AUTOGENERATED(Register,Unreal-226b)
		//}AUTOGENERATED
	}
	else if (launchInfo.gameExecutableName == "DeusEx" && launchInfo.gameVersionString == "1112fm")
	{
		//{AUTOGENERATED(Register,DeusEx-1112fm)
		//}AUTOGENERATED
	}

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
	NInternetLink::RegisterFunctions();
	NTcpLink::RegisterFunctions();
	NUdpLink::RegisterFunctions();
	if (IsDeusEx())
	{
		NDebugInfo::RegisterFunctions();
		NDeusExDecoration::RegisterFunctions();
		NDeusExPlayer::RegisterFunctions();
		NDeusExSaveInfo::RegisterFunctions();
		NDumpLocation::RegisterFunctions();
		NGameDirectory::RegisterFunctions();
		NParticleIterator::RegisterFunctions();
		NScriptedPawn::RegisterFunctions();
		NPlayerPawnExt::RegisterFunctions();
	}
}
