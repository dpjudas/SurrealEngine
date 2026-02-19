
#include "Precomp.h"
#include "NGameDirectory.h"
#include "VM/NativeFunc.h"
#include "UObject/UActor.h"
#include "Engine.h"

void NGameDirectory::RegisterFunctions()
{
	RegisterVMNativeFunc_1("GameDirectory", "DeleteSaveInfo", &NGameDirectory::DeleteSaveInfo, 3091);
	RegisterVMNativeFunc_2("GameDirectory", "GenerateNewSaveFilename", &NGameDirectory::GenerateNewSaveFilename, 3083);
	RegisterVMNativeFunc_2("GameDirectory", "GenerateSaveFilename", &NGameDirectory::GenerateSaveFilename, 3082);
	RegisterVMNativeFunc_1("GameDirectory", "GetDirCount", &NGameDirectory::GetDirCount, 3084);
	RegisterVMNativeFunc_2("GameDirectory", "GetDirFilename", &NGameDirectory::GetDirFilename, 3085);
	RegisterVMNativeFunc_0("GameDirectory", "GetGameDirectory", &NGameDirectory::GetGameDirectory, 3080);
	RegisterVMNativeFunc_1("GameDirectory", "GetNewSaveFileIndex", &NGameDirectory::GetNewSaveFileIndex, 3081);
	RegisterVMNativeFunc_2("GameDirectory", "GetSaveDirectorySize", &NGameDirectory::GetSaveDirectorySize, 3094);
	RegisterVMNativeFunc_1("GameDirectory", "GetSaveFreeSpace", &NGameDirectory::GetSaveFreeSpace, 3093);
	RegisterVMNativeFunc_2("GameDirectory", "GetSaveInfo", &NGameDirectory::GetSaveInfo, 3088);
	RegisterVMNativeFunc_2("GameDirectory", "GetSaveInfoFromDirectoryIndex", &NGameDirectory::GetSaveInfoFromDirectoryIndex, 3089);
	RegisterVMNativeFunc_1("GameDirectory", "GetTempSaveInfo", &NGameDirectory::GetTempSaveInfo, 3090);
	RegisterVMNativeFunc_0("GameDirectory", "PurgeAllSaveInfo", &NGameDirectory::PurgeAllSaveInfo, 3092);
	RegisterVMNativeFunc_1("GameDirectory", "SetDirFilter", &NGameDirectory::SetDirFilter, 3087);
	RegisterVMNativeFunc_1("GameDirectory", "SetDirType", &NGameDirectory::SetDirType, 3086);
}

void NGameDirectory::DeleteSaveInfo(UObject* Self, UObject* saveInfo)
{
	LogUnimplemented("GameDirectory.DeleteSaveInfo");
}

void NGameDirectory::GenerateNewSaveFilename(UObject* Self, int* newIndex, std::string& ReturnValue)
{
	LogUnimplemented("GameDirectory.GenerateNewSaveFilename");
	ReturnValue = "";
}

void NGameDirectory::GenerateSaveFilename(UObject* Self, int saveIndex, std::string& ReturnValue)
{
	LogUnimplemented("GameDirectory.GenerateSaveFilename");
	ReturnValue = "";
}

void NGameDirectory::GetDirCount(UObject* Self, int& ReturnValue)
{
	LogUnimplemented("GameDirectory.GetDirCount");
	ReturnValue = 0;
}

void NGameDirectory::GetDirFilename(UObject* Self, int fileIndex, std::string& ReturnValue)
{
	LogUnimplemented("GameDirectory.GetDirFilename");
	ReturnValue = "";
}

void NGameDirectory::GetGameDirectory(UObject* Self)
{
	LogUnimplemented("GameDirectory.GetGameDirectory");
}

void NGameDirectory::GetNewSaveFileIndex(UObject* Self, int& ReturnValue)
{
	LogUnimplemented("GameDirectory.GetNewSaveFileIndex");
	ReturnValue = 0;
}

void NGameDirectory::GetSaveDirectorySize(UObject* Self, int saveIndex, int& ReturnValue)
{
	LogUnimplemented("GameDirectory.GetSaveDirectorySize");
	ReturnValue = 0;
}

void NGameDirectory::GetSaveFreeSpace(UObject* Self, int& ReturnValue)
{
	LogUnimplemented("GameDirectory.GetSaveFreeSpace");
	ReturnValue = 0;
}

void NGameDirectory::GetSaveInfo(UObject* Self, int fileIndex, UObject*& ReturnValue)
{
	LogUnimplemented("GameDirectory.GetSaveInfo");
	ReturnValue = nullptr;
}

void NGameDirectory::GetSaveInfoFromDirectoryIndex(UObject* Self, int DirectoryIndex, UObject*& ReturnValue)
{
	LogUnimplemented("GameDirectory.GetSaveInfoFromDirectoryIndex");
	ReturnValue = nullptr;
}

void NGameDirectory::GetTempSaveInfo(UObject* Self, UObject*& ReturnValue)
{
	LogUnimplemented("GameDirectory.GetTempSaveInfo");
	ReturnValue = nullptr;
}

void NGameDirectory::PurgeAllSaveInfo(UObject* Self)
{
	LogUnimplemented("GameDirectory.PurgeAllSaveInfo");
}

void NGameDirectory::SetDirFilter(UObject* Self, const std::string& strFilter)
{
	LogUnimplemented("GameDirectory.SetDirFilter");
}

void NGameDirectory::SetDirType(UObject* Self, uint8_t newDirType)
{
	LogUnimplemented("GameDirectory.SetDirType");
}
