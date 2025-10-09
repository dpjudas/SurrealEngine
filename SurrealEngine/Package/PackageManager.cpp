
#include "Precomp.h"
#include "PackageManager.h"
#include "Package.h"
#include "PackageStream.h"
#include "IniFile.h"
#include "PackageWriter.h"
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
	gameRootFolderPath = fs::path(launchInfo.gameRootFolder);
	gameSystemFolderPath = gameRootFolderPath / "System";

	CreateTransientPackage();
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

void PackageManager::CreateTransientPackage()
{
	packages["Transient"].set(GC::Alloc<Package>(this, "Transient", ""));
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
	const auto mapPath = convert_path_separators(path);

	auto mapFilename = mapPath.filename();

	if (!mapFilename.has_extension())
		mapFilename.replace_extension(GetMapExtension());

	// Check if the map is in the map list
	for (auto& mapName : maps)
	{
		if (StrCompare::equals_ignore_case(mapName, mapFilename.string()))
			mapFilename.replace_filename(mapName); // Workaround against case sensitivity problems under Linux.
	}

	// Check the map name against the map folders we know
	// This should work if path only contains the map name and not any "."s or ".."s
	for (auto& folder : mapFolders)
	{
		if (auto finalPath = fs::path(folder) / mapFilename ; fs::exists(finalPath))
			return GC::Alloc<Package>(this, mapFilename.stem().string(), finalPath.string());
	}

	// Path is relative to the Maps folder?
	// Or is it relative to the package requesting the map load?
	// Or is it relative to the previous map?
	//
	// Only one of the above is most likely true. Let's begin with assuming its relative to the Maps folder.
	const auto mapsFolder = gameRootFolderPath / "Maps";

	auto absolute_path = (mapsFolder / mapPath).lexically_normal();

	// Add the file extension if it is missing
	if (!absolute_path.has_extension())
		absolute_path.replace_extension(GetMapExtension());

	return GC::Alloc<Package>(this, mapFilename.stem().string(), absolute_path.string());
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

void PackageManager::CloseStreams()
{
	openStreams.clear();
}

Package* PackageManager::LoadSaveFile(const std::string& path)
{
	const auto saveFolder = gameRootFolderPath / "Save";
	auto fullPath = saveFolder / fs::path(path);

	if (!fullPath.has_extension() || (fullPath.extension().string() != GetSaveExtension()))
		fullPath.replace_extension(GetSaveExtension());

	if (fs::exists(fullPath))
		return GC::Alloc<Package>(this, fullPath.stem().string(), fullPath.string());

	return nullptr;
}

Package* PackageManager::LoadSaveSlot(const uint32_t slotNum)
{
	return LoadSaveFile("Save" + std::to_string(slotNum) + "." + GetSaveExtension());
}

void PackageManager::ScanForMaps()
{
	for (auto& mapFolderPath : mapFolders)
	{
		for (const auto& dir_entry: fs::directory_iterator{mapFolderPath})
			if (dir_entry.is_regular_file())
				maps.push_back(dir_entry.path().filename().string());
	}
}

void PackageManager::ScanFolder(const std::string& packagedir, const std::string& search)
{
	const auto packageDirPath = fs::path(packagedir);
	if (fs::exists(packageDirPath))
	{
		const auto searchExt = fs::path(search).extension();
		for (const auto& dir_entry: fs::directory_iterator{packageDirPath})
		{
			// Do not add the package again if it exists
			// This is useful for example when you have HD textures installed in a different folder
			// And you wish to load them instead of the original ones
			if (dir_entry.is_regular_file() && dir_entry.path().extension() == searchExt)
			{
				NameString fileNameString(dir_entry.path().stem().string());
				auto it = packageFilenames.find(fileNameString);
				if (it == packageFilenames.end())
					packageFilenames[fileNameString] = (packageDirPath / dir_entry.path().filename()).string();
			}
		}
	}
}

void PackageManager::ScanPaths()
{
	Array<std::string> paths;
	if (launchInfo.engineVersion <= 219) // khg with 219 uses old format. unreal uses new in 226
	{
		for (int i = 0; i < 16; i++)
		{
			std::string value = GetIniValue("system", "Core.System", "Paths[" + std::to_string(i) + "]");
			if (!value.empty())
				paths.push_back(std::move(value));
		}
	}
	else
	{
		paths = GetIniValues("system", "Core.System", "Paths");
	}
	mapExtension = GetIniValue("System", "URL", "MapExt");
	saveExtension = GetIniValue("System", "URL", "SaveExt");

	for (auto& currentPathStr: paths)
	{
		auto currentPath = convert_path_separators(currentPathStr);

		// Get the filename
		auto filename = currentPath.filename();

		// Calculate the final, absolute path
		auto finalPath = (gameSystemFolderPath / currentPath).lexically_normal().parent_path();

		// Add map folders in a separate list, so ScanForMaps() can use them
		if (filename.string() == "*." + GetMapExtension())
			mapFolders.push_back(finalPath.string());

		ScanFolder(finalPath.string(), filename.string());
	}

	if (IsKlingonHonorGuard())
	{
		for (const auto& dir_entry: fs::directory_iterator{gameSystemFolderPath})
		{
			if (dir_entry.is_regular_file() && dir_entry.path().extension().string() == ".avi")
			{
				auto fileName = dir_entry.path().filename();
				aviFilenames[NameString(fileName.string())] = (gameSystemFolderPath / fileName).string();
			}
		}
	}
}

std::string PackageManager::GetVideoFilename(const std::string& name)
{
	NameString filename = name;
	auto it = aviFilenames.find(name);
	if (it == aviFilenames.end())
		return {};
	return it->second;
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

UClass* PackageManager::FindClass(const NameString& name)
{
	const std::string& value = name.ToString();
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
	if (iniName == "user")
		return GetUserIniFile();
	if (iniName == "System")
		return GetSystemIniFile();

	return std::make_unique<IniFile>(*iniFiles[iniName]);
}

std::unique_ptr<IniFile> PackageManager::GetUserIniFile()
{
	if (IsKlingonHonorGuard() || launchInfo.engineVersion <= 219)
		return std::make_unique<IniFile>(*iniFiles[launchInfo.gameExecutableName]);

	return std::make_unique<IniFile>(*iniFiles["User"]);
}

std::unique_ptr<IniFile> PackageManager::GetSystemIniFile()
{
	// Clive Barker's Undying uses System.ini instead of ExeName.ini
	if (IsCliveBarkersUndying())
		return std::make_unique<IniFile>(*iniFiles["System"]);

	return std::make_unique<IniFile>(*iniFiles[launchInfo.gameExecutableName]);
}

std::unique_ptr<IniFile>& PackageManager::LoadIniFile(NameString iniName)
{
	if (iniName == "user")
		return LoadUserIniFile();
	if (iniName == "System")
		return LoadSystemIniFile();
	
	auto& ini = iniFiles[iniName];
	if (!ini)
	{
		const auto iniFilePath = gameSystemFolderPath / (iniName.ToString() + ".ini");
		ini = std::make_unique<IniFile>(iniFilePath.string());
	}

	return ini;
}

std::unique_ptr<IniFile>& PackageManager::LoadUserIniFile()
{
	if (IsKlingonHonorGuard())
	{
		// User.ini contents are in the system ini file
		auto& ini = iniFiles[launchInfo.gameExecutableName];

		if (!ini)
		{
			const auto iniFilePath = gameSystemFolderPath / (launchInfo.gameExecutableName + ".ini");
			ini = std::make_unique<IniFile>(iniFilePath.string());
		}

		return ini;
	}

	auto& ini = iniFiles["User"];

	if (!ini)
	{
		// Case sensitivity check
		if (fs::exists(gameSystemFolderPath / "User.ini"))
			ini = std::make_unique<IniFile>((gameSystemFolderPath / "User.ini").string());
		else
			ini = std::make_unique<IniFile>((gameSystemFolderPath/ "user.ini").string());
	}

	return ini;
}

std::unique_ptr<IniFile>& PackageManager::LoadSystemIniFile()
{
	const std::string iniName = IsCliveBarkersUndying() ? "System" : launchInfo.gameExecutableName;

	auto& ini = iniFiles[iniName];

	if (!ini)
		ini = std::make_unique<IniFile>((gameSystemFolderPath / (iniName + ".ini")).string());

	return ini;
}


Array<NameString> PackageManager::GetIniKeysFromSection(NameString iniName, const NameString& sectionName)
{
	return LoadIniFile(iniName)->GetKeys(sectionName);
}

std::string PackageManager::GetIniValue(NameString iniName, const NameString& sectionName, const NameString& keyName, std::string default_value, const int index)
{
	return LoadIniFile(iniName)->GetValue(sectionName, keyName, default_value, index);
}

Array<std::string> PackageManager::GetIniValues(NameString iniName, const NameString& sectionName, const NameString& keyName, Array<std::string> default_values)
{
	return LoadIniFile(iniName)->GetValues(sectionName, keyName, default_values);
}

std::string PackageManager::GetDefaultIniValue(const NameString& sectionName, const NameString& keyName, std::string default_value, const int index)
{
	return defaultIniFile->GetValue(sectionName, keyName, default_value, index);
}

Array<std::string> PackageManager::GetDefaultIniValues(const NameString& sectionName, const NameString& keyName, Array<std::string> default_values)
{
	return defaultIniFile->GetValues(sectionName, keyName, default_values);
}

std::string PackageManager::GetDefUserIniValue(const NameString& sectionName, const NameString& keyName, std::string default_value, const int index)
{
	if (IsKlingonHonorGuard() || launchInfo.engineVersion <= 219)
		return defaultIniFile->GetValue(sectionName, keyName, default_value, index);

	return defaultUserFile->GetValue(sectionName, keyName, default_value, index);
}

Array<std::string> PackageManager::GetDefUserIniValues(const NameString& sectionName, const NameString& keyName, Array<std::string> default_values)
{
	if (IsKlingonHonorGuard() || launchInfo.engineVersion <= 219)
		return defaultIniFile->GetValues(sectionName, keyName, default_values);

	return defaultUserFile->GetValues(sectionName, keyName, default_values);
}

void PackageManager::SetIniValue(NameString iniName, const NameString& sectionName, const NameString& keyName, const std::string& newValue, const int index)
{
	LoadIniFile(iniName)->SetValue(sectionName, keyName, newValue, index);
}

void PackageManager::SetIniValues(NameString iniName, const NameString& sectionName, const NameString& keyName, const Array<std::string>& newValues)
{
	LoadIniFile(iniName)->SetValues(sectionName, keyName, newValues);
}

void PackageManager::SaveAllIniFiles()
{
	for (auto& iniFile: iniFiles)
	{
		if (iniFile.first == launchInfo.gameExecutableName || iniFile.first == "System")
		{
			const std::string engineIniName = "SE-" + iniFile.first.ToString() + ".ini";
			iniFile.second->UpdateIfExists((gameSystemFolderPath / engineIniName).string());
		}
		else if (iniFile.first == "User")
			iniFile.second->UpdateIfExists((gameSystemFolderPath / "SE-User.ini").string());
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
	// If those also do not exist, import Default.ini and DefUser.ini files
	const auto systemIniName = IsCliveBarkersUndying() ? "System" : launchInfo.gameExecutableName;
	std::string systemIniFileName = "SE-" + systemIniName + ".ini";
	std::string userIniName = "SE-User.ini";

	if ( !File::try_open_existing((gameSystemFolderPath / systemIniFileName).string()) )
	{
		missing_se_system_ini = true;
		systemIniFileName = systemIniFileName.substr(3); // Trim off the "SE-" part
		if (!File::try_open_existing((gameSystemFolderPath / systemIniFileName).string()))
			systemIniFileName = "Default.ini"; // use the default ini as a last resort
	}

	// Also load Default.ini, so that we can reset values.
	defaultIniFile = std::make_unique<IniFile>((gameSystemFolderPath / "Default.ini").string());

	iniFiles[systemIniName] = std::make_unique<IniFile>((gameSystemFolderPath / systemIniFileName).string());

	if (launchInfo.engineVersion > 219)
	{
		if (!File::try_open_existing((gameSystemFolderPath / userIniName).string()))
		{
			userIniName = userIniName.substr(3); // Trim off the "SE-" part
			if (!File::try_open_existing((gameSystemFolderPath / userIniName).string()))
				userIniName = "DefUser.ini";
		}

		iniFiles["User"] = std::make_unique<IniFile>((gameSystemFolderPath / userIniName).string());
		defaultUserFile = std::make_unique<IniFile>((gameSystemFolderPath / "DefUser.ini").string());
	}
}

void PackageManager::LoadIntFiles()
{
	for (const auto& dir_entry: fs::directory_iterator{gameSystemFolderPath})
	{
		try
		{
			if (dir_entry.is_regular_file() && dir_entry.path().extension().string() == ".int")
			{
				const auto intFileName = dir_entry.path().filename();

				auto intFile = std::make_unique<IniFile>((gameSystemFolderPath / intFileName).string());

				for (const std::string& value: intFile->GetValues("Public", "Object"))
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

				intFiles[intFileName.stem().string()] = std::move(intFile);
			}
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

std::string PackageManager::Localize(NameString packageName, const NameString& sectionName, const NameString& keyName, const int index)
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
			const auto intFileName = fs::path(packageName.ToString() + ".int");
			intFile = std::make_unique<IniFile>((gameSystemFolderPath / intFileName).string());
		}
		catch (...)
		{
			intFile = std::make_unique<IniFile>();
		}
	}

	std::string value = intFile->GetValue(sectionName, keyName, {}, index);
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
