#pragma once

struct GameLaunchInfo
{
	int engineVersion = 0;					// Engine version (e.g. 226, 227, 436...)
	int engineSubVersion = 0;				// Engine sub version displayed as a letter (Note: Isn't always consistent)
	bool noEntryMap = false;
	std::string gameName = "";				// Name of the game (e.g. "Unreal Tournament")
	std::string gameRootFolder = "";		// Path to the folder that contains all the subfolders and files
	std::string gameExecutableName = "";	// Name of the game executable (e.g. "UnrealTournament")
	std::string gameVersionString = "";		// Version (+ sub version) info as a string (e.g. "469d")
	std::string url = "";					// The UnrealURL to launch upon startup

	bool IsUnreal1() const { return gameExecutableName == "Unreal"; }
	bool IsUnreal1_226() const { return IsUnreal1() && engineVersion == 226; }
	bool IsUnreal1_227() const { return IsUnreal1() && engineVersion == 227; }
	bool IsUnrealTournament() const { return gameExecutableName == "UnrealTournament"; }
	bool IsUnrealTournament_469() const { return IsUnrealTournament() && engineVersion == 469; }
	bool IsDeusEx() const { return gameExecutableName == "DeusEx"; }
	bool IsCliveBarkersUndying() const { return gameExecutableName == "Undying"; }
	bool IsKlingonHonorGuard() const { return gameExecutableName == "Klingons" || gameExecutableName == "Khg"; }
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
