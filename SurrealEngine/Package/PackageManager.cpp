
#include "Precomp.h"
#include "PackageManager.h"
#include "Package.h"
#include "PackageStream.h"
#include "IniFile.h"
#include "PackageWriter.h"
#include "Utils/File.h"
#include "Utils/StrTools.h"
#include "VM/NativeFunc.h"
#include "Packages/ConSys/UConAudioList.h"
#include "Packages/ConSys/UConCamera.h"
#include "Packages/ConSys/UConChoice.h"
#include "Packages/ConSys/UConFlagRef.h"
#include "Packages/ConSys/UConItem.h"
#include "Packages/ConSys/UConLight.h"
#include "Packages/ConSys/UConListItem.h"
#include "Packages/ConSys/UConObject.h"
#include "Packages/ConSys/UConSpeech.h"
#include "Packages/ConSys/UConversation.h"
#include "Packages/ConSys/UConversationList.h"
#include "Packages/ConSys/UConversationMissionList.h"
#include "Packages/ConSys/Events/UConEvent.h"
#include "Packages/ConSys/Events/UConEventAddCredits.h"
#include "Packages/ConSys/Events/UConEventAddGoal.h"
#include "Packages/ConSys/Events/UConEventAddNote.h"
#include "Packages/ConSys/Events/UConEventAddSkillPoints.h"
#include "Packages/ConSys/Events/UConEventAnimation.h"
#include "Packages/ConSys/Events/UConEventCheckFlag.h"
#include "Packages/ConSys/Events/UConEventCheckObject.h"
#include "Packages/ConSys/Events/UConEventCheckPersona.h"
#include "Packages/ConSys/Events/UConEventChoice.h"
#include "Packages/ConSys/Events/UConEventComment.h"
#include "Packages/ConSys/Events/UConEventEnd.h"
#include "Packages/ConSys/Events/UConEventJump.h"
#include "Packages/ConSys/Events/UConEventMoveCamera.h"
#include "Packages/ConSys/Events/UConEventRandomLabel.h"
#include "Packages/ConSys/Events/UConEventSetFlag.h"
#include "Packages/ConSys/Events/UConEventSpeech.h"
#include "Packages/ConSys/Events/UConEventTrade.h"
#include "Packages/ConSys/Events/UConEventTransferObject.h"
#include "Packages/ConSys/Events/UConEventTrigger.h"
#include "Packages/ConSys/History/UConHistory.h"
#include "Packages/ConSys/History/UConHistoryEvent.h"
#include "Packages/Core/UClass.h"
#include "Packages/Core/UCommandlet.h"
#include "Packages/Core/UConst.h"
#include "Packages/Core/UEnum.h"
#include "Packages/Core/UField.h"
#include "Packages/Core/UFunction.h"
#include "Packages/Core/ULanguage.h"
#include "Packages/Core/UObject.h"
#include "Packages/Core/UPackage.h"
#include "Packages/Core/USimpleCommandlet.h"
#include "Packages/Core/UState.h"
#include "Packages/Core/UStruct.h"
#include "Packages/Core/USubsystem.h"
#include "Packages/Core/UTextBuffer.h"
#include "Packages/Core/Properties/UAnyProperty.h"
#include "Packages/Core/Properties/UArrayProperty.h"
#include "Packages/Core/Properties/UBoolProperty.h"
#include "Packages/Core/Properties/UByteProperty.h"
#include "Packages/Core/Properties/UClassProperty.h"
#include "Packages/Core/Properties/UFixedArrayProperty.h"
#include "Packages/Core/Properties/UFloatProperty.h"
#include "Packages/Core/Properties/UIntProperty.h"
#include "Packages/Core/Properties/UMapProperty.h"
#include "Packages/Core/Properties/UNameProperty.h"
#include "Packages/Core/Properties/UObjectProperty.h"
#include "Packages/Core/Properties/UPointerProperty.h"
#include "Packages/Core/Properties/UProperty.h"
#include "Packages/Core/Properties/UStringProperty.h"
#include "Packages/Core/Properties/UStrProperty.h"
#include "Packages/Core/Properties/UStructProperty.h"
#include "Packages/DeusEx/UDeusExDecoration.h"
#include "Packages/DeusEx/UDeusExLevelInfo.h"
#include "Packages/DeusEx/UDeusExPlayer.h"
#include "Packages/DeusEx/UDeusExSaveInfo.h"
#include "Packages/DeusEx/UGameDirectory.h"
#include "Packages/DeusEx/UScriptedPawn.h"
#include "Packages/DeusExText/UDeusExTextParser.h"
#include "Packages/Emitter/UDistantLightActor.h"
#include "Packages/Emitter/UEmitterGarbageCollector.h"
#include "Packages/Emitter/UEmitterRC.h"
#include "Packages/Emitter/UKillParticleForce.h"
#include "Packages/Emitter/UParticleConcentrateForce.h"
#include "Packages/Emitter/UVelocityForce.h"
#include "Packages/Emitter/UXBeamEmitter.h"
#include "Packages/Emitter/UXEmitter.h"
#include "Packages/Emitter/UXMeshEmitter.h"
#include "Packages/Emitter/UXParticleEmitter.h"
#include "Packages/Emitter/UXParticleForces.h"
#include "Packages/Emitter/UXRainRestrictionVolume.h"
#include "Packages/Emitter/UXSpriteEmitter.h"
#include "Packages/Emitter/UXTrailEmitter.h"
#include "Packages/Emitter/UXTrailParticle.h"
#include "Packages/Emitter/UXWeatherEmitter.h"
#include "Packages/Engine/UCanvas.h"
#include "Packages/Engine/UClient.h"
#include "Packages/Engine/UConsole.h"
#include "Packages/Engine/UPlayer.h"
#include "Packages/Engine/URenderIterator.h"
#include "Packages/Engine/USurrealClient.h"
#include "Packages/Engine/UViewport.h"
#include "Packages/Engine/Actors/UActor.h"
#include "Packages/Engine/Actors/UAnimationNotify.h"
#include "Packages/Engine/Actors/UDecal.h"
#include "Packages/Engine/Actors/UHUD.h"
#include "Packages/Engine/Actors/ULight.h"
#include "Packages/Engine/Actors/UMenu.h"
#include "Packages/Engine/Actors/UProjectile.h"
#include "Packages/Engine/Actors/UProjector.h"
#include "Packages/Engine/Actors/USkeletalMeshInstance.h"
#include "Packages/Engine/Actors/USpawnNotify.h"
#include "Packages/Engine/Actors/Brush/UBrush.h"
#include "Packages/Engine/Actors/Brush/UMover.h"
#include "Packages/Engine/Actors/Decoration/UCarcass.h"
#include "Packages/Engine/Actors/Decoration/UDecoration.h"
#include "Packages/Engine/Actors/Info/UDynamicZoneInfo.h"
#include "Packages/Engine/Actors/Info/UGameInfo.h"
#include "Packages/Engine/Actors/Info/UGameReplicationInfo.h"
#include "Packages/Engine/Actors/Info/UInfo.h"
#include "Packages/Engine/Actors/Info/UInternetInfo.h"
#include "Packages/Engine/Actors/Info/ULevelInfo.h"
#include "Packages/Engine/Actors/Info/UMutator.h"
#include "Packages/Engine/Actors/Info/UPlayerReplicationInfo.h"
#include "Packages/Engine/Actors/Info/UReplicationInfo.h"
#include "Packages/Engine/Actors/Info/USavedMove.h"
#include "Packages/Engine/Actors/Info/USkyZoneInfo.h"
#include "Packages/Engine/Actors/Info/UStatLog.h"
#include "Packages/Engine/Actors/Info/UStatLogFile.h"
#include "Packages/Engine/Actors/Info/UWarpZoneInfo.h"
#include "Packages/Engine/Actors/Info/UZoneInfo.h"
#include "Packages/Engine/Actors/Inventory/UInventory.h"
#include "Packages/Engine/Actors/Inventory/UWeapon.h"
#include "Packages/Engine/Actors/InventoryAttachment/UInventoryAttachment.h"
#include "Packages/Engine/Actors/InventoryAttachment/UWeaponAttachment.h"
#include "Packages/Engine/Actors/InventoryAttachment/UWeaponMuzzleFlash.h"
#include "Packages/Engine/Actors/Keypoint/UInterpolationPoint.h"
#include "Packages/Engine/Actors/Keypoint/UKeypoint.h"
#include "Packages/Engine/Actors/Keypoint/Ulocationid.h"
#include "Packages/Engine/Actors/NavigationPoint/UButtonMarker.h"
#include "Packages/Engine/Actors/NavigationPoint/UInventorySpot.h"
#include "Packages/Engine/Actors/NavigationPoint/ULiftCenter.h"
#include "Packages/Engine/Actors/NavigationPoint/ULiftExit.h"
#include "Packages/Engine/Actors/NavigationPoint/UNavigationPoint.h"
#include "Packages/Engine/Actors/NavigationPoint/UPathNode.h"
#include "Packages/Engine/Actors/NavigationPoint/UPlayerStart.h"
#include "Packages/Engine/Actors/NavigationPoint/UTeleporter.h"
#include "Packages/Engine/Actors/NavigationPoint/UTriggerMarker.h"
#include "Packages/Engine/Actors/NavigationPoint/UWarpZoneMarker.h"
#include "Packages/Engine/Actors/Pawn/UCamera.h"
#include "Packages/Engine/Actors/Pawn/UPawn.h"
#include "Packages/Engine/Actors/Pawn/UPlayerPawn.h"
#include "Packages/Engine/Actors/Pawn/UScout.h"
#include "Packages/Engine/Actors/Triggers/UTrigger.h"
#include "Packages/Engine/Actors/Triggers/UTriggers.h"
#include "Packages/Engine/Network/UActorChannel.h"
#include "Packages/Engine/Network/UChannel.h"
#include "Packages/Engine/Network/UControlChannel.h"
#include "Packages/Engine/Network/UDemoPlayPendingLevel.h"
#include "Packages/Engine/Network/UDemoRecConnection.h"
#include "Packages/Engine/Network/UFileChannel.h"
#include "Packages/Engine/Network/UNetConnection.h"
#include "Packages/Engine/Network/UNetPendingLevel.h"
#include "Packages/Engine/Network/UPendingLevel.h"
#include "Packages/Engine/Resources/UFont.h"
#include "Packages/Engine/Resources/UMusic.h"
#include "Packages/Engine/Resources/UPalette.h"
#include "Packages/Engine/Resources/UPrimitive.h"
#include "Packages/Engine/Resources/USound.h"
#include "Packages/Engine/Resources/Level/UBspNodes.h"
#include "Packages/Engine/Resources/Level/UBspSurfs.h"
#include "Packages/Engine/Resources/Level/ULevel.h"
#include "Packages/Engine/Resources/Level/ULevelBase.h"
#include "Packages/Engine/Resources/Level/ULevelSummary.h"
#include "Packages/Engine/Resources/Level/UModel.h"
#include "Packages/Engine/Resources/Level/UPolys.h"
#include "Packages/Engine/Resources/Level/UVectors.h"
#include "Packages/Engine/Resources/Level/UVerts.h"
#include "Packages/Engine/Resources/Mesh/UAnimation.h"
#include "Packages/Engine/Resources/Mesh/ULodMesh.h"
#include "Packages/Engine/Resources/Mesh/UMesh.h"
#include "Packages/Engine/Resources/Mesh/USkeletalMesh.h"
#include "Packages/Engine/Resources/Mesh/UStaticMesh.h"
#include "Packages/Engine/Resources/Textures/UBitmap.h"
#include "Packages/Engine/Resources/Textures/UFireTexture.h"
#include "Packages/Engine/Resources/Textures/UFractalTexture.h"
#include "Packages/Engine/Resources/Textures/UIceTexture.h"
#include "Packages/Engine/Resources/Textures/UScriptedTexture.h"
#include "Packages/Engine/Resources/Textures/UTexture.h"
#include "Packages/Engine/Resources/Textures/UWaterTexture.h"
#include "Packages/Engine/Resources/Textures/UWaveTexture.h"
#include "Packages/Engine/Resources/Textures/UWetTexture.h"
#include "Packages/Engine/Subsystems/UAudioSubsystem.h"
#include "Packages/Engine/Subsystems/UEditorEngine.h"
#include "Packages/Engine/Subsystems/UEngine.h"
#include "Packages/Engine/Subsystems/UGameEngine.h"
#include "Packages/Engine/Subsystems/UNetDriver.h"
#include "Packages/Engine/Subsystems/URenderBase.h"
#include "Packages/Engine/Subsystems/URenderDevice.h"
#include "Packages/Engine/Subsystems/USurrealAudioDevice.h"
#include "Packages/Engine/Subsystems/USurrealNetworkDevice.h"
#include "Packages/Engine/Subsystems/USurrealRenderDevice.h"
#include "Packages/Extension/UExtensionObject.h"
#include "Packages/Extension/UExtString.h"
#include "Packages/Extension/UPlayerPawnExt.h"
#include "Packages/Extension/Flags/UFlag.h"
#include "Packages/Extension/Flags/UFlagBase.h"
#include "Packages/Extension/Flags/UFlagBool.h"
#include "Packages/Extension/Flags/UFlagByte.h"
#include "Packages/Extension/Flags/UFlagFloat.h"
#include "Packages/Extension/Flags/UFlagInt.h"
#include "Packages/Extension/Flags/UFlagName.h"
#include "Packages/Extension/Flags/UFlagRotator.h"
#include "Packages/Extension/Flags/UFlagVector.h"
#include "Packages/Extension/Windows/UBorderWindow.h"
#include "Packages/Extension/Windows/UComputerWindow.h"
#include "Packages/Extension/Windows/UGC.h"
#include "Packages/Extension/Windows/UListWindow.h"
#include "Packages/Extension/Windows/UScaleManagerWindow.h"
#include "Packages/Extension/Windows/UScaleWindow.h"
#include "Packages/Extension/Windows/UScrollAreaWindow.h"
#include "Packages/Extension/Windows/UTileWindow.h"
#include "Packages/Extension/Windows/UViewportWindow.h"
#include "Packages/Extension/Windows/UWindow.h"
#include "Packages/Extension/Windows/TabGroup/UClipWindow.h"
#include "Packages/Extension/Windows/TabGroup/UModalWindow.h"
#include "Packages/Extension/Windows/TabGroup/URadioBoxWindow.h"
#include "Packages/Extension/Windows/TabGroup/URootWindow.h"
#include "Packages/Extension/Windows/TabGroup/UTabGroupWindow.h"
#include "Packages/Extension/Windows/Text/UButtonWindow.h"
#include "Packages/Extension/Windows/Text/UCheckboxWindow.h"
#include "Packages/Extension/Windows/Text/UEditWindow.h"
#include "Packages/Extension/Windows/Text/ULargeTextWindow.h"
#include "Packages/Extension/Windows/Text/UTextLogWindow.h"
#include "Packages/Extension/Windows/Text/UTextWindow.h"
#include "Packages/Extension/Windows/Text/UToggleWindow.h"
#include "Packages/IpDrv/UInternetLink.h"
#include "Packages/IpDrv/UTcpLink.h"
#include "Packages/IpDrv/UUdpLink.h"
#include "Packages/RMusicPlayer/URMusic_Component.h"
#include "Packages/RMusicPlayer/URMusic_Player.h"
#include "Packages/UPak/UPathNodeIterator.h"
#include "Packages/UPak/UPawnPathNodeIterator.h"
#include "Native/NActor.h"
#include "Native/N227Emitter.h"
#include "Native/N227Projector.h"
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
#include "Native/NUPakPathNodeIterator.h"
#include "Native/NUPakPawnPathNodeIterator.h"
#include "Native/NBorderWindow.h"
#include "Native/NButtonWindow.h"
#include "Native/NCheckboxWindow.h"
#include "Native/NClipWindow.h"
#include "Native/NComputerWindow.h"
#include "Native/NConEvent.h"
#include "Native/NConEventRandomLabel.h"
#include "Native/NConversation.h"
#include "Native/NDeusExTextParser.h"
#include "Native/NEditWindow.h"
#include "Native/NExtensionObject.h"
#include "Native/NExtString.h"
#include "Native/NFlagBase.h"
#include "Native/NGC.h"
#include "Native/NLargeTextWindow.h"
#include "Native/NListWindow.h"
#include "Native/NModalWindow.h"
#include "Native/NRadioBoxWindow.h"
#include "Native/NRMusicPlayer.h"
#include "Native/NRootWindow.h"
#include "Native/NScaleManagerWindow.h"
#include "Native/NScaleWindow.h"
#include "Native/NScrollAreaWindow.h"
#include "Native/NTextLogWindow.h"
#include "Native/NTextWindow.h"
#include "Native/NTileWindow.h"
#include "Native/NTimeDemo.h"
#include "Native/NToggleWindow.h"
#include "Native/NViewportWindow.h"
#include "Native/NWebRequest.h"
#include "Native/NWebResponse.h"
#include "Native/NWindow.h"

