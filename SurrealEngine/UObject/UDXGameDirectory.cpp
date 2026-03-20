#include "UDXGameDirectory.h"

#include "Utils/Logger.h"
#include "Engine.h"
#include "Package/PackageManager.h"

void UDXGameDirectory::GetGameDirectory()
{
    if (GameDirectoryType() == EGameDirectoryTypes::GD_Maps)
    {
        currentDirectory = fs::path(engine->LaunchInfo.gameRootFolder) / "Maps";
        PopulateDirectoryList();
    }
    else
    {
        currentDirectory = engine->packages->GetSaveFolderPath();
        PopulateSaveInfoPointers();
    }
}

int UDXGameDirectory::GetNewSaveFileIndex()
{
    const auto saveFolder = engine->packages->GetSaveFolderPath();

    // Save folders being formatted like Save0001 implies that the number can go up to 9999
    for (int i = 1 ; i < 10000 ; i++)
    {
        auto folderPath = saveFolder / GetSaveIndexFolderName(i);
        if (!fs::exists(folderPath) || (fs::exists(folderPath) && !fs::is_directory(folderPath)))
            return i;
    }

    return 0;
}

std::string UDXGameDirectory::GenerateSaveFilename(int saveIndex)
{
    return GetSaveIndexFolderName(saveIndex);
}

std::string UDXGameDirectory::GenerateNewSaveFileName(std::optional<int> newIndex)
{
    return GenerateSaveFilename(newIndex ? *newIndex : GetNewSaveFileIndex());
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
    if (GameDirectoryType() != EGameDirectoryTypes::GD_SaveGames)
        // We're not in the Save folder
        return nullptr;

    auto pkg = engine->packages->GetSaveInfoPackage(GetSaveIndexFolderName(fileIndex));

    if (!pkg)
        return nullptr;

    return Cast<UDXSaveInfo>(pkg->GetUObject("DeusExSaveInfo", "MyDeusExSaveInfo"));
}

UDXSaveInfo* UDXGameDirectory::GetSaveInfoFromDirectoryIndex(int DirectoryIndex)
{
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
    const auto saveFolderName = GetSaveIndexFolderName(saveInfo.DirectoryIndex());

    for (auto it = LoadedSaveInfoPointers().cbegin() ; it != LoadedSaveInfoPointers().cend() ; it++)
    {
        if (*it == &saveInfo)
        {
            LoadedSaveInfoPointers().erase(it);
            engine->packages->RemoveSaveInfoPackage(saveFolderName);
            fs::remove_all(engine->packages->GetSaveFolderPath() / saveFolderName); // Also delete the folder
            return;
        }
    }
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

void UDXGameDirectory::PopulateSaveInfoPointers()
{
    Array<UDXSaveInfo*> saveInfos;

    for (const auto& saveInfoPackage : engine->packages->GetSaveInfoPackages())
    {
        saveInfos.push_back(Cast<UDXSaveInfo>(saveInfoPackage.second->GetUObject("DeusExSaveInfo", "MyDeusExSaveInfo")));
    }

    LoadedSaveInfoPointers() = saveInfos;
}
