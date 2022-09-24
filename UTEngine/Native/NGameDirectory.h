#pragma once

#include "UObject/UObject.h"

class NGameDirectory
{
public:
	static void RegisterFunctions();

	static void DeleteSaveInfo(UObject* Self, UObject* saveInfo);
	static void GenerateNewSaveFilename(UObject* Self, int* newIndex, std::string& ReturnValue);
	static void GenerateSaveFilename(UObject* Self, int saveIndex, std::string& ReturnValue);
	static void GetDirCount(UObject* Self, int& ReturnValue);
	static void GetDirFilename(UObject* Self, int fileIndex, std::string& ReturnValue);
	static void GetGameDirectory(UObject* Self);
	static void GetNewSaveFileIndex(UObject* Self, int& ReturnValue);
	static void GetSaveDirectorySize(UObject* Self, int saveIndex, int& ReturnValue);
	static void GetSaveFreeSpace(UObject* Self, int& ReturnValue);
	static void GetSaveInfo(UObject* Self, int fileIndex, UObject*& ReturnValue);
	static void GetSaveInfoFromDirectoryIndex(UObject* Self, int DirectoryIndex, UObject*& ReturnValue);
	static void GetTempSaveInfo(UObject* Self, UObject*& ReturnValue);
	static void PurgeAllSaveInfo(UObject* Self);
	static void SetDirFilter(UObject* Self, const std::string& strFilter);
	static void SetDirType(UObject* Self, uint8_t newDirType);
};
