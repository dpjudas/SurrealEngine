
#include "Precomp.h"
#include "GameFolder.h"
#include "Utils/File.h"
#include "Utils/UTF16.h"
#include "UE1GameDatabase.h"
#include "Utils/CommandLine.h"
#include "UI/Launcher/LauncherWindow.h"
#include <filesystem>

GameLaunchInfo GameFolderSelection::GetLaunchInfo()
{
	std::vector<GameLaunchInfo> foundGames;

	for (const std::string& folder : commandline->GetItems())
	{
		GameLaunchInfo game = ExamineFolder(folder);
		if (!game.gameName.empty())
			foundGames.push_back(game);
	}

	if (foundGames.empty())
	{
		// Check if we're within an UE1-game System folder.
		auto p = std::filesystem::current_path();
		if (p.filename().string() == "System")
		{
			GameLaunchInfo game = ExamineFolder(p.parent_path().string());
			if (!game.gameName.empty())
				foundGames.push_back(game);
		}
	}

	if (foundGames.empty())
	{
		for (const std::string& folder : FindGameFolders())
		{
			GameLaunchInfo game = ExamineFolder(folder);
			if (!game.gameName.empty())
				foundGames.push_back(game);
		}
	}
	
	if (foundGames.empty())
	{
		// If we STILL didn't find anything, then there is nothing else we can do
		Exception::Throw("Unable to find a game folder");
	}

	int selectedGame = LauncherWindow::ExecModal(foundGames);
	if (selectedGame < 0)
		return {};

	GameLaunchInfo info = foundGames[selectedGame];

	info.engineVersion = commandline->GetArgInt("-e", "--engineversion", info.engineVersion);
	info.gameName = commandline->GetArg("-g", "--game", info.gameName);
	info.noEntryMap = commandline->HasArg("-n", "--noentrymap") || info.noEntryMap;
	info.url = commandline->GetArg("-u", "--url", info.url);

	return info;
}