PackageManager::PackageManager(const GameLaunchInfo& launchInfo) : launchInfo(launchInfo)
{
	gameRootFolderPath = fs::path(launchInfo.gameRootFolder);
	gameSystemFolderPath = gameRootFolderPath / "System";

	RegisterNativeClasses();
	CreateTransientPackage();
	RegisterFunctions();
	LoadEngineIniFiles();
	LoadFileExtensions();
	LoadIntFiles();
	LoadPackageRemaps();
	ScanPaths();
	ScanForMaps();

	if (launchInfo.IsDeusEx())
		ScanSaveInfos();

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
		if (StrTools::equals_ignore_case(mapName, mapFilename.string()))
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

void PackageManager::UnloadPackage(Package* package)
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
	auto fullPath = gameSaveFolderPath / fs::path(path);

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
	if (!IsRune() && launchInfo.ue1Version <= 219) // khg with 219 uses old format. unreal uses new in 226
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

	// Handle SavePath and CachePath
	const auto savePath = convert_path_separators(GetIniValue("System", "Core.System", "SavePath"));
	gameSaveFolderPath = (gameSystemFolderPath / savePath).lexically_normal();

	const auto cachePath = convert_path_separators(GetIniValue("System", "Core.System", "CachePath"));
	gameCacheFolderPath = (gameSystemFolderPath / cachePath).lexically_normal();

	// Unreal 227j+ and UT 469d+ store localization files in SystemLocalized folder
	if ((IsUnreal1_227() && launchInfo.gameSubVersion >= 10) || (IsUnrealTournament_469() && launchInfo.gameSubVersion >= 4))
	{
		auto langpaths = GetIniValues("System", "Core.System", "LangPaths");

		// Substitute <lang> with the actual language extension
		for (auto& langpath : langpaths)
		{
			auto pos = langpath.find("<lang>");
			while (pos != std::string::npos)
			{
				langpath.replace(pos, 6, languageExtension);
				pos = langpath.find("<lang>");
			}

			paths.push_back(langpath);
		}
	}

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
			if (dir_entry.is_regular_file() && NameString(dir_entry.path().extension().string()) == ".avi")
			{
				auto fileName = dir_entry.path().filename();
				aviFilenames[NameString(fileName.string())] = (gameSystemFolderPath / fileName).string();
			}
		}
	}
}

