
#include "Precomp.h"
#include "NStatLog.h"
#include "VM/NativeFunc.h"
#include "Engine.h"

void NStatLog::RegisterFunctions()
{
	RegisterVMNativeFunc_0("StatLog", "BatchLocal", &NStatLog::BatchLocal, 0);
	RegisterVMNativeFunc_1("StatLog", "BrowseRelativeLocalURL", &NStatLog::BrowseRelativeLocalURL, 0);
	RegisterVMNativeFunc_0("StatLog", "ExecuteLocalLogBatcher", &NStatLog::ExecuteLocalLogBatcher, 0);
	RegisterVMNativeFunc_0("StatLog", "ExecuteSilentLogBatcher", &NStatLog::ExecuteSilentLogBatcher, 0);
	RegisterVMNativeFunc_0("StatLog", "ExecuteWorldLogBatcher", &NStatLog::ExecuteWorldLogBatcher, 0);
	RegisterVMNativeFunc_1("StatLog", "GetGMTRef", &NStatLog::GetGMTRef, 0);
	RegisterVMNativeFunc_1("StatLog", "GetMapFileName", &NStatLog::GetMapFileName, 0);
	RegisterVMNativeFunc_2("StatLog", "GetPlayerChecksum", &NStatLog::GetPlayerChecksum, 0);
	RegisterVMNativeFunc_1("StatLog", "InitialCheck", &NStatLog::InitialCheck, 0);
	RegisterVMNativeFunc_1("StatLog", "LogMutator", &NStatLog::LogMutator, 0);
}

void NStatLog::BatchLocal()
{
	// Called by Stats->View Local ngStats menu item
	engine->Log.push_back("Warning: StatLog.BatchLocal not implemented");
}

void NStatLog::BrowseRelativeLocalURL(const std::string& URL)
{
	// Called by Stats->Help with ngStats
	engine->Log.push_back("Warning: StatLog.BrowseRelativeLocalURL not implemented. URL was " + URL);
}

void NStatLog::ExecuteLocalLogBatcher(UObject* Self)
{
	engine->Log.push_back("Warning: StatLog.ExecuteLocalLogBatcher not implemented");
}

void NStatLog::ExecuteSilentLogBatcher(UObject* Self)
{
	engine->Log.push_back("Warning: StatLog.ExecuteSilentLogBatcher not implemented");
}

void NStatLog::ExecuteWorldLogBatcher(UObject* Self)
{
	engine->Log.push_back("Warning: StatLog.ExecuteWorldLogBatcher not implemented");
}

void NStatLog::GetGMTRef(UObject* Self, std::string& ReturnValue)
{
	ReturnValue = "GMTRef dummy value";
	engine->Log.push_back("Warning: StatLog.GetGMTRef not implemented");
}

void NStatLog::GetMapFileName(UObject* Self, std::string& ReturnValue)
{
	ReturnValue = "StatLog.GetMapFileName dummy value";
	engine->Log.push_back("Warning: StatLog.GetMapFileName not implemented");
}

void NStatLog::GetPlayerChecksum(UObject* P, std::string& Checksum)
{
	Checksum = "GetPlayerChecksum dummy value";
	engine->Log.push_back("Warning: StatLog.GetPlayerChecksum not implemented");
}

void NStatLog::InitialCheck(UObject* Self, UObject* Game)
{
	engine->Log.push_back("Warning: StatLog.InitialCheck not implemented");
}

void NStatLog::LogMutator(UObject* Self, UObject* M)
{
	engine->Log.push_back("Warning: StatLog.LogMutator not implemented");
}