GameLaunchInfo GameFolderSelection::ExamineFolder(const std::string& path)
{
	GameLaunchInfo info;

	if (path.empty())
		return info;

	auto ue1_game = FindUE1GameInPath(path);

	if (ue1_game.first != KnownUE1Games::UE1_GAME_NOT_FOUND)
	{
		info.gameRootFolder = path;
		info.gameExecutableName = FilePath::remove_extension(ue1_game.second);

		switch (ue1_game.first)
		{
			case KnownUE1Games::UNREAL_200:
			{
				info.gameName = "Unreal";
				info.engineVersion = 200;
				info.engineSubVersion = 0;
				info.gameVersionString = "200";
			}
			break;
			case KnownUE1Games::UNREAL_220:
			{
				info.gameName = "Unreal";
				info.engineVersion = 220;
				info.engineSubVersion = 0;
				info.gameVersionString = "220";
			}
			break;
			case KnownUE1Games::UNREAL_224v:
			{
				info.gameName = "Unreal";
				info.engineVersion = 224;
				info.engineSubVersion = 22;
				info.gameVersionString = "224v";
			}
			break;
			case KnownUE1Games::UNREAL_225f:
			{
				info.gameName = "Unreal";
				info.engineVersion = 225;
				info.engineSubVersion = 6;
				info.gameVersionString = "225f";
			}
			break;
			case KnownUE1Games::UNREAL_226f:
			{
				info.gameName = "Unreal";
				info.engineVersion = 226;
				info.engineSubVersion = 6;
				info.gameVersionString = "226f";
			}
			break;
			case KnownUE1Games::UNREALGOLD_226b:
			{
				info.gameName = "Unreal";
				info.engineVersion = 226;
				info.engineSubVersion = 2;
				info.gameVersionString = "226b";
			}
			break;
			case KnownUE1Games::UNREALGOLD_227i:
			{
				info.gameName = "Unreal";
				info.engineVersion = 227;
				info.engineSubVersion = 9;
				info.gameVersionString = "227i";
			}
			break;
			case KnownUE1Games::UNREALGOLD_227j:
			{
				info.gameName = "Unreal";
				info.engineVersion = 227;
				info.engineSubVersion = 10;
				info.gameVersionString = "227j";
			}
			break;
			case KnownUE1Games::UNREALGOLD_227k_11:
			{
				info.gameName = "Unreal";
				info.engineVersion = 227;
				info.engineSubVersion = 11;
				info.gameVersionString = "227k_11";
			}
			break;
			case KnownUE1Games::UT99_436:
			{
				info.gameName = "Unreal Tournament";
				info.engineVersion = 436;
				info.engineSubVersion = 0;
				info.gameVersionString = "436";
			}
			break;
			case KnownUE1Games::UT99_451:
			{
				info.gameName = "Unreal Tournament";
				info.engineVersion = 451;
				info.engineSubVersion = 0;
				info.gameVersionString = "451";
			}
			break;
			case KnownUE1Games::UT99_469a:
			{
				info.gameName = "Unreal Tournament";
				info.engineVersion = 469;
				info.engineSubVersion = 1;
				info.gameVersionString = "469a";
			}
			break;
			case KnownUE1Games::UT99_469b:
			{
				info.gameName = "Unreal Tournament";
				info.engineVersion = 469;
				info.engineSubVersion = 2;
				info.gameVersionString = "469b";
			}
			break;
			case KnownUE1Games::UT99_469c:
			{
				info.gameName = "Unreal Tournament";
				info.engineVersion = 469;
				info.engineSubVersion = 3;
				info.gameVersionString = "469c";
			}
			break;
			case KnownUE1Games::UT99_469d:
			{
				info.gameName = "Unreal Tournament";
				info.engineVersion = 469;
				info.engineSubVersion = 4;
				info.gameVersionString = "469d";
			}
			break;
			case KnownUE1Games::DEUS_EX_1002f:
			{
				info.gameName = "Deus Ex";
				info.engineVersion = 1002;
				info.engineSubVersion = 0;
				info.gameVersionString = "1002";
			}
			break;
			case KnownUE1Games::DEUS_EX_1112fm:
			{
				info.gameName = "Deus Ex";
				info.engineVersion = 1112;
				info.engineSubVersion = 0;
				info.gameVersionString = "1112fm";
			}
			break;
			case KnownUE1Games::NERF_300:
			{
				info.gameName = "Nerf Arena Blast";
				info.engineVersion = 300;
				info.engineSubVersion = 0;
				info.gameVersionString = "300";
			}
			break;
			case KnownUE1Games::KLINGON_219:
			{
				info.gameName = "Klingon Honor Guard";
				info.engineVersion = 219;
				info.engineSubVersion = 0;
				info.gameVersionString = "219";
			}
			break;
			case KnownUE1Games::TNN_200:
			{
				info.gameName = "TNN";
				info.engineVersion = 200;
				info.engineSubVersion = 0;
				info.gameVersionString = "200";
			}
			break;
			case KnownUE1Games::RUNE_110:
			{
				info.gameName = "Rune Classic";
				info.engineVersion = 110;
				info.engineSubVersion = 0;
				info.gameVersionString = "1.10";
			}
			break;
			case KnownUE1Games::UNDYING_420:
			{
				info.gameName = "Clive Barker's Undying";
				info.engineVersion = 420;
				info.engineSubVersion = 0;
				info.gameVersionString = "420";
			}
			break;
			case KnownUE1Games::TACTICAL_OPS_436:
			{
				info.gameName = "Tactical-Ops: Assault on Terror";
				info.engineVersion = 436;
				info.engineSubVersion = 0;
				info.gameVersionString = "436";
			}
			break;
			case KnownUE1Games::TACTICAL_OPS_469:
			{
				// exe is identical to UT v469d
				info.gameName = "Tactical-Ops: Assault on Terror";
				info.engineVersion = 469;
				info.engineSubVersion = 3;
				info.gameVersionString = "469d";
			}
			break;
			case KnownUE1Games::WHEELOFTIME_333:
			{
				info.gameName = "Wheel of Time";
				info.engineVersion = 333;
				info.engineSubVersion = 0;
				info.gameVersionString = "333";
			}
			break;
		}
	}
	
	// If no game is found, this will be an empty GameLaunchInfo
	return info;
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
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, to_utf16("SOFTWARE\\WOW6432Node\\Unreal Technology\\Installed Apps\\" + keyname).c_str(), 0, KEY_READ, &regkey) == ERROR_SUCCESS)
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
		FindEpicRegisteredGame("Unreal"),
		FindEpicRegisteredGame("Deus Ex")
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
