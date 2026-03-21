
#include "Precomp.h"
#include "NDeusExPlayer.h"
#include "VM/NativeFunc.h"
#include "UObject/UActor.h"
#include "Engine.h"

void NDeusExPlayer::RegisterFunctions()
{
	RegisterVMNativeFunc_0("DeusExPlayer", "ConBindEvents", &NDeusExPlayer::ConBindEvents, 2100);
	RegisterVMNativeFunc_1("DeusExPlayer", "CreateDataVaultImageNoteObject", &NDeusExPlayer::CreateDataVaultImageNoteObject, 3014);
	RegisterVMNativeFunc_1("DeusExPlayer", "CreateDumpLocationObject", &NDeusExPlayer::CreateDumpLocationObject, 3015);
	RegisterVMNativeFunc_1("DeusExPlayer", "CreateGameDirectoryObject", &NDeusExPlayer::CreateGameDirectoryObject, 3013);
	RegisterVMNativeFunc_1("DeusExPlayer", "CreateHistoryEvent", &NDeusExPlayer::CreateHistoryEvent, 3003);
	RegisterVMNativeFunc_1("DeusExPlayer", "CreateHistoryObject", &NDeusExPlayer::CreateHistoryObject, 3002);
	RegisterVMNativeFunc_1("DeusExPlayer", "CreateLogObject", &NDeusExPlayer::CreateLogObject, 3010);
	RegisterVMNativeFunc_1("DeusExPlayer", "DeleteSaveGameFiles", &NDeusExPlayer::DeleteSaveGameFiles, 3012);
	RegisterVMNativeFunc_1("DeusExPlayer", "GetDeusExVersion", &NDeusExPlayer::GetDeusExVersion, 1099);
	RegisterVMNativeFunc_2("DeusExPlayer", "SaveGame", &NDeusExPlayer::SaveGame, 3011);
	RegisterVMNativeFunc_3("DeusExPlayer", "SetBoolFlagFromString", &NDeusExPlayer::SetBoolFlagFromString, 3001);
	RegisterVMNativeFunc_1("DeusExPlayer", "UnloadTexture", &NDeusExPlayer::UnloadTexture, 3016);
}

void NDeusExPlayer::ConBindEvents(UObject* Self)
{
	auto PlayerSelf = UObject::Cast<UDeusExPlayer>(Self);
	PlayerSelf->ConBindEvents();
}

void NDeusExPlayer::CreateDataVaultImageNoteObject(UObject* Self, UObject*& ReturnValue)
{
	auto PlayerSelf = UObject::Cast<UDeusExPlayer>(Self);
	ReturnValue = PlayerSelf->CreateDataVaultImageNoteObject();
}

void NDeusExPlayer::CreateDumpLocationObject(UObject* Self, UObject*& ReturnValue)
{
	auto PlayerSelf = UObject::Cast<UDeusExPlayer>(Self);
	ReturnValue = PlayerSelf->CreateDumpLocationObject();
}

void NDeusExPlayer::CreateGameDirectoryObject(UObject* Self, UObject*& ReturnValue)
{
	auto PlayerSelf = UObject::Cast<UDeusExPlayer>(Self);
	ReturnValue = PlayerSelf->CreateGameDirectoryObject();
}

void NDeusExPlayer::CreateHistoryEvent(UObject* Self, UObject*& ReturnValue)
{
	auto PlayerSelf = UObject::Cast<UDeusExPlayer>(Self);
	ReturnValue = PlayerSelf->CreateHistoryEvent();
}

void NDeusExPlayer::CreateHistoryObject(UObject* Self, UObject*& ReturnValue)
{
	auto PlayerSelf = UObject::Cast<UDeusExPlayer>(Self);
	ReturnValue = PlayerSelf->CreateHistoryObject();
}

void NDeusExPlayer::CreateLogObject(UObject* Self, UObject*& ReturnValue)
{
	auto PlayerSelf = UObject::Cast<UDeusExPlayer>(Self);
	ReturnValue = PlayerSelf->CreateLogObject();
}

void NDeusExPlayer::DeleteSaveGameFiles(UObject* Self, std::optional<std::string> saveDirectory)
{
	auto PlayerSelf = UObject::Cast<UDeusExPlayer>(Self);
	PlayerSelf->DeleteSaveGameFiles(saveDirectory);
}

void NDeusExPlayer::GetDeusExVersion(UObject* Self, std::string& ReturnValue)
{
	auto PlayerSelf = UObject::Cast<UDeusExPlayer>(Self);
	ReturnValue = PlayerSelf->GetDeusExVersion();
}

void NDeusExPlayer::SaveGame(UObject* Self, int saveIndex, std::optional<std::string> saveDesc)
{
	auto PlayerSelf = UObject::Cast<UDeusExPlayer>(Self);
	PlayerSelf->SaveGame(saveIndex, saveDesc);
}

void NDeusExPlayer::SetBoolFlagFromString(UObject* Self, const std::string& flagNameString, bool bValue, NameString& ReturnValue)
{
	auto PlayerSelf = UObject::Cast<UDeusExPlayer>(Self);
	ReturnValue = PlayerSelf->SetBoolFlagFromString(flagNameString, bValue);
}

void NDeusExPlayer::UnloadTexture(UObject* Self, UObject* Texture)
{
	auto PlayerSelf = UObject::Cast<UDeusExPlayer>(Self);
	PlayerSelf->UnloadTexture(Texture);
}
