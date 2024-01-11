
#include "Precomp.h"
#include "GameFolder.h"
#include "File.h"
#include "UTF16.h"
#include "CommandLine.h"
#include <filesystem>
#include "TinySHA1/TinySHA1.hpp"

GameLaunchInfo GameFolderSelection::GetLaunchInfo()
{
	std::vector<GameLaunchInfo> foundGames;

	for (const std::string& folder : commandline->GetItems())
	{
		GameLaunchInfo game = ExamineFolder(folder);
		if (!game.name.empty())
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
		throw std::runtime_error("Unable to find a game folder");
	}

	// To do: present some UI here instead of grabbing the first game we found

	GameLaunchInfo info = foundGames.front();

	info.engineVersion = commandline->GetArgInt("-e", "--engineversion", info.engineVersion);
	info.gameName = commandline->GetArg("-g", "--game", info.gameName);
	info.noEntryMap = commandline->HasArg("-n", "--noentrymap") || info.noEntryMap;
	info.url = commandline->GetArg("-u", "--url", info.url);

	return info;
}

GameFolder GameFolderSelection::ExamineFolder(const std::string& path)
{
	GameLaunchInfo info;

	if (path.empty())
		return info;
	
	std::string path_with_system = FilePath::combine(path, "System");

	for (auto& executable_name : knownUE1ExecutableNames)
	{
		std::string executable_path = FilePath::combine(path_with_system, executable_name);

		if (File::try_open_existing(executable_path))
		{
			// Such executable exists, let's try to take SHA1Sum of it
			auto bytes = File::read_all_bytes(executable_path);

			sha1::SHA1 s;
			s.processBytes(bytes.data(), bytes.size());
			uint32_t digest[5];
			s.getDigest(digest);

			char temp[41];
			snprintf(temp, 41, "%08x%08x%08x%08x%08x", digest[0], digest[1], digest[2], digest[3], digest[4]);

			std::string sha1sum(temp);

			// Now check whether there is a match within the database or not
			auto it = SHA1Database.find(sha1sum);

			if (it != SHA1Database.end())
			{
				info.gameRootFolder = path;
				info.gameExecutableName = FilePath::remove_extension(executable_name);

				switch (it->second)
				{
					case KnownUE1Games::UNREALGOLD_226b:
					{
						info.gameName = "Unreal";
						info.engineVersion = 226;
						info.engineSubVersion = 2;
					}
					break;
					case KnownUE1Games::UNREALGOLD_227i:
					{
						info.gameName = "Unreal";
						info.engineVersion = 227;
						info.engineSubVersion = 9;
					}
					break;
					case KnownUE1Games::UNREALGOLD_227j:
					{
						info.gameName = "Unreal";
						info.engineVersion = 227;
						info.engineSubVersion = 10;
					}
					break;
					case KnownUE1Games::UT99_436:
					{
						info.gameName = "Unreal Tournament";
						info.engineVersion = 436;
						info.engineSubVersion = 0;
					}
					break;
					case KnownUE1Games::UT99_451:
					{
						info.gameName = "Unreal Tournament";
						info.engineVersion = 451;
						info.engineSubVersion = 0;
					}
					break;
					case KnownUE1Games::UT99_469a:
					{
						info.gameName = "Unreal Tournament";
						info.engineVersion = 469;
						info.engineSubVersion = 0;
					}
					break;
					case KnownUE1Games::UT99_469b:
					{
						info.gameName = "Unreal Tournament";
						info.engineVersion = 469;
						info.engineSubVersion = 1;
					}
					break;
					case KnownUE1Games::UT99_469c:
					{
						info.gameName = "Unreal Tournament";
						info.engineVersion = 469;
						info.engineSubVersion = 2;
					}
					break;
					case KnownUE1Games::UT99_469d:
					{
						info.gameName = "Unreal Tournament";
						info.engineVersion = 469;
						info.engineSubVersion = 3;
					}
					break;
					case KnownUE1Games::DEUS_EX_1002f:
					{
						info.gameName = "Deus Ex";
						info.engineVersion = 1002;
						info.engineSubVersion = 0;
					}
					break;
					case KnownUE1Games::DEUS_EX_1112fm:
					{
						info.gameName = "Deus Ex";
						info.engineVersion = 1112;
						info.engineSubVersion = 0;
					}
					break;
					case KnownUE1Games::NERF_300:
					{
						info.gameName = "Nerf Arena Blast";
						info.engineVersion = 300;
						info.engineSubVersion = 0;
					}
					break;
					case KnownUE1Games::KLINGON_219:
					{
						info.gameName = "Klingon Honor Guard";
						info.engineVersion = 219;
						info.engineSubVersion = 0;
					}
					break;
					case KnownUE1Games::TNN_200:
					{
						info.gameName = "TNN";
						info.engineVersion = 200;
						info.engineSubVersion = 0;
					}
				}
			}
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
