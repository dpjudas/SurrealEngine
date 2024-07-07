
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
	Exception::Throw("DeusExPlayer.ConBindEvents not implemented");
}

void NDeusExPlayer::CreateDataVaultImageNoteObject(UObject* Self, UObject*& ReturnValue)
{
	Exception::Throw("DeusExPlayer.CreateDataVaultImageNoteObject not implemented");
}

void NDeusExPlayer::CreateDumpLocationObject(UObject* Self, UObject*& ReturnValue)
{
	LogUnimplemented("DeusExPlayer.CreateDumpLocationObject");
	ReturnValue = nullptr;
}

void NDeusExPlayer::CreateGameDirectoryObject(UObject* Self, UObject*& ReturnValue)
{
	Exception::Throw("DeusExPlayer.CreateGameDirectoryObject not implemented");
}

void NDeusExPlayer::CreateHistoryEvent(UObject* Self, UObject*& ReturnValue)
{
	Exception::Throw("DeusExPlayer.CreateHistoryEvent not implemented");
}

void NDeusExPlayer::CreateHistoryObject(UObject* Self, UObject*& ReturnValue)
{
	Exception::Throw("DeusExPlayer.CreateHistoryObject not implemented");
}

void NDeusExPlayer::CreateLogObject(UObject* Self, UObject*& ReturnValue)
{
	Exception::Throw("DeusExPlayer.CreateLogObject not implemented");
}

void NDeusExPlayer::DeleteSaveGameFiles(UObject* Self, std::string* saveDirectory)
{
	Exception::Throw("DeusExPlayer.DeleteSaveGameFiles not implemented");
}

void NDeusExPlayer::GetDeusExVersion(UObject* Self, std::string& ReturnValue)
{
	Exception::Throw("DeusExPlayer.GetDeusExVersion not implemented");
}

void NDeusExPlayer::SaveGame(UObject* Self, int saveIndex, std::string* saveDesc)
{
	Exception::Throw("DeusExPlayer.SaveGame not implemented");
}

void NDeusExPlayer::SetBoolFlagFromString(UObject* Self, const std::string& flagNameString, bool bValue, NameString& ReturnValue)
{
	Exception::Throw("DeusExPlayer.SetBoolFlagFromString not implemented");
}

void NDeusExPlayer::UnloadTexture(UObject* Self, UObject* Texture)
{
	Exception::Throw("DeusExPlayer.UnloadTexture not implemented");
}