void PackageManager::ScanSaveInfos()
{
	for (const auto& entry : fs::directory_iterator{gameSaveFolderPath})
	{
		if (!entry.is_directory())
			continue;

		const auto save = entry.path() / ("SaveInfo." + saveExtension);

		if (!fs::exists(save) || !fs::is_regular_file(save))
			continue;

		auto saveFolderName = entry.path().filename().string();

		saveInfos[saveFolderName] = GC::Alloc<Package>(this, saveFolderName, save.string());
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

Package* PackageManager::GetSaveInfoPackage(const NameString& saveFolderName)
{
	auto idx = saveInfos.find(saveFolderName);

	if (idx != saveInfos.end())
		return idx->second;

	return nullptr;
}

void PackageManager::RemoveSaveInfoPackage(const NameString& saveFolderName)
{
	auto idx = saveInfos.find(saveFolderName);

	if (idx != saveInfos.end())
		saveInfos.erase(idx);
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
	s.Stream = std::make_shared<PackageStream>(package, File::open_existing(package->GetPackageFilePath()));
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
	if (IsKlingonHonorGuard() || launchInfo.ue1Version <= 219)
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
	if (IsKlingonHonorGuard() || launchInfo.ue1Version <= 219)
		return defaultIniFile->GetValue(sectionName, keyName, default_value, index);

	return defaultUserFile->GetValue(sectionName, keyName, default_value, index);
}

Array<std::string> PackageManager::GetDefUserIniValues(const NameString& sectionName, const NameString& keyName, Array<std::string> default_values)
{
	if (IsKlingonHonorGuard() || launchInfo.ue1Version <= 219)
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

	if (launchInfo.ue1Version > 219)
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

void PackageManager::LoadFileExtensions()
{
	const auto systemIni = GetSystemIniFile();

	mapExtension = systemIni->GetValue("URL", "MapExt", "unr");
	saveExtension = systemIni->GetValue("URL", "SaveExt", "usa");
	languageExtension = systemIni->GetValue("Engine.Engine", "Language", "int");
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
	if (IsUnreal1())
	{
		NUPakPathNodeIterator::RegisterFunctions();
		NUPakPawnPathNodeIterator::RegisterFunctions();
	}
	if (IsUnreal1_227())
	{
		NXParticleEmitter::RegisterFunctions();
		NXEmitter::RegisterFunctions();
		N227Projector::RegisterFunctions();
	}
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
		NBorderWindow::RegisterFunctions();
		NButtonWindow::RegisterFunctions();
		NCheckboxWindow::RegisterFunctions();
		NClipWindow::RegisterFunctions();
		NComputerWindow::RegisterFunctions();
		NConEvent::RegisterFunctions();
		NConEventRandomLabel::RegisterFunctions();
		NConversation::RegisterFunctions();
		NDeusExTextParser::RegisterFunctions();
		NEditWindow::RegisterFunctions();
		NExtensionObject::RegisterFunctions();
		NExtString::RegisterFunctions();
		NFlagBase::RegisterFunctions();
		NGC::RegisterFunctions();
		NLargeTextWindow::RegisterFunctions();
		NListWindow::RegisterFunctions();
		NModalWindow::RegisterFunctions();
		NRadioBoxWindow::RegisterFunctions();
		NRootWindow::RegisterFunctions();
		NScaleManagerWindow::RegisterFunctions();
		NScaleWindow::RegisterFunctions();
		NScrollAreaWindow::RegisterFunctions();
		NTextLogWindow::RegisterFunctions();
		NTextWindow::RegisterFunctions();
		NTileWindow::RegisterFunctions();
		NTimeDemo::RegisterFunctions();
		NToggleWindow::RegisterFunctions();
		NViewportWindow::RegisterFunctions();
		NWebRequest::RegisterFunctions();
		NWebResponse::RegisterFunctions();
		NWindow::RegisterFunctions();
	}

	if (fs::exists(gameSystemFolderPath / "RMusicPlayer.u"))
	{
		NRMusicPlayer::RegisterFunctions();
	}
}

void PackageManager::RegisterNativeClasses()
{
	NameString corePackage = "Core";
	NameString emitterPackage = "Emitter";
	NameString enginePackage = "Engine";
	NameString ipdrvPackage = "IpDrv";
	NameString upakPackage = "UPak";
	NameString extensionPackage = "Extension";
	NameString conSysPackage = "ConSys";
	NameString deusExPackage = "DeusEx";
	NameString deusExTextPackage = "DeusExText";
	NameString RMusicPlayerPackage = "RMusicPlayer";

	RegisterNativeClass<UObject>(corePackage, "Object");
	RegisterNativeClass<UPackage>(corePackage, "Package");
	RegisterNativeClass<UField>(corePackage, "Field", "Object");
	RegisterNativeClass<UConst>(corePackage, "Const", "Field");
	RegisterNativeClass<UEnum>(corePackage, "Enum", "Field");
	RegisterNativeClass<UStruct>(corePackage, "Struct", "Field");
	RegisterNativeClass<UFunction>(corePackage, "Function", "Struct");
	RegisterNativeClass<UState>(corePackage, "State", "Struct");
	RegisterNativeClass<UClass>(corePackage, "Class", "State");
	RegisterNativeClass<UAbstractProperty>(corePackage, "Property", "Field");
	RegisterNativeClass<UPointerProperty>(corePackage, "PointerProperty", "Property");
	RegisterNativeClass<UByteProperty>(corePackage, "ByteProperty", "Property");
	RegisterNativeClass<UObjectProperty>(corePackage, "ObjectProperty", "Property");
	RegisterNativeClass<UClassProperty>(corePackage, "ClassProperty", "ObjectProperty");
	RegisterNativeClass<UFixedArrayProperty>(corePackage, "FixedArrayProperty", "Property");
	RegisterNativeClass<UArrayProperty>(corePackage, "ArrayProperty", "Property");
	RegisterNativeClass<UMapProperty>(corePackage, "MapProperty", "Property");
	RegisterNativeClass<UStructProperty>(corePackage, "StructProperty", "Property");
	RegisterNativeClass<UIntProperty>(corePackage, "IntProperty", "Property");
	RegisterNativeClass<UBoolProperty>(corePackage, "BoolProperty", "Property");
	RegisterNativeClass<UFloatProperty>(corePackage, "FloatProperty", "Property");
	RegisterNativeClass<UNameProperty>(corePackage, "NameProperty", "Property");
	RegisterNativeClass<UStrProperty>(corePackage, "StrProperty", "Property");
	RegisterNativeClass<UStringProperty>(corePackage, "StringProperty", "Property");
	RegisterNativeClass<UTextBuffer>(corePackage, "TextBuffer", "Object");

	if (launchInfo.IsUnreal1_227())
	{
		RegisterNativeClass<UAnyProperty>(corePackage, "AnyProperty", "Property");
	}

	if (launchInfo.ue1Version < 400)
	{
		RegisterNativeClass<UCommandlet>(corePackage, "Commandlet", "Object");
		RegisterNativeClass<USimpleCommandlet>(corePackage, "SimpleCommandlet", "Commandlet");
		RegisterNativeClass<URenderIterator>(enginePackage, "RenderIterator", "Object");
	}

	RegisterNativeClass<USubsystem>(corePackage, "Subsystem", "Object");
	RegisterNativeClass<ULanguage>(corePackage, "Language", "Object");

	RegisterNativeClass<UEngine>(enginePackage, "Engine", "Subsystem");
	RegisterNativeClass<UGameEngine>(enginePackage, "GameEngine", "Engine");
	RegisterNativeClass<UEditorEngine>(enginePackage, "EditorEngine", "Engine");
	RegisterNativeClass<URenderBase>(enginePackage, "RenderBase", "Subsystem");
	RegisterNativeClass<URenderDevice>(enginePackage, "RenderDevice", "Subsystem");
	RegisterNativeClass<UAudioSubsystem>(enginePackage, "AudioSubsystem", "Subsystem");
	RegisterNativeClass<UNetDriver>(enginePackage, "NetDriver", "Subsystem");

	RegisterNativeClass<UPalette>(enginePackage, "Palette", "Object");
	RegisterNativeClass<USound>(enginePackage, "Sound", "Object");
	RegisterNativeClass<UMusic>(enginePackage, "Music", "Object");

	RegisterNativeClass<UPrimitive>(enginePackage, "Primitive", "Object");
	RegisterNativeClass<UMesh>(enginePackage, "Mesh", "Primitive");
	RegisterNativeClass<ULodMesh>(enginePackage, "LodMesh", "Mesh");
	RegisterNativeClass<USkeletalMesh>(enginePackage, "SkeletalMesh", "LodMesh");
	RegisterNativeClass<UAnimation>(enginePackage, "Animation", "Object");

	RegisterNativeClass<UModel>(enginePackage, "Model", "Primitive");
	RegisterNativeClass<ULevelBase>(enginePackage, "LevelBase", "Object");
	RegisterNativeClass<ULevel>(enginePackage, "Level", "LevelBase");
	RegisterNativeClass<ULevelSummary>(enginePackage, "LevelSummary", "Object");
	RegisterNativeClass<UPolys>(enginePackage, "Polys", "Object");
	RegisterNativeClass<UBspNodes>(enginePackage, "BspNodes", "Object");
	RegisterNativeClass<UBspSurfs>(enginePackage, "BspSurfs", "Object");
	RegisterNativeClass<UVectors>(enginePackage, "Vectors", "Object");
	RegisterNativeClass<UVerts>(enginePackage, "Verts", "Object");

	RegisterNativeClass<UBitmap>(enginePackage, "Bitmap", "Object");
	RegisterNativeClass<UTexture>(enginePackage, "Texture", "Bitmap");
	RegisterNativeClass<UFractalTexture>(enginePackage, "FractalTexture", "Texture");
	RegisterNativeClass<UFireTexture>(enginePackage, "FireTexture", "FractalTexture");
	RegisterNativeClass<UIceTexture>(enginePackage, "IceTexture", "FractalTexture");
	RegisterNativeClass<UWaterTexture>(enginePackage, "WaterTexture", "FractalTexture");
	RegisterNativeClass<UWaveTexture>(enginePackage, "WaveTexture", "WaterTexture");
	RegisterNativeClass<UWetTexture>(enginePackage, "WetTexture", "WaterTexture");
	RegisterNativeClass<UScriptedTexture>(enginePackage, "ScriptedTexture", "Texture");

	if (launchInfo.ue1Version <= 220)
		RegisterNativeClass<UFont>(enginePackage, "Font", "Texture");
	else
		RegisterNativeClass<UFont>(enginePackage, "Font", "Object");

	RegisterNativeClass<UClient>(enginePackage, "Client", "Object");
	RegisterNativeClass<UViewport>(enginePackage, "Viewport", "Player");
	RegisterNativeClass<UCanvas>(enginePackage, "Canvas", "Object");
	RegisterNativeClass<UConsole>(enginePackage, "Console", "Object");
	RegisterNativeClass<UPlayer>(enginePackage, "Player", "Object");
	RegisterNativeClass<UNetConnection>(enginePackage, "NetConnection", "Player");
	RegisterNativeClass<UDemoRecConnection>(enginePackage, "DemoRecConnection", "NetConnection");
	RegisterNativeClass<UPendingLevel>(enginePackage, "PendingLevel", "Object");
	RegisterNativeClass<UNetPendingLevel>(enginePackage, "NetPendingLevel", "PendingLevel");
	RegisterNativeClass<UDemoPlayPendingLevel>(enginePackage, "DemoPlayPendingLevel", "PendingLevel");
	RegisterNativeClass<UChannel>(enginePackage, "Channel", "Object");
	RegisterNativeClass<UControlChannel>(enginePackage, "ControlChannel", "Channel");
	RegisterNativeClass<UActorChannel>(enginePackage, "ActorChannel", "Channel");
	RegisterNativeClass<UFileChannel>(enginePackage, "FileChannel", "Channel");

	RegisterNativeClass<USurrealRenderDevice>(enginePackage, "SurrealRenderDevice", "RenderDevice");
	RegisterNativeClass<USurrealAudioDevice>(enginePackage, "SurrealAudioDevice", "AudioSubsystem");
	RegisterNativeClass<USurrealNetworkDevice>(enginePackage, "SurrealNetworkDevice", "NetDriver");
	RegisterNativeClass<USurrealClient>(enginePackage, "SurrealClient", "Client");

	RegisterNativeClass<UActor>(enginePackage, "Actor", "Object");
	RegisterNativeClass<ULight>(enginePackage, "Light", "Actor");
	RegisterNativeClass<UInventory>(enginePackage, "Inventory", "Actor");
	RegisterNativeClass<UWeapon>(enginePackage, "Weapon", "Inventory");
	RegisterNativeClass<UNavigationPoint>(enginePackage, "NavigationPoint", "Actor");
	RegisterNativeClass<ULiftExit>(enginePackage, "LiftExit", "NavigationPoint");
	RegisterNativeClass<ULiftCenter>(enginePackage, "LiftCenter", "NavigationPoint");
	RegisterNativeClass<UWarpZoneMarker>(enginePackage, "WarpZoneMarker", "NavigationPoint");
	RegisterNativeClass<UInventorySpot>(enginePackage, "InventorySpot", "NavigationPoint");
	RegisterNativeClass<UTriggerMarker>(enginePackage, "TriggerMarker", "NavigationPoint");
	RegisterNativeClass<UButtonMarker>(enginePackage, "ButtonMarker", "NavigationPoint");
	RegisterNativeClass<UPlayerStart>(enginePackage, "PlayerStart", "NavigationPoint");
	RegisterNativeClass<UTeleporter>(enginePackage, "Teleporter", "NavigationPoint");
	RegisterNativeClass<UPathNode>(enginePackage, "PathNode", "NavigationPoint");
	RegisterNativeClass<UDecoration>(enginePackage, "Decoration", "Actor");
	RegisterNativeClass<UCarcass>(enginePackage, "Carcass", "Decoration");
	RegisterNativeClass<UProjectile>(enginePackage, "Projectile", "Actor");
	RegisterNativeClass<UKeypoint>(enginePackage, "Keypoint", "Actor");
	RegisterNativeClass<Ulocationid>(enginePackage, "locationid", "Keypoint");
	RegisterNativeClass<UInterpolationPoint>(enginePackage, "InterpolationPoint", "Keypoint");
	RegisterNativeClass<UTriggers>(enginePackage, "Triggers", "Actor");
	RegisterNativeClass<UTrigger>(enginePackage, "Trigger", "Triggers");
	RegisterNativeClass<UHUD>(enginePackage, "HUD", "Actor");
	RegisterNativeClass<UMenu>(enginePackage, "Menu", "Actor");
	RegisterNativeClass<UInfo>(enginePackage, "Info", "Actor");
	RegisterNativeClass<UMutator>(enginePackage, "Mutator", "Info");
	RegisterNativeClass<UGameInfo>(enginePackage, "GameInfo", "Info");
	RegisterNativeClass<UZoneInfo>(enginePackage, "ZoneInfo", "Info");
	RegisterNativeClass<ULevelInfo>(enginePackage, "LevelInfo", "ZoneInfo");
	RegisterNativeClass<UWarpZoneInfo>(enginePackage, "WarpZoneInfo", "ZoneInfo");
	RegisterNativeClass<USkyZoneInfo>(enginePackage, "SkyZoneInfo", "ZoneInfo");
	RegisterNativeClass<USavedMove>(enginePackage, "SavedMove", "Info");
	RegisterNativeClass<UReplicationInfo>(enginePackage, "ReplicationInfo", "Info");
	RegisterNativeClass<UPlayerReplicationInfo>(enginePackage, "PlayerReplicationInfo", "ReplicationInfo");
	RegisterNativeClass<UGameReplicationInfo>(enginePackage, "GameReplicationInfo", "ReplicationInfo");
	RegisterNativeClass<UInternetInfo>(enginePackage, "InternetInfo", "Info");
	RegisterNativeClass<UStatLog>(enginePackage, "StatLog", "Info");
	RegisterNativeClass<UStatLogFile>(enginePackage, "StatLogFile", "StatLog");
	RegisterNativeClass<UDecal>(enginePackage, "Decal", "Actor");
	RegisterNativeClass<USpawnNotify>(enginePackage, "SpawnNotify", "Actor");
	RegisterNativeClass<UBrush>(enginePackage, "Brush", "Actor");
	RegisterNativeClass<UMover>(enginePackage, "Mover", "Brush");
	RegisterNativeClass<UPawn>(enginePackage, "Pawn", "Actor");
	RegisterNativeClass<UScout>(enginePackage, "Scout", "Pawn");
	RegisterNativeClass<UPlayerPawn>(enginePackage, "PlayerPawn", "Pawn");
	RegisterNativeClass<UCamera>(enginePackage, "Camera", "PlayerPawn");

	RegisterNativeClass<UInternetLink>(ipdrvPackage, "InternetLink", "InternetInfo");
	RegisterNativeClass<UTcpLink>(ipdrvPackage, "TcpLink", "InternetLink");
	RegisterNativeClass<UUdpLink>(ipdrvPackage, "UdpLink", "InternetLink");

	if (IsUnreal1())
	{
		RegisterNativeClass<UPakPathNodeIterator>(upakPackage, "PathNodeIterator", "Actor");
		RegisterNativeClass<UPakPawnPathNodeIterator>(upakPackage, "PawnPathNodeIterator", "PathNodeIterator");
	}

	if (IsUnreal1_227())
	{
		RegisterNativeClass<U227SkeletalMeshInstance>(enginePackage, "SkeletalMeshInstance", "Object");
		RegisterNativeClass<U227AnimationNotify>(enginePackage, "AnimationNotify", "Object");
		RegisterNativeClass<UInventoryAttachment>(enginePackage, "InventoryAttachment", "Actor");
		RegisterNativeClass<UWeaponAttachment>(enginePackage, "WeaponAttachment", "InventoryAttachment");
		RegisterNativeClass<UWeaponMuzzleFlash>(enginePackage, "WeaponMuzzleFlash", "InventoryAttachment");
		RegisterNativeClass<U227Projector>(enginePackage, "Projector", "Actor");
		RegisterNativeClass<UDynamicZoneInfo>(enginePackage, "DynamicZoneInfo", "ZoneInfo");
		RegisterNativeClass<UDistantLightActor>(emitterPackage, "DistantLightActor", "Light");
		RegisterNativeClass<UStaticMesh>(enginePackage, "StaticMesh", "Mesh");
		RegisterNativeClass<UEmitterRC>(emitterPackage, "EmitterRC", "Actor");
		RegisterNativeClass<UXParticleEmitter>(emitterPackage, "XParticleEmitter", "Actor");
		RegisterNativeClass<UXParticleForces>(emitterPackage, "XParticleForces", "Actor");
		RegisterNativeClass<UVelocityForce>(emitterPackage, "VelocityForce", "XParticleForces");
		RegisterNativeClass<UKillParticleForce>(emitterPackage, "KillParticleForce", "XParticleForces");
		RegisterNativeClass<UParticleConcentrateForce>(emitterPackage, "ParticleConcentrateForce", "XParticleForces");
		RegisterNativeClass<UXEmitter>(emitterPackage, "XEmitter", "XParticleEmitter");
		RegisterNativeClass<UEmitterGarbageCollector>(emitterPackage, "EmitterGarbageCollector", "Info");
		RegisterNativeClass<UXTrailParticle>(emitterPackage, "XTrailParticle", "Actor");
		RegisterNativeClass<UXBeamEmitter>(emitterPackage, "XBeamEmitter", "XEmitter");
		RegisterNativeClass<UXMeshEmitter>(emitterPackage, "XMeshEmitter", "XEmitter");
		RegisterNativeClass<UXSpriteEmitter>(emitterPackage, "XSpriteEmitter", "XEmitter");
		RegisterNativeClass<UXWeatherEmitter>(emitterPackage, "XWeatherEmitter", "XParticleEmitter");
		RegisterNativeClass<UXTrailEmitter>(emitterPackage, "XTrailEmitter", "XEmitter");
		RegisterNativeClass<UXRainRestrictionVolume>(emitterPackage, "XRainRestrictionVolume", "Actor");
	}
	
	if (IsDeusEx())
	{
		RegisterNativeClass<UPlayerPawnExt>(extensionPackage, "PlayerPawnExt", "PlayerPawn");
		RegisterNativeClass<UExtensionObject>(extensionPackage, "ExtensionObject", "Object");
		RegisterNativeClass<UFlagBase>(extensionPackage, "FlagBase", "ExtensionObject");
		RegisterNativeClass<UFlag>(extensionPackage, "Flag", "ExtensionObject");
		RegisterNativeClass<UFlagBool>(extensionPackage, "FlagBool", "Flag");
		RegisterNativeClass<UFlagByte>(extensionPackage, "FlagByte", "Flag");
		RegisterNativeClass<UFlagFloat>(extensionPackage, "FlagFloat", "Flag");
		RegisterNativeClass<UFlagInt>(extensionPackage, "FlagInt", "Flag");
		RegisterNativeClass<UFlagName>(extensionPackage, "FlagName", "Flag");
		RegisterNativeClass<UFlagRotator>(extensionPackage, "FlagRotator", "Flag");
		RegisterNativeClass<UFlagVector>(extensionPackage, "FlagVector", "Flag");
		RegisterNativeClass<UDXExtString>(extensionPackage, "ExtString", "Object");
		RegisterNativeClass<UDXGameDirectory>(deusExPackage, "GameDirectory", "Object");
		RegisterNativeClass<UDeusExLevelInfo>(deusExPackage, "DeusExLevelInfo", "Info");
		RegisterNativeClass<UDXTextParser>(deusExTextPackage, "DeusExTextParser", "Object");
		RegisterNativeClass<UDXSaveInfo>(deusExPackage, "DeusExSaveInfo", "Object");
		RegisterNativeClass<UGC>(extensionPackage, "GC", "ExtensionObject");
		RegisterNativeClass<UWindow>(extensionPackage, "Window", "ExtensionObject");
		RegisterNativeClass<UViewportWindow>(extensionPackage, "ViewportWindow", "Window");
		RegisterNativeClass<UToggleWindow>(extensionPackage, "ToggleWindow", "ButtonWindow");
		RegisterNativeClass<UCheckboxWindow>(extensionPackage, "CheckboxWindow", "ToggleWindow");
		RegisterNativeClass<UTileWindow>(extensionPackage, "TileWindow", "Window");
		RegisterNativeClass<UTextWindow>(extensionPackage, "TextWindow", "Window");
		RegisterNativeClass<UButtonWindow>(extensionPackage, "ButtonWindow", "TextWindow");
		RegisterNativeClass<UTextLogWindow>(extensionPackage, "TextLogWindow", "TextWindow");
		RegisterNativeClass<ULargeTextWindow>(extensionPackage, "LargeTextWindow", "TextWindow");
		RegisterNativeClass<UEditWindow>(extensionPackage, "EditWindow", "LargeTextWindow");
		RegisterNativeClass<UTabGroupWindow>(extensionPackage, "TabGroupWindow", "Window");
		RegisterNativeClass<UModalWindow>(extensionPackage, "ModalWindow", "TabGroupWindow");
		RegisterNativeClass<URootWindow>(extensionPackage, "RootWindow", "ModalWindow");
		RegisterNativeClass<URadioBoxWindow>(extensionPackage, "RadioBoxWindow", "TabGroupWindow");
		RegisterNativeClass<UClipWindow>(extensionPackage, "ClipWindow", "TabGroupWindow");
		RegisterNativeClass<UScrollAreaWindow>(extensionPackage, "ScrollAreaWindow", "Window");
		RegisterNativeClass<UScaleWindow>(extensionPackage, "ScaleWindow", "Window");
		RegisterNativeClass<UScaleManagerWindow>(extensionPackage, "ScaleManagerWindow", "Window");
		RegisterNativeClass<UListWindow>(extensionPackage, "ListWindow", "Window");
		RegisterNativeClass<UComputerWindow>(extensionPackage, "ComputerWindow", "Window");
		RegisterNativeClass<UBorderWindow>(extensionPackage, "BorderWindow", "Window");
		RegisterNativeClass<UDeusExPlayer>(deusExPackage, "DeusExPlayer", "PlayerPawnExt");
		RegisterNativeClass<UScriptedPawn>(deusExPackage, "ScriptedPawn", "Pawn");
		RegisterNativeClass<UDeusExDecoration>(deusExPackage, "DeusExDecoration", "Decoration");
		RegisterNativeClass<UConLight>(conSysPackage, "ConLight", "Light");
		RegisterNativeClass<UConObject>(conSysPackage, "ConObject", "Object");
		RegisterNativeClass<UConAudioList>(conSysPackage, "ConAudioList", "ConObject");
		RegisterNativeClass<UConCamera>(conSysPackage, "ConCamera", "ConObject");
		RegisterNativeClass<UConChoice>(conSysPackage, "ConChoice", "ConObject");
		RegisterNativeClass<UConEvent>(conSysPackage, "ConEvent", "ConObject");
		RegisterNativeClass<UConEventAddCredits>(conSysPackage, "ConEventAddCredits", "ConEvent");
		RegisterNativeClass<UConEventAddGoal>(conSysPackage, "ConEventAddGoal", "ConEvent");
		RegisterNativeClass<UConEventAddNote>(conSysPackage, "ConEventAddNote", "ConEvent");
		RegisterNativeClass<UConEventAddSkillPoints>(conSysPackage, "ConEventAddSkillPoints", "ConEvent");
		RegisterNativeClass<UConEventAnimation>(conSysPackage, "ConEventAnimation", "ConEvent");
		RegisterNativeClass<UConEventCheckFlag>(conSysPackage, "ConEventCheckFlag", "ConEvent");
		RegisterNativeClass<UConEventCheckObject>(conSysPackage, "ConEventCheckObject", "ConEvent");
		RegisterNativeClass<UConEventCheckPersona>(conSysPackage, "ConEventCheckPersona", "ConEvent");
		RegisterNativeClass<UConEventChoice>(conSysPackage, "ConEventChoice", "ConEvent");
		RegisterNativeClass<UConEventComment>(conSysPackage, "ConEventComment", "ConEvent");
		RegisterNativeClass<UConEventEnd>(conSysPackage, "ConEventEnd", "ConEvent");
		RegisterNativeClass<UConEventJump>(conSysPackage, "ConEventJump", "ConEvent");
		RegisterNativeClass<UConEventMoveCamera>(conSysPackage, "ConEventMoveCamera", "ConEvent");
		RegisterNativeClass<UConEventRandomLabel>(conSysPackage, "ConEventRandomLabel", "ConEvent");
		RegisterNativeClass<UConEventSetFlag>(conSysPackage, "ConEventSetFlag", "ConEvent");
		RegisterNativeClass<UConEventSpeech>(conSysPackage, "ConEventSpeech", "ConEvent");
		RegisterNativeClass<UConEventTrade>(conSysPackage, "ConEventTrade", "ConEvent");
		RegisterNativeClass<UConEventTransferObject>(conSysPackage, "ConEventTransferObject", "ConEvent");
		RegisterNativeClass<UConEventTrigger>(conSysPackage, "ConEventTrigger", "ConEvent");
		RegisterNativeClass<UConFlagRef>(conSysPackage, "ConFlagRef", "ConObject");
		RegisterNativeClass<UConHistory>(conSysPackage, "ConHistory", "ConObject");
		RegisterNativeClass<UConHistoryEvent>(conSysPackage, "ConHistoryEvent", "ConObject");
		RegisterNativeClass<UConItem>(conSysPackage, "ConItem", "ConObject");
		RegisterNativeClass<UConListItem>(conSysPackage, "ConListItem", "ConObject");
		RegisterNativeClass<UConSpeech>(conSysPackage, "ConSpeech", "ConObject");
		RegisterNativeClass<UConversation>(conSysPackage, "Conversation", "ConObject");
		RegisterNativeClass<UConversationList>(conSysPackage, "ConversationList", "ConObject");
		RegisterNativeClass<UConversationMissionList>(conSysPackage, "ConversationMissionList", "ConObject");
	}

	if (fs::exists(gameSystemFolderPath / "RMusicPlayer.u"))
	{
		RegisterNativeClass<URMusic_Component>(RMusicPlayerPackage, "RMusic_Component", "Actor");
		RegisterNativeClass<URMusic_Player>(RMusicPlayerPackage, "RMusic_Player", "RMusic_Component");
	}
}

template<typename T>
void PackageManager::RegisterNativeClass(const NameString& packageName, const NameString& className, const NameString& baseClass)
{
	NativeClass nativeClass;
	nativeClass.Package = packageName;
	nativeClass.Name = className;
	nativeClass.Base = baseClass;
	nativeClass.CreateFunc = [](const NameString& name, UClass* cls, ObjectFlags flags) -> UObject* { return GC::Alloc<T>(name, cls, flags); };
	NativeClasses[className] = std::move(nativeClass);
}
