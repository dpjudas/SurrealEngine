
#include "Precomp.h"
#include "GameFolder.h"
#include "File.h"
#include "UTF16.h"
#include "CommandLine.h"

GameLaunchInfo GameFolderSelection::GetLaunchInfo()
{
	std::vector<GameFolder> foundGames;

	for (const std::string& folder : commandline->GetItems())
	{
		GameFolder game = ExamineFolder(folder);
		if (!game.name.empty())
			foundGames.push_back(game);
	}

	for (const std::string& folder : FindGameFolders())
	{
		GameFolder game = ExamineFolder(folder);
		if (!game.name.empty())
			foundGames.push_back(game);
	}

	if (foundGames.empty())
	{
		throw std::runtime_error("Unable to find a game folder");
	}

	// To do: present some UI here instead of grabbing the first game we found

	GameLaunchInfo info = foundGames.front().launchInfo;

	info.engineVersion = commandline->GetArgInt("-e", "--engineversion", info.engineVersion);
	info.noEntryMap = commandline->HasArg("-n", "--noentrymap");
	info.url = commandline->GetArg("-u", "--url", info.url);

	return info;
}

GameFolder GameFolderSelection::ExamineFolder(const std::string& path)
{
	GameFolder folder;

	if (path.empty())
		return folder;

	// To do: use sha1 checksums to identify which game and engine versions we found

	if (File::try_open_existing(FilePath::combine(path, "System/UnrealTournament.ini")))
	{
		folder.name = "Unreal Tournament";
		folder.launchInfo.folder = path;
		folder.launchInfo.engineVersion = 436;
	}
	else if (File::try_open_existing(FilePath::combine(path, "System/Unreal.ini")))
	{
		folder.name = "Unreal";
		folder.launchInfo.folder = path;
		folder.launchInfo.engineVersion = 226;
	}
	else if (File::try_open_existing(FilePath::combine(path, "System/klingons.ini")))
	{
		folder.name = "Klingon Honor Guard";
		folder.launchInfo.folder = path;
		folder.launchInfo.engineVersion = 219;
	}

	// info.folder = R"(C:\Games\UnrealTournament436)"; info.engineVersion = 436;
	// info.folder = R"(C:\Games\utdemo348)"; info.engineVersion = 348;
	// info.folder = R"(C:\Games\UTDemo338)"; info.engineVersion = 338;
	// info.folder = R"(C:\Games\utdemo3dfx)"; info.engineVersion = 322;
	// info.folder = R"(C:\Games\Steam\steamapps\common\Unreal Gold)"; info.engineVersion = 226;
	// info.folder = R"(C:\Games\klingon)"; info.engineVersion = 219;

	return folder;
}

#ifdef WIN32

static std::string FindEpicRegisteredGame(const std::string& keyname)
{
	std::vector<wchar_t> buffer(1024);
	HKEY regkey = 0;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, to_utf16("SOFTWARE\\Unreal Technology\\Installed Apps\\" + keyname).c_str(), 0, KEY_READ, &regkey) == ERROR_SUCCESS)
	{
		DWORD type = 0;
		DWORD size = (DWORD)(buffer.size() * sizeof(wchar_t));
		LSTATUS result = RegQueryValueEx(regkey, L"Folder", 0, &type, (LPBYTE)buffer.data(), &size);
		RegCloseKey(regkey);
		if (result == ERROR_SUCCESS && type == REG_SZ)
		{
			buffer.back() = 0;
			return from_utf16(buffer.data());
		}
	}
	return {};
}

std::string GameFolderSelection::GetExePath()
{
	std::vector<wchar_t> buffer(1024);
	if (GetModuleFileName(0, buffer.data(), 1024))
	{
		buffer.back() = 0;
		return FilePath::remove_last_component(from_utf16(buffer.data()));
	}
	return {};
}

std::vector<std::string> GameFolderSelection::FindGameFolders()
{
	return
	{
		FindEpicRegisteredGame("UnrealTournament"),
		FindEpicRegisteredGame("Unreal Gold"),
		FindEpicRegisteredGame("Unreal")
	};
}

#else

std::string GameFolderSelection::GetExePath()
{
	return {};
}

std::vector<std::string> GameFolderSelection::FindGameFolders()
{
	return {};
}

#endif
