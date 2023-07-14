#pragma once

#include "UObject/UObject.h"

class NDeusExPlayer
{
public:
	static void RegisterFunctions();

	static void ConBindEvents(UObject* Self);
	static void CreateDataVaultImageNoteObject(UObject* Self, UObject*& ReturnValue);
	static void CreateDumpLocationObject(UObject* Self, UObject*& ReturnValue);
	static void CreateGameDirectoryObject(UObject* Self, UObject*& ReturnValue);
	static void CreateHistoryEvent(UObject* Self, UObject*& ReturnValue);
	static void CreateHistoryObject(UObject* Self, UObject*& ReturnValue);
	static void CreateLogObject(UObject* Self, UObject*& ReturnValue);
	static void DeleteSaveGameFiles(UObject* Self, std::string* saveDirectory);
	static void GetDeusExVersion(UObject* Self, std::string& ReturnValue);
	static void SaveGame(UObject* Self, int saveIndex, std::string* saveDesc);
	static void SetBoolFlagFromString(UObject* Self, const std::string& flagNameString, bool bValue, NameString& ReturnValue);
	static void UnloadTexture(UObject* Self, UObject* Texture);
};
