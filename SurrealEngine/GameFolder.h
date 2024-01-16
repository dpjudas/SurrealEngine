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
};

class GameFolderSelection
{
public:
	static GameLaunchInfo GetLaunchInfo();

private:
	static GameLaunchInfo ExamineFolder(const std::string& path);
	static std::vector<std::string> FindGameFolders();
	static std::string GetExePath();
};
