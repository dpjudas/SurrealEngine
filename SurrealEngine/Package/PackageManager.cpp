
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

	LoadIntFiles();
	LoadPackageRemaps();
	ScanForMaps();
	ScanPaths();

	InitPropertyOffsets(this);

	// File::write_all_text("C:\\Development\\UTNativeProps.txt", NativeObjExtractor::Run(this));
	// File::write_all_text("C:\\Development\\UTNativeFuncs.txt", NativeFuncExtractor::Run(this));
}

Package* PackageManager::GetPackage(const NameString& name)
{
	auto& package = packages[name];
	if (package)
		return package.get();
	
	// TODO: Is this how PackageRemap really works?
	// There is no documentation of it anywhere it seems
	NameString remapped_name = name;

	auto remap_it = packageRemaps.find(name.ToString());
	if (remap_it != packageRemaps.end())
	{
		remapped_name = NameString(remap_it->second);
	}

	auto it = packageFilenames.find(remapped_name);
	if (it != packageFilenames.end())
	{
		package = std::make_unique<Package>(this, remapped_name, it->second);
	}
	else
	{
		throw std::runtime_error("Could not find package " + name.ToString());
	}

	return package.get();
}

void PackageManager::UnloadPackage(const NameString& name)
{
	auto it = packages.find(name);
	if (it != packages.end())
	{
		for (auto streamit = openStreams.begin(); streamit != openStreams.end(); ++streamit)
		{
			if (streamit->Pkg == it->second.get())
			{
				openStreams.erase(streamit);
				break;
			}
		}
		packages.erase(it);
	}
}

void PackageManager::ScanForMaps()
{
	std::string packagedir = FilePath::combine(launchInfo.folder, "Maps");
	for (std::string filename : Directory::files(FilePath::combine(packagedir, "*.unr")))
	{
		maps.push_back(filename);
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

	for (auto& current_path : paths)
	{
		// Get the filename
		std::string filename = FilePath::last_component(current_path);
		current_path = FilePath::remove_last_component(current_path);

		// Paths are relative from the System folder the executable (and the ini file) is in
		// So we should start from there
		auto resulting_root_path = FilePath::combine(launchInfo.folder, "System");

		auto first_component = FilePath::first_component(current_path);

		while (first_component == ".." || first_component == ".")
		{
			if (first_component == "..")
			{
				// "Go one directory up" as many as the amount of ".."s in the current_path
				resulting_root_path = FilePath::remove_last_component(resulting_root_path);
			}

			current_path = FilePath::remove_first_component(current_path);
			first_component = FilePath::first_component(current_path);
		}

		// Combine everything
		auto final_path = FilePath::combine(resulting_root_path, current_path);

		ScanFolder(final_path, filename);
	}
}

std::vector<NameString> PackageManager::GetPackageNames() const
{
	std::vector<NameString> names;
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
		throw std::runtime_error("Could not find class " + className.ToString());
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

std::vector<NameString> PackageManager::GetIniKeysFromSection(NameString iniName, const NameString& sectionName)
{
	if (iniName == "system" || iniName == "System")
		iniName = launchInfo.gameName;
	else if (iniName == "user")
		iniName = "User";

	auto& ini = iniFiles[iniName];
	if (!ini)
	{
		ini = std::make_unique<IniFile>(FilePath::combine(launchInfo.folder, "System/" + iniName.ToString() + ".ini"));
	}

	return ini->GetKeys(sectionName);
}

std::string PackageManager::GetIniValue(NameString iniName, const NameString& sectionName, const NameString& keyName)
{
	if (iniName == "system" || iniName == "System")
		iniName = launchInfo.gameName;
	else if (iniName == "user")
		iniName = "User";

	auto& ini = iniFiles[iniName];
	if (!ini)
	{
		ini = std::make_unique<IniFile>(FilePath::combine(launchInfo.folder, "System/" + iniName.ToString() + ".ini"));
	}

	return ini->GetValue(sectionName, keyName);
}

std::vector<std::string> PackageManager::GetIniValues(NameString iniName, const NameString& sectionName, const NameString& keyName)
{
	if (iniName == "system" || iniName == "System")
		iniName = launchInfo.gameName;
	else if (iniName == "user")
		iniName = "User";

	auto& ini = iniFiles[iniName];
	if (!ini)
	{
		ini = std::make_unique<IniFile>(FilePath::combine(launchInfo.folder, "System/" + iniName.ToString() + ".ini"));
	}

	return ini->GetValues(sectionName, keyName);
}

void PackageManager::LoadPackageRemaps()
{
	auto remap_keys = GetIniKeysFromSection("system", "PackageRemap");

	for (auto& key : remap_keys)
	{
		packageRemaps[key.ToString()] = GetIniValue("system", "PackageRemap", key);
	}
}

void PackageManager::LoadIntFiles()
{
	std::string systemdir = FilePath::combine(launchInfo.folder, "System");
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

std::vector<IntObject>& PackageManager::GetIntObjects(const NameString& metaclass)
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
			intFile = std::make_unique<IniFile>(FilePath::combine(launchInfo.folder, "System/" + packageName.ToString() + ".int"));
		}
		catch (...)
		{
			intFile = std::make_unique<IniFile>();
		}
	}

	return intFile->GetValue(sectionName, keyName);
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
