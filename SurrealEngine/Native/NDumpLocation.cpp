
#include "Precomp.h"
#include "NDumpLocation.h"
#include "VM/NativeFunc.h"
#include "UObject/UActor.h"
#include "Engine.h"

void NDumpLocation::RegisterFunctions()
{
	RegisterVMNativeFunc_3("DumpLocation", "AddDumpFileLocation", &NDumpLocation::AddDumpFileLocation, 3033);
	RegisterVMNativeFunc_0("DumpLocation", "CloseDumpFile", &NDumpLocation::CloseDumpFile, 3025);
	RegisterVMNativeFunc_1("DumpLocation", "DeleteDumpFile", &NDumpLocation::DeleteDumpFile, 3026);
	RegisterVMNativeFunc_1("DumpLocation", "DeleteDumpFileLocation", &NDumpLocation::DeleteDumpFileLocation, 3032);
	RegisterVMNativeFunc_1("DumpLocation", "GetCurrentUser", &NDumpLocation::GetCurrentUser, 3035);
	RegisterVMNativeFunc_1("DumpLocation", "GetDumpFileCount", &NDumpLocation::GetDumpFileCount, 3023);
	RegisterVMNativeFunc_1("DumpLocation", "GetDumpFileIndex", &NDumpLocation::GetDumpFileIndex, 3022);
	RegisterVMNativeFunc_2("DumpLocation", "GetDumpFileLocationCount", &NDumpLocation::GetDumpFileLocationCount, 3040);
	RegisterVMNativeFunc_0("DumpLocation", "GetDumpFileLocationInfo", &NDumpLocation::GetDumpFileLocationInfo, 3031);
	RegisterVMNativeFunc_1("DumpLocation", "GetDumpLocationIndex", &NDumpLocation::GetDumpLocationIndex, 3029);
	RegisterVMNativeFunc_1("DumpLocation", "GetFirstDumpFile", &NDumpLocation::GetFirstDumpFile, 3020);
	RegisterVMNativeFunc_1("DumpLocation", "GetFirstDumpFileLocation", &NDumpLocation::GetFirstDumpFileLocation, 3027);
	RegisterVMNativeFunc_1("DumpLocation", "GetNextDumpFile", &NDumpLocation::GetNextDumpFile, 3021);
	RegisterVMNativeFunc_1("DumpLocation", "GetNextDumpFileLocation", &NDumpLocation::GetNextDumpFileLocation, 3028);
	RegisterVMNativeFunc_1("DumpLocation", "GetNextDumpFileLocationID", &NDumpLocation::GetNextDumpFileLocationID, 3034);
	RegisterVMNativeFunc_1("DumpLocation", "HasLocationBeenSaved", &NDumpLocation::HasLocationBeenSaved, 3039);
	RegisterVMNativeFunc_0("DumpLocation", "LoadLocation", &NDumpLocation::LoadLocation, 3038);
	RegisterVMNativeFunc_2("DumpLocation", "OpenDumpFile", &NDumpLocation::OpenDumpFile, 3024);
	RegisterVMNativeFunc_0("DumpLocation", "SaveLocation", &NDumpLocation::SaveLocation, 3037);
	RegisterVMNativeFunc_2("DumpLocation", "SelectDumpFileLocation", &NDumpLocation::SelectDumpFileLocation, 3030);
	RegisterVMNativeFunc_1("DumpLocation", "SetPlayer", &NDumpLocation::SetPlayer, 3036);
}

void NDumpLocation::AddDumpFileLocation(UObject* Self, const std::string& FileName, const std::string& newTitle, const std::string& newDescription)
{
	throw std::runtime_error("DumpLocation.AddDumpFileLocation not implemented");
}

void NDumpLocation::CloseDumpFile(UObject* Self)
{
	throw std::runtime_error("DumpLocation.CloseDumpFile not implemented");
}

void NDumpLocation::DeleteDumpFile(UObject* Self, const std::string& FileName)
{
	throw std::runtime_error("DumpLocation.DeleteDumpFile not implemented");
}

void NDumpLocation::DeleteDumpFileLocation(UObject* Self, int dumpLocationID)
{
	throw std::runtime_error("DumpLocation.DeleteDumpFileLocation not implemented");
}

void NDumpLocation::GetCurrentUser(UObject* Self, std::string& ReturnValue)
{
	throw std::runtime_error("DumpLocation.GetCurrentUser not implemented");
}

void NDumpLocation::GetDumpFileCount(UObject* Self, int& ReturnValue)
{
	throw std::runtime_error("DumpLocation.GetDumpFileCount not implemented");
}

void NDumpLocation::GetDumpFileIndex(UObject* Self, int& ReturnValue)
{
	throw std::runtime_error("DumpLocation.GetDumpFileIndex not implemented");
}

void NDumpLocation::GetDumpFileLocationCount(UObject* Self, const std::string& FileName, int& ReturnValue)
{
	throw std::runtime_error("DumpLocation.GetDumpFileLocationCount not implemented");
}

void NDumpLocation::GetDumpFileLocationInfo(UObject* Self)
{
	throw std::runtime_error("DumpLocation.GetDumpFileLocationInfo not implemented");
}

void NDumpLocation::GetDumpLocationIndex(UObject* Self, int& ReturnValue)
{
	throw std::runtime_error("DumpLocation.GetDumpLocationIndex not implemented");
}

void NDumpLocation::GetFirstDumpFile(UObject* Self, std::string& ReturnValue)
{
	throw std::runtime_error("DumpLocation.GetFirstDumpFile not implemented");
}

void NDumpLocation::GetFirstDumpFileLocation(UObject* Self, bool& ReturnValue)
{
	throw std::runtime_error("DumpLocation.GetFirstDumpFileLocation not implemented");
}

void NDumpLocation::GetNextDumpFile(UObject* Self, std::string& ReturnValue)
{
	throw std::runtime_error("DumpLocation.GetNextDumpFile not implemented");
}

void NDumpLocation::GetNextDumpFileLocation(UObject* Self, bool& ReturnValue)
{
	throw std::runtime_error("DumpLocation.GetNextDumpFileLocation not implemented");
}

void NDumpLocation::GetNextDumpFileLocationID(UObject* Self, int& ReturnValue)
{
	throw std::runtime_error("DumpLocation.GetNextDumpFileLocationID not implemented");
}

void NDumpLocation::HasLocationBeenSaved(UObject* Self, bool& ReturnValue)
{
	throw std::runtime_error("DumpLocation.HasLocationBeenSaved not implemented");
}

void NDumpLocation::LoadLocation(UObject* Self)
{
	throw std::runtime_error("DumpLocation.LoadLocation not implemented");
}

void NDumpLocation::OpenDumpFile(UObject* Self, const std::string& FileName, bool& ReturnValue)
{
	throw std::runtime_error("DumpLocation.OpenDumpFile not implemented");
}

void NDumpLocation::SaveLocation(UObject* Self)
{
	throw std::runtime_error("DumpLocation.SaveLocation not implemented");
}

void NDumpLocation::SelectDumpFileLocation(UObject* Self, int dumpLocationID, bool& ReturnValue)
{
	throw std::runtime_error("DumpLocation.SelectDumpFileLocation not implemented");
}

void NDumpLocation::SetPlayer(UObject* Self, UObject* NewPlayer)
{
	throw std::runtime_error("DumpLocation.SetPlayer not implemented");
}
