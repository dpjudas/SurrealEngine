
#include "Precomp.h"
#include "NStatLog.h"
#include "VM/NativeFunc.h"

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
	// throw std::runtime_error("StatLog.BatchLocal not implemented");
}

void NStatLog::BrowseRelativeLocalURL(const std::string& URL)
{
	// Called by Stats->Help with ngStats
	//throw std::runtime_error("StatLog.BrowseRelativeLocalURL not implemented");
}

void NStatLog::ExecuteLocalLogBatcher(UObject* Self)
{
	throw std::runtime_error("StatLog.ExecuteLocalLogBatcher not implemented");
}

void NStatLog::ExecuteSilentLogBatcher(UObject* Self)
{
	throw std::runtime_error("StatLog.ExecuteSilentLogBatcher not implemented");
}

void NStatLog::ExecuteWorldLogBatcher(UObject* Self)
{
	throw std::runtime_error("StatLog.ExecuteWorldLogBatcher not implemented");
}

void NStatLog::GetGMTRef(UObject* Self, std::string& ReturnValue)
{
	throw std::runtime_error("StatLog.GetGMTRef not implemented");
}

void NStatLog::GetMapFileName(UObject* Self, std::string& ReturnValue)
{
	throw std::runtime_error("StatLog.GetMapFileName not implemented");
}

void NStatLog::GetPlayerChecksum(UObject* P, std::string& Checksum)
{
	throw std::runtime_error("StatLog.GetPlayerChecksum not implemented");
}

void NStatLog::InitialCheck(UObject* Self, UObject* Game)
{
	throw std::runtime_error("StatLog.InitialCheck not implemented");
}

void NStatLog::LogMutator(UObject* Self, UObject* M)
{
	throw std::runtime_error("StatLog.LogMutator not implemented");
}
