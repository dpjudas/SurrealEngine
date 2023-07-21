#pragma once

struct GameLaunchInfo
{
	std::string folder;
	int engineVersion = 0;
	std::string gameName;
	bool noEntryMap = false;
	std::string url;
};

struct GameFolder
{
	std::string name;
	GameLaunchInfo launchInfo;
};

class GameFolderSelection
{
public:
	static GameLaunchInfo GetLaunchInfo();

private:
	static GameFolder ExamineFolder(const std::string& path);
	static std::vector<std::string> FindGameFolders();
	static std::string GetExePath();
};
