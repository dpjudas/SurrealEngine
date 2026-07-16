#pragma once

struct GameLaunchInfo
{
	int ue1Version = 0;						// Epic UE1 engine version game was forked from. 500 means unknown version newer than UT but before UE2
	int gameVersion = 0;					// Engine version as named by the game (e.g. 226, 227, 436...)
	int gameSubVersion = 0;				// Engine sub version displayed as a letter (Note: Isn't always consistent)
	bool noEntryMap = false;
	std::string gameName = "";				// Name of the game (e.g. "Unreal Tournament")
	std::string gameRootFolder = "";		// Path to the folder that contains all the subfolders and files
	std::string gameExecutableName = "";	// Name of the game executable (e.g. "UnrealTournament")
	std::string gameVersionString = "";		// Version (+ sub version) info as a string (e.g. "469d")
	std::string url = "";					// The UnrealURL to launch upon startup

	bool IsUnreal1() const { return gameExecutableName == "Unreal"; }
	bool IsUnreal1_226() const { return IsUnreal1() && gameVersion == 226; }
	bool IsUnreal1_227() const { return IsUnreal1() && gameVersion == 227; }
	bool IsUnreal1_227k() const { return IsUnreal1_227() && gameSubVersion == 11; }
	bool IsUnrealTournament() const { return gameExecutableName == "UnrealTournament"; }
	bool IsUnrealTournament_469() const { return IsUnrealTournament() && gameVersion == 469; }
	bool IsDeusEx() const { return gameExecutableName == "DeusEx"; }
	bool IsCliveBarkersUndying() const { return gameExecutableName == "Undying"; }
	bool IsKlingonHonorGuard() const { return gameExecutableName == "Klingons" || gameExecutableName == "Khg"; }
	bool IsRune() const { return gameExecutableName == "Rune"; }
	bool IsHarryPotter1() const { return gameExecutableName == "HP"; }
	bool IsHarryPotter2() const { return gameExecutableName == "Game"; }
};

class GameFolderSelection
{
public:
	static void UpdateList();
	static GameLaunchInfo GetLaunchInfo(int selectedGame);

	static Array<GameLaunchInfo> Games;

private:
	static GameLaunchInfo ExamineFolder(const std::string& path);
	static Array<std::string> FindGameFolders();
	static std::string GetExePath();
};
