
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
	Exception::Throw("GameDirectory.DeleteSaveInfo not implemented");
}

void NGameDirectory::GenerateNewSaveFilename(UObject* Self, int* newIndex, std::string& ReturnValue)
{
	Exception::Throw("GameDirectory.GenerateNewSaveFilename not implemented");
}

void NGameDirectory::GenerateSaveFilename(UObject* Self, int saveIndex, std::string& ReturnValue)
{
	Exception::Throw("GameDirectory.GenerateSaveFilename not implemented");
}

void NGameDirectory::GetDirCount(UObject* Self, int& ReturnValue)
{
	Exception::Throw("GameDirectory.GetDirCount not implemented");
}

void NGameDirectory::GetDirFilename(UObject* Self, int fileIndex, std::string& ReturnValue)
{
	Exception::Throw("GameDirectory.GetDirFilename not implemented");
}

void NGameDirectory::GetGameDirectory(UObject* Self)
{
	Exception::Throw("GameDirectory.GetGameDirectory not implemented");
}

void NGameDirectory::GetNewSaveFileIndex(UObject* Self, int& ReturnValue)
{
	Exception::Throw("GameDirectory.GetNewSaveFileIndex not implemented");
}

void NGameDirectory::GetSaveDirectorySize(UObject* Self, int saveIndex, int& ReturnValue)
{
	Exception::Throw("GameDirectory.GetSaveDirectorySize not implemented");
}

void NGameDirectory::GetSaveFreeSpace(UObject* Self, int& ReturnValue)
{
	Exception::Throw("GameDirectory.GetSaveFreeSpace not implemented");
}

void NGameDirectory::GetSaveInfo(UObject* Self, int fileIndex, UObject*& ReturnValue)
{
	Exception::Throw("GameDirectory.GetSaveInfo not implemented");
}

void NGameDirectory::GetSaveInfoFromDirectoryIndex(UObject* Self, int DirectoryIndex, UObject*& ReturnValue)
{
	Exception::Throw("GameDirectory.GetSaveInfoFromDirectoryIndex not implemented");
}

void NGameDirectory::GetTempSaveInfo(UObject* Self, UObject*& ReturnValue)
{
	Exception::Throw("GameDirectory.GetTempSaveInfo not implemented");
}

void NGameDirectory::PurgeAllSaveInfo(UObject* Self)
{
	Exception::Throw("GameDirectory.PurgeAllSaveInfo not implemented");
}

void NGameDirectory::SetDirFilter(UObject* Self, const std::string& strFilter)
{
	Exception::Throw("GameDirectory.SetDirFilter not implemented");
}

void NGameDirectory::SetDirType(UObject* Self, uint8_t newDirType)
{
	Exception::Throw("GameDirectory.SetDirType not implemented");
}
