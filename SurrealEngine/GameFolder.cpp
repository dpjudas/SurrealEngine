
#include "Precomp.h"
#include "GameFolder.h"
#include "Utils/File.h"
#include "Utils/UTF16.h"
#include "UE1GameDatabase.h"
#include "LauncherSettings.h"
#include "Utils/CommandLine.h"
#include <filesystem>

Array<GameLaunchInfo> GameFolderSelection::Games;

void GameFolderSelection::UpdateList()
{
	Games.clear();

	for (const std::string& folder : commandline->GetItems())
	{
		GameLaunchInfo game = ExamineFolder(folder);
		if (!game.gameName.empty())
			Games.push_back(game);
	}

	if (Games.empty())
	{
		// Check if we're within an UE1-game System folder.
		auto p = std::filesystem::current_path();
		if (p.filename().string() == "System")
		{
			GameLaunchInfo game = ExamineFolder(p.parent_path().string());
			if (!game.gameName.empty())
				Games.push_back(game);
		}
	}

	if (Games.empty())
	{
		for (const std::string& folder : FindGameFolders())
		{
			GameLaunchInfo game = ExamineFolder(folder);
			if (!game.gameName.empty())
				Games.push_back(game);
		}
	}
}

GameLaunchInfo GameFolderSelection::GetLaunchInfo(int selectedGame)
{
	if (selectedGame < 0 || (size_t)selectedGame >= Games.size())
		return {};

	GameLaunchInfo info = Games[selectedGame];
	info.ue1Version = commandline->GetArgInt("-e", "--engineversion", info.ue1Version);
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
		info.gameExecutableName = fs::path(ue1_game.second).stem().string();

		switch (ue1_game.first)
		{
			case KnownUE1Games::UNREAL_200:
			{
				info.gameName = "Unreal";
				info.gameVersion = 200;
				info.gameSubVersion = 0;
				info.gameVersionString = "200";
			}
			break;
			case KnownUE1Games::UNREAL_209:
			{
				info.gameName = "Unreal";
				info.gameVersion = 209;
				info.gameSubVersion = 0;
				info.gameVersionString = "209";
			}
			break;
			case KnownUE1Games::UNREAL_216:
			{
				info.gameName = "Unreal";
				info.gameVersion = 216;
				info.gameSubVersion = 0;
				info.gameVersionString = "216";
			}
			break;
			case KnownUE1Games::UNREAL_217:
			{
				info.gameName = "Unreal";
				info.gameVersion = 217;
				info.gameSubVersion = 0;
				info.gameVersionString = "217";
			}
			break;
			case KnownUE1Games::UNREAL_218:
			{
				info.gameName = "Unreal";
				info.gameVersion = 218;
				info.gameSubVersion = 0;
				info.gameVersionString = "218";
			}
			break;
			case KnownUE1Games::UNREAL_219:
			{
				info.gameName = "Unreal";
				info.gameVersion = 219;
				info.gameSubVersion = 0;
				info.gameVersionString = "219";
			}
			break;
			case KnownUE1Games::UNREAL_220:
			{
				info.gameName = "Unreal";
				info.gameVersion = 220;
				info.gameSubVersion = 0;
				info.gameVersionString = "220";
			}
			break;
			case KnownUE1Games::UNREAL_221:
			{
				info.gameName = "Unreal";
				info.gameVersion = 221;
				info.gameSubVersion = 0;
				info.gameVersionString = "221";
			}
			break;
			case KnownUE1Games::UNREAL_224v:
			{
				info.gameName = "Unreal";
				info.gameVersion = 224;
				info.gameSubVersion = 22;
				info.gameVersionString = "224v";
			}
			break;
			case KnownUE1Games::UNREAL_225f:
			{
				info.gameName = "Unreal";
				info.gameVersion = 225;
				info.gameSubVersion = 6;
				info.gameVersionString = "225f";
			}
			break;
			case KnownUE1Games::UNREAL_226f:
			{
				info.gameName = "Unreal";
				info.gameVersion = 226;
				info.gameSubVersion = 6;
				info.gameVersionString = "226f";
			}
			break;
			case KnownUE1Games::UNREALGOLD_226b:
			{
				info.gameName = "Unreal";
				info.gameVersion = 226;
				info.gameSubVersion = 2;
				info.gameVersionString = "226b";
			}
			break;
			case KnownUE1Games::UNREALGOLD_227i:
			{
				info.gameName = "Unreal";
				info.gameVersion = 227;
				info.gameSubVersion = 9;
				info.gameVersionString = "227i";
			}
			break;
			case KnownUE1Games::UNREALGOLD_227j:
			{
				info.gameName = "Unreal";
				info.gameVersion = 227;
				info.gameSubVersion = 10;
				info.gameVersionString = "227j";
			}
			break;
			case KnownUE1Games::UNREALGOLD_227k_11:
			{
				info.gameName = "Unreal";
				info.gameVersion = 227;
				info.gameSubVersion = 11;
				info.gameVersionString = "227k_11";
			}
			break;
			case KnownUE1Games::UNREALGOLD_227k_14:
			{
				info.gameName = "Unreal";
				info.gameVersion = 227;
				info.gameSubVersion = 11;
				info.gameVersionString = "227k_14";
			}
			break;
			case KnownUE1Games::UT99_400:
			{
				info.gameName = "Unreal Tournament";
				info.gameVersion = 400;
				info.gameSubVersion = 0;
				info.gameVersionString = "400";
			}
			break;
			case KnownUE1Games::UT99_436:
			{
				info.gameName = "Unreal Tournament";
				info.gameVersion = 436;
				info.gameSubVersion = 0;
				info.gameVersionString = "436";
			}
			break;
			case KnownUE1Games::UT99_451:
			{
				info.gameName = "Unreal Tournament";
				info.gameVersion = 451;
				info.gameSubVersion = 0;
				info.gameVersionString = "451";
			}
			break;
			case KnownUE1Games::UT99_469a:
			{
				info.gameName = "Unreal Tournament";
				info.gameVersion = 469;
				info.gameSubVersion = 1;
				info.gameVersionString = "469a";
			}
			break;
			case KnownUE1Games::UT99_469b:
			{
				info.gameName = "Unreal Tournament";
				info.gameVersion = 469;
				info.gameSubVersion = 2;
				info.gameVersionString = "469b";
			}
			break;
			case KnownUE1Games::UT99_469c:
			{
				info.gameName = "Unreal Tournament";
				info.gameVersion = 469;
				info.gameSubVersion = 3;
				info.gameVersionString = "469c";
			}
			break;
			case KnownUE1Games::UT99_469d:
			{
				info.gameName = "Unreal Tournament";
				info.gameVersion = 469;
				info.gameSubVersion = 4;
				info.gameVersionString = "469d";
			}
			break;
			case KnownUE1Games::UT99_469e:
			{
				info.gameName = "Unreal Tournament";
				info.gameVersion = 469;
				info.gameSubVersion = 5;
				info.gameVersionString = "469e";
			}
			break;
			case KnownUE1Games::DEUS_EX_1002f:
			{
				info.gameName = "Deus Ex";
				info.ue1Version = 500;
				info.gameVersion = 1002;
				info.gameSubVersion = 0;
				info.gameVersionString = "1002";
			}
			break;
			case KnownUE1Games::DEUS_EX_1112fm:
			{
				info.gameName = "Deus Ex";
				info.ue1Version = 500;
				info.gameVersion = 1112;
				info.gameSubVersion = 0;
				info.gameVersionString = "1112fm";
			}
			break;
			case KnownUE1Games::NERF_300:
			{
				info.gameName = "Nerf Arena Blast";
				info.gameVersion = 300;
				info.gameSubVersion = 0;
				info.gameVersionString = "300";
			}
			break;
			case KnownUE1Games::KLINGON_219:
			{
				info.gameName = "Klingon Honor Guard";
				info.gameVersion = 219;
				info.gameSubVersion = 0;
				info.gameVersionString = "219";
			}
			break;
			case KnownUE1Games::TNN_200:
			{
				info.gameName = "TNN";
				info.gameVersion = 200;
				info.gameSubVersion = 0;
				info.gameVersionString = "200";
			}
			break;
			case KnownUE1Games::RUNE_107:
			{
				info.gameName = "Rune Gold";
				info.ue1Version = 500;
				info.gameVersion = 107;
				info.gameSubVersion = 0;
				info.gameVersionString = "1.07";
			}
			break;
			case KnownUE1Games::RUNE_110:
			{
				info.gameName = "Rune Classic";
				info.ue1Version = 500;
				info.gameVersion = 110;
				info.gameSubVersion = 0;
				info.gameVersionString = "1.10";
			}
			break;
			case KnownUE1Games::RUNE_111:
			{
				info.gameName = "Rune Classic";
				info.ue1Version = 500;
				info.gameVersion = 111;
				info.gameSubVersion = 0;
				info.gameVersionString = "1.11";
			}
			break;
			case KnownUE1Games::UNDYING_420:
			{
				info.gameName = "Clive Barker's Undying";
				info.ue1Version = 500;
				info.gameVersion = 420;
				info.gameSubVersion = 0;
				info.gameVersionString = "420";
			}
			break;
			case KnownUE1Games::TACTICAL_OPS_436:
			{
				info.gameName = "Tactical-Ops: Assault on Terror";
				info.gameVersion = 436;
				info.gameSubVersion = 0;
				info.gameVersionString = "436";
			}
			break;
			case KnownUE1Games::TACTICAL_OPS_469:
			{
				// exe is identical to UT v469d
				info.gameName = "Tactical-Ops: Assault on Terror";
				info.gameVersion = 469;
				info.gameSubVersion = 3;
				info.gameVersionString = "469d";
			}
			break;
			case KnownUE1Games::WHEELOFTIME_333:
			{
				info.gameName = "Wheel of Time";
				info.gameVersion = 333;
				info.gameSubVersion = 0;
				info.gameVersionString = "333";
			}
			break;
			case KnownUE1Games::HARRYPOTTER1_433:
			{
				info.gameName = "Harry Potter and the Philosopher's/Sorcerer's Stone (HP1)";
				info.ue1Version = 500;
				info.gameVersion = 433;
				info.gameSubVersion = 0;
				info.gameVersionString = "433";
			}
			break;
			case KnownUE1Games::HARRYPOTTER2_433:
			{
				info.gameName = "Harry Potter and the Chamber of Secrets (HP2)";
				info.ue1Version = 500;
				info.gameVersion = 433;
				info.gameSubVersion = 0;
				info.gameVersionString = "433";
			}
			break;
			case KnownUE1Games::UE1_GAME_NOT_FOUND:
				// Do nothing, we want the fields to be empty.
			break;
		}

		if (info.ue1Version == 0 && info.gameVersion != 0)
		{
			// If the Epic UE1 engine version isn't specified it matches the game version
			info.ue1Version = info.gameVersion;
		}
	}
	
	// If no game is found, this will be an empty GameLaunchInfo
	return info;
}

#ifdef WIN32

static std::string FindEpicRegisteredGame(const std::string& keyname)
{
	Array<wchar_t> buffer(1024);
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
	Array<wchar_t> buffer(1024);
	if (GetModuleFileName(0, buffer.data(), 1024))
	{
		buffer.back() = 0;
		return fs::path(from_utf16(buffer.data())).parent_path().string();
	}
	return {};
}

Array<std::string> GameFolderSelection::FindGameFolders()
{
#if 1
	return LauncherSettings::Get().Games.SearchList;
#else
	return
	{
		FindEpicRegisteredGame("UnrealTournament"),
		FindEpicRegisteredGame("Unreal Gold"),
		FindEpicRegisteredGame("Unreal"),
		FindEpicRegisteredGame("Deus Ex")
	};
#endif
}

#else

std::string GameFolderSelection::GetExePath()
{
	return {};
}

Array<std::string> GameFolderSelection::FindGameFolders()
{
	return LauncherSettings::Get().Games.SearchList;
}

#endif
