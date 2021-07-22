
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
	engine->LogUnimplemented("StatLog.BatchLocal");
}

void NStatLog::BrowseRelativeLocalURL(const std::string& URL)
{
	// Called by Stats->Help with ngStats
	engine->LogUnimplemented("StatLog.BrowseRelativeLocalURL(" + URL + ")");
}

void NStatLog::ExecuteLocalLogBatcher(UObject* Self)
{
	engine->LogUnimplemented("StatLog.ExecuteLocalLogBatcher");
}

void NStatLog::ExecuteSilentLogBatcher(UObject* Self)
{
	engine->LogUnimplemented("StatLog.ExecuteSilentLogBatcher");
}

void NStatLog::ExecuteWorldLogBatcher(UObject* Self)
{
	engine->LogUnimplemented("StatLog.ExecuteWorldLogBatcher");
}

void NStatLog::GetGMTRef(UObject* Self, std::string& ReturnValue)
{
	ReturnValue = "GMTRef dummy value";
	engine->LogUnimplemented("StatLog.GetGMTRef");
}

void NStatLog::GetMapFileName(UObject* Self, std::string& ReturnValue)
{
	ReturnValue = "StatLog.GetMapFileName dummy value";
	engine->LogUnimplemented("StatLog.GetMapFileName");
}

void NStatLog::GetPlayerChecksum(UObject* P, std::string& Checksum)
{
	Checksum = "GetPlayerChecksum dummy value";
	engine->LogUnimplemented("StatLog.GetPlayerChecksum");
}

void NStatLog::InitialCheck(UObject* Self, UObject* Game)
{
	engine->LogUnimplemented("StatLog.InitialCheck");
}

void NStatLog::LogMutator(UObject* Self, UObject* M)
{
	engine->LogUnimplemented("StatLog.LogMutator");
}
