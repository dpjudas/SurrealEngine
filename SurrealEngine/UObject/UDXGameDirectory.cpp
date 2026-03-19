#include "UDXGameDirectory.h"

#include "Utils/Logger.h"
#include "Engine.h"

void UDXGameDirectory::GetGameDirectory()
{
    if (GameDirectoryType() == EGameDirectoryTypes::GD_Maps)
        currentDirectory = fs::path(engine->LaunchInfo.gameRootFolder) / "Maps";
    else
        currentDirectory = fs::path(engine->LaunchInfo.gameRootFolder) / "Save";
}

int UDXGameDirectory::GetNewSaveFileIndex()
{
    LogUnimplemented("GameDirectory.GetNewSaveFileIndex()");
    return 0;
}

std::string UDXGameDirectory::GenerateSaveFilename(int saveIndex)
{
    LogUnimplemented("GameDirectory.GenerateSaveFilename()");
    return "";
}

std::string UDXGameDirectory::GenerateNewSaveFileName(std::optional<int> newIndex)
{
    LogUnimplemented("GameDirectory.GenerateNewSaveFileName()");
    return "";
}

int UDXGameDirectory::GetDirCount()
{
    int count = 0;

    if (fs::exists(currentDirectory) && fs::is_directory(currentDirectory))
        for (auto& p : fs::directory_iterator(currentDirectory))
            count++;

    return count;
}

std::string UDXGameDirectory::GetDirFilename(int fileIndex)
{
    return DirectoryList()[fileIndex];
}

void UDXGameDirectory::SetDirType(EGameDirectoryTypes newDirType)
{
    GameDirectoryType() = newDirType;
}

void UDXGameDirectory::SetDirFilter(const std::string& strFilter)
{
    CurrentFilter() = strFilter;
}

UDXSaveInfo* UDXGameDirectory::GetSaveInfo(int fileIndex)
{
    LogUnimplemented("GameDirectory.GetSaveInfo()");

    if (GameDirectoryType() != EGameDirectoryTypes::GD_SaveGames)
        // We're not in the Save folder
        return nullptr;

    // TODO: Need to load SaveInfo.dxs from currentFolder / saveFolderName, then return it.
    // auto saveFolderName = GetSaveIndexFolderName(fileIndex);

    return nullptr;
}

UDXSaveInfo* UDXGameDirectory::GetSaveInfoFromDirectoryIndex(int DirectoryIndex)
{
    // This one won't work yet as we aren't populating the LoadedSaveInfoPointers array.
    LogUnimplemented("GameDirectory.GetSaveInfoFromDirectoryIndex()");

    for (const auto& dxSaveInfo : LoadedSaveInfoPointers())
        if (dxSaveInfo->DirectoryIndex() == DirectoryIndex)
            return dxSaveInfo;

    return nullptr;
}

UDXSaveInfo* UDXGameDirectory::GetTempSaveInfo()
{
    return TempSaveInfo();
}

void UDXGameDirectory::DeleteSaveInfo(UDXSaveInfo& saveInfo)
{
    LogUnimplemented("GameDirectory.DeleteSaveInfo()");
}

void UDXGameDirectory::PurgeAllSaveInfo()
{
    if (GameDirectoryType() == EGameDirectoryTypes::GD_Maps)
        return;

    if (fs::exists(currentDirectory) && fs::is_directory(currentDirectory))
        for (auto& p : fs::directory_iterator(currentDirectory))
            // We're not using remove_all() as we don't want to remove the Save folder itself
            fs::remove(p.path());
}

int UDXGameDirectory::GetSaveFreeSpace()
{
    // Returns a value in KBs, which limits us to ~2TB of "free space" max.
    // Should be enough but still
    const auto freeSpaceInKBs = static_cast<int>(fs::space(currentDirectory).free / 1024);
    // Capped at 1TB
    return std::min(freeSpaceInKBs, 1000 * 1024 * 1024);
}

int UDXGameDirectory::GetSaveDirectorySize(int saveIndex)
{
    if (GameDirectoryType() != EGameDirectoryTypes::GD_SaveGames)
        // We're not in the Save folder
        return 0;

    int size = 0;

    for (auto& p : fs::directory_iterator(currentDirectory / GetSaveIndexFolderName(saveIndex)))
        size += (int)p.file_size();

    return size;
}

std::string UDXGameDirectory::GetSaveIndexFolderName(int saveIndex)
{
    std::string folderName = std::to_string(saveIndex);
    folderName.insert(0, 4 - folderName.length(), '0'); // Pad with 0s
    return "Save" + folderName;
}

void UDXGameDirectory::PopulateDirectoryList()
{
    Array<std::string> newList;

    for (auto& p : fs::directory_iterator(currentDirectory))
        if (p.is_regular_file())
            newList.push_back(p.path().filename().string());

    DirectoryList() = newList;
}
