
#include "Precomp.h"
#include "UDeusExPlayer.h"
#include "Engine.h"
#include "Package/PackageManager.h"
#include "Packages/DeusEx/UGameDirectory.h"
#include "Packages/ConSys/History/UConHistory.h"
#include "Utils/Logger.h"

void UDeusExPlayer::ConBindEvents()
{
	DeusExConBindEvents();
}

UObject* UDeusExPlayer::CreateDataVaultImageNoteObject()
{
	auto cls = engine->packages->FindClass("DeusEx.DataVaultImageNote");
	return engine->packages->GetTransientPackage()->NewObject("DataVaultImageNote", cls, ObjectFlags::Transient);
}

UObject* UDeusExPlayer::CreateDumpLocationObject()
{
	auto cls = engine->packages->FindClass("DeusEx.DumpLocation");
	return engine->packages->GetTransientPackage()->NewObject("DumpLocation", cls, ObjectFlags::Transient);
}

UObject* UDeusExPlayer::CreateGameDirectoryObject()
{
	if (!m_GameDirectory)
	{
		auto cls = engine->packages->FindClass("DeusEx.GameDirectory");
		m_GameDirectory = Cast<UDXGameDirectory>(engine->packages->GetTransientPackage()->NewObject("GameDirectory", cls, ObjectFlags::Transient));
	}

	return m_GameDirectory;
}

UObject* UDeusExPlayer::CreateHistoryEvent()
{
	auto cls = engine->packages->FindClass("ConSys.ConHistoryEvent");
	return engine->packages->GetTransientPackage()->NewObject("ConHistoryEvent", cls, ObjectFlags::Transient);
}

UObject* UDeusExPlayer::CreateHistoryObject()
{
	auto cls = engine->packages->FindClass("ConSys.ConHistory");
	return Cast<UConHistory>(engine->packages->GetTransientPackage()->NewObject("ConHistory", cls, ObjectFlags::Transient));
}

UObject* UDeusExPlayer::CreateLogObject()
{
	auto cls = engine->packages->FindClass("DeusEx.DeusExLog");
	return engine->packages->GetTransientPackage()->NewObject("DeusExLog", cls, ObjectFlags::Transient);
}

void UDeusExPlayer::DeleteSaveGameFiles(std::optional<std::string> saveDirectory)
{
	LogUnimplemented("DeusExPlayer.DeleteSaveGameFiles");
}

std::string UDeusExPlayer::GetDeusExVersion()
{
	return "1.112fm. Surreal Engine Edition!";
}

void UDeusExPlayer::SaveGame(int saveIndex, std::optional<std::string> saveDesc)
{
	engine->SaveGameInfo.SaveGameSlot = saveIndex;
	engine->SaveGameInfo.SaveGameDescription = *saveDesc;
}

NameString UDeusExPlayer::SetBoolFlagFromString(const std::string& flagNameString, bool bValue)
{
	// Not called directly from script
	LogUnimplemented("DeusExPlayer.SetBoolFlagFromString");
	return {};
}

void UDeusExPlayer::UnloadTexture(UObject* Texture)
{
	// Nothing going on here because SE never unloads textures atm. This is just here so it doesn't throw LogUnimplemented.
}
