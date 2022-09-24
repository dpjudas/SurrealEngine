#pragma once

#include "UObject/UObject.h"

class NDumpLocation
{
public:
	static void RegisterFunctions();

	static void AddDumpFileLocation(UObject* Self, const std::string& FileName, const std::string& newTitle, const std::string& newDescription);
	static void CloseDumpFile(UObject* Self);
	static void DeleteDumpFile(UObject* Self, const std::string& FileName);
	static void DeleteDumpFileLocation(UObject* Self, int dumpLocationID);
	static void GetCurrentUser(UObject* Self, std::string& ReturnValue);
	static void GetDumpFileCount(UObject* Self, int& ReturnValue);
	static void GetDumpFileIndex(UObject* Self, int& ReturnValue);
	static void GetDumpFileLocationCount(UObject* Self, const std::string& FileName, int& ReturnValue);
	static void GetDumpFileLocationInfo(UObject* Self);
	static void GetDumpLocationIndex(UObject* Self, int& ReturnValue);
	static void GetFirstDumpFile(UObject* Self, std::string& ReturnValue);
	static void GetFirstDumpFileLocation(UObject* Self, bool& ReturnValue);
	static void GetNextDumpFile(UObject* Self, std::string& ReturnValue);
	static void GetNextDumpFileLocation(UObject* Self, bool& ReturnValue);
	static void GetNextDumpFileLocationID(UObject* Self, int& ReturnValue);
	static void HasLocationBeenSaved(UObject* Self, bool& ReturnValue);
	static void LoadLocation(UObject* Self);
	static void OpenDumpFile(UObject* Self, const std::string& FileName, bool& ReturnValue);
	static void SaveLocation(UObject* Self);
	static void SelectDumpFileLocation(UObject* Self, int dumpLocationID, bool& ReturnValue);
	static void SetPlayer(UObject* Self, UObject* NewPlayer);
};
