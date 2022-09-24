
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
	throw std::runtime_error("DeusExPlayer.ConBindEvents not implemented");
}

void NDeusExPlayer::CreateDataVaultImageNoteObject(UObject* Self, UObject*& ReturnValue)
{
	throw std::runtime_error("DeusExPlayer.CreateDataVaultImageNoteObject not implemented");
}

void NDeusExPlayer::CreateDumpLocationObject(UObject* Self, UObject*& ReturnValue)
{
	throw std::runtime_error("DeusExPlayer.CreateDumpLocationObject not implemented");
}

void NDeusExPlayer::CreateGameDirectoryObject(UObject* Self, UObject*& ReturnValue)
{
	throw std::runtime_error("DeusExPlayer.CreateGameDirectoryObject not implemented");
}

void NDeusExPlayer::CreateHistoryEvent(UObject* Self, UObject*& ReturnValue)
{
	throw std::runtime_error("DeusExPlayer.CreateHistoryEvent not implemented");
}

void NDeusExPlayer::CreateHistoryObject(UObject* Self, UObject*& ReturnValue)
{
	throw std::runtime_error("DeusExPlayer.CreateHistoryObject not implemented");
}

void NDeusExPlayer::CreateLogObject(UObject* Self, UObject*& ReturnValue)
{
	throw std::runtime_error("DeusExPlayer.CreateLogObject not implemented");
}

void NDeusExPlayer::DeleteSaveGameFiles(UObject* Self, std::string* saveDirectory)
{
	throw std::runtime_error("DeusExPlayer.DeleteSaveGameFiles not implemented");
}

void NDeusExPlayer::GetDeusExVersion(UObject* Self, std::string& ReturnValue)
{
	throw std::runtime_error("DeusExPlayer.GetDeusExVersion not implemented");
}

void NDeusExPlayer::SaveGame(UObject* Self, int saveIndex, std::string* saveDesc)
{
	throw std::runtime_error("DeusExPlayer.SaveGame not implemented");
}

void NDeusExPlayer::SetBoolFlagFromString(UObject* Self, const std::string& flagNameString, bool bValue, NameString& ReturnValue)
{
	throw std::runtime_error("DeusExPlayer.SetBoolFlagFromString not implemented");
}

void NDeusExPlayer::UnloadTexture(UObject* Self, UObject* Texture)
{
	throw std::runtime_error("DeusExPlayer.UnloadTexture not implemented");
}
