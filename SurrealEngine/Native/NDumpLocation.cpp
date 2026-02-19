
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
	LogUnimplemented("DumpLocation.AddDumpFileLocation");
}

void NDumpLocation::CloseDumpFile(UObject* Self)
{
	LogUnimplemented("DumpLocation.CloseDumpFile");
}

void NDumpLocation::DeleteDumpFile(UObject* Self, const std::string& FileName)
{
	LogUnimplemented("DumpLocation.DeleteDumpFile");
}

void NDumpLocation::DeleteDumpFileLocation(UObject* Self, int dumpLocationID)
{
	LogUnimplemented("DumpLocation.DeleteDumpFileLocation");
}

void NDumpLocation::GetCurrentUser(UObject* Self, std::string& ReturnValue)
{
	LogUnimplemented("DumpLocation.GetCurrentUser");
	ReturnValue = "";
}

void NDumpLocation::GetDumpFileCount(UObject* Self, int& ReturnValue)
{
	LogUnimplemented("DumpLocation.GetDumpFileCount");
	ReturnValue = 0;
}

void NDumpLocation::GetDumpFileIndex(UObject* Self, int& ReturnValue)
{
	LogUnimplemented("DumpLocation.GetDumpFileIndex");
	ReturnValue = 0;
}

void NDumpLocation::GetDumpFileLocationCount(UObject* Self, const std::string& FileName, int& ReturnValue)
{
	LogUnimplemented("DumpLocation.GetDumpFileLocationCount");
	ReturnValue = 0;
}

void NDumpLocation::GetDumpFileLocationInfo(UObject* Self)
{
	LogUnimplemented("DumpLocation.GetDumpFileLocationInfo");
}

void NDumpLocation::GetDumpLocationIndex(UObject* Self, int& ReturnValue)
{
	LogUnimplemented("DumpLocation.GetDumpLocationIndex");
	ReturnValue = 0;
}

void NDumpLocation::GetFirstDumpFile(UObject* Self, std::string& ReturnValue)
{
	LogUnimplemented("DumpLocation.GetFirstDumpFile");
	ReturnValue = "";
}

void NDumpLocation::GetFirstDumpFileLocation(UObject* Self, BitfieldBool& ReturnValue)
{
	LogUnimplemented("DumpLocation.GetFirstDumpFileLocation");
	ReturnValue = false;
}

void NDumpLocation::GetNextDumpFile(UObject* Self, std::string& ReturnValue)
{
	LogUnimplemented("DumpLocation.GetNextDumpFile");
	ReturnValue = "";
}

void NDumpLocation::GetNextDumpFileLocation(UObject* Self, BitfieldBool& ReturnValue)
{
	LogUnimplemented("DumpLocation.GetNextDumpFileLocation");
	ReturnValue = false;
}

void NDumpLocation::GetNextDumpFileLocationID(UObject* Self, int& ReturnValue)
{
	LogUnimplemented("DumpLocation.GetNextDumpFileLocationID");
	ReturnValue = 0;
}

void NDumpLocation::HasLocationBeenSaved(UObject* Self, BitfieldBool& ReturnValue)
{
	LogUnimplemented("DumpLocation.HasLocationBeenSaved");
	ReturnValue = false;
}

void NDumpLocation::LoadLocation(UObject* Self)
{
	LogUnimplemented("DumpLocation.LoadLocation");
}

void NDumpLocation::OpenDumpFile(UObject* Self, const std::string& FileName, BitfieldBool& ReturnValue)
{
	LogUnimplemented("DumpLocation.OpenDumpFile");
	ReturnValue = false;
}

void NDumpLocation::SaveLocation(UObject* Self)
{
	LogUnimplemented("DumpLocation.SaveLocation");
}

void NDumpLocation::SelectDumpFileLocation(UObject* Self, int dumpLocationID, BitfieldBool& ReturnValue)
{
	LogUnimplemented("DumpLocation.SelectDumpFileLocation");
	ReturnValue = false;
}

void NDumpLocation::SetPlayer(UObject* Self, UObject* NewPlayer)
{
	LogUnimplemented("DumpLocation.SetPlayer");
}
