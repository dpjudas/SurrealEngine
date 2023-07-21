#pragma once

#include "UObject/UObject.h"

class NStatLog
{
public:
	static void RegisterFunctions();

	static void BatchLocal();
	static void BrowseRelativeLocalURL(const std::string& URL);
	static void ExecuteLocalLogBatcher(UObject* Self);
	static void ExecuteSilentLogBatcher(UObject* Self);
	static void ExecuteWorldLogBatcher(UObject* Self);
	static void GetGMTRef(UObject* Self, std::string& ReturnValue);
	static void GetMapFileName(UObject* Self, std::string& ReturnValue);
	static void GetPlayerChecksum(UObject* P, std::string& Checksum);
	static void InitialCheck(UObject* Self, UObject* Game);
	static void LogMutator(UObject* Self, UObject* M);
};
