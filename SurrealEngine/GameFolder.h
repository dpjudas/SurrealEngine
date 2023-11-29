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

enum class KnownUE1Games
{
	UNREALGOLD_226,
	UNREALGOLD_227i,
	UNREALGOLD_227j,
	UT99_436,
	UT99_451,
	UT99_469d_RC4,
	DEUS_EX_1112fm,
	KLINGON_219
};

const std::map<std::string, KnownUE1Games> SHA1Database = {
	// Unreal Gold, 226
	{"a4e8149a3e3a9aeba3921eb5004973c4cb1a5c35", KnownUE1Games::UNREALGOLD_226},

	// Unreal Gold, 227i, Windows + Linux versions
	// Windows
	{"81e2fff3d9f40f49447adc03ef4374059a902502", KnownUE1Games::UNREALGOLD_227i},
	// Linux
	{"4eb0ace08b28edc75d798b1482b9952f262bdc6f", KnownUE1Games::UNREALGOLD_227i},

	// Unreal Gold, 227j, Windows + Linux versions (32 + 64 bit)
	// Windows 32 bit
	{"f3c0ff6ae6594080a72f2ee1f41da1b13cbed55c", KnownUE1Games::UNREALGOLD_227j},
	// Windows 64 bit
	{"a89ab3ca17fb73f4087fb5d5c4420eae0313b4ce", KnownUE1Games::UNREALGOLD_227j},
	// Linux, 32 bit
	{"a613cfcb78ad38190a8612a5267af1c6052ab95d", KnownUE1Games::UNREALGOLD_227j},
	// Linux, 64 bit
	{"60425d1dc310e49f1aa0d05c81abc3195f0b03d9", KnownUE1Games::UNREALGOLD_227j},

	// Unreal Tournament, v436, Windows version
	// (TODO: get sha1sum of the Linux version)
	{"38e7c23e17b4acbc745d955f234726bad730d3e2", KnownUE1Games::UT99_436},

	// Unreal Tournament, v451b, Windows + Linux versions
	// Windows
	{"f3c6253b1f7458789a2653ca8bfa50938d6f53a6", KnownUE1Games::UT99_451},
	// Linux
	{"04ac234a7e4d01a44e476a50ea8cd73d838e5137", KnownUE1Games::UT99_451},

	// Unreal Tournament, v469d RC4, Windows + Linux versions (32 + 64 bit)
	// (TODO: Should be recalculated for 469d final, whenever it comes out)
	// Windows
	{"a396908c74b922041c8de6c7959923de20f379f7", KnownUE1Games::UT99_469d_RC4},
	// Linux, 32 bit
	{"2504e5ee2e3bd38e0cda65062862d2a83c68cf91", KnownUE1Games::UT99_469d_RC4},
	// Linux, 64 bit
	{"9cee66da8d3cddff5685f36104be009252e414ee", KnownUE1Games::UT99_469d_RC4},

	// Deus Ex, v1112fm
	{"2a933e26aa9cfb33b37f78afe21434caa031f14a", KnownUE1Games::DEUS_EX_1112fm},

	// Klingon Honor Guard, 209
	// ???
};

const std::vector<std::string> knownUE1ExecutableNames = {
	"Unreal.exe",
	"UnrealLinux.bin",
	"UnrealTournament.exe",
	"ut-bin",
	"ut-bin-x86",
	"ut-bin-x64",
	"DeusEx.exe",
	"klingon.exe",
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
