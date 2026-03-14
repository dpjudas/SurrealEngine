#pragma once

#include "UDXSaveInfo.h"
#include "Utils/Array.h"

#include <filesystem>

namespace fs = std::filesystem;

enum class EGameDirectoryTypes : uint8_t
{
    GD_Maps,
    GD_SaveGames
};

class UDXGameDirectory : public UObject
{
public:
    using UObject::UObject;

    EGameDirectoryTypes& GameDirectoryType() { return Value<EGameDirectoryTypes>(PropOffsets_GameDirectory.gameDirectoryType); }
    std::string& CurrentFilter() { return Value<std::string>(PropOffsets_GameDirectory.currentFilter); }
    Array<std::string>& DirectoryList() { return DynamicArray<std::string>(PropOffsets_GameDirectory.directoryList); }
    Array<UDXSaveInfo*>& LoadedSaveInfoPointers() { return DynamicArray<UDXSaveInfo*>(PropOffsets_GameDirectory.loadedSaveInfoPointers); }
    UDXSaveInfo*& TempSaveInfo() { return Value<UDXSaveInfo*>(PropOffsets_GameDirectory.tempSaveInfo); }

    void GetGameDirectory(); // Depends on what GameDirectoryType is set
    int GetNewSaveFileIndex();
    std::string GenerateSaveFilename(int saveIndex);
    std::string GenerateNewSaveFileName(int* newIndex);
    int GetDirCount();
    std::string GetDirFilename(int fileIndex);
    void SetDirType(EGameDirectoryTypes newDirType);
    void SetDirFilter(const std::string& strFilter);
    UDXSaveInfo* GetSaveInfo(int fileIndex);
    UDXSaveInfo* GetSaveInfoFromDirectoryIndex(int DirectoryIndex);
    UDXSaveInfo* GetTempSaveInfo();
    void DeleteSaveInfo(UDXSaveInfo& saveInfo);
    void PurgeAllSaveInfo();
    int GetSaveFreeSpace();
    int GetSaveDirectorySize(int saveIndex);

private:
    fs::path currentDirectory;
    std::string GetSaveIndexFolderName(int saveIndex);
    void PopulateDirectoryList();
};
