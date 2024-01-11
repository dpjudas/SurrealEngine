#pragma once

struct GameLaunchInfo
{
	std::string folder;
	int engineVersion = 0;
	int engineSubVersion = 0;
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
	UNREAL_200,
	UNREAL_219,
	UNREAL_226f,
	UNREALGOLD_226b,
	UNREALGOLD_227i,
	UNREALGOLD_227j,
	UT99_400,
	UT99_436,
	UT99_451,
	UT99_469a,
	UT99_469b,
	UT99_469c,
	UT99_469d,
	DEUS_EX_1002f,
	DEUS_EX_1112fm,
	KLINGON_219,
	NERF_300,
	TNN_200,
	UNDYING_420
};

const std::map<std::string, KnownUE1Games> SHA1Database = {
	// Unreal Gold, 226
	{"a4e8149a3e3a9aeba3921eb5004973c4cb1a5c35", KnownUE1Games::UNREALGOLD_226b},

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

	// Unreal Tournament, v469a, Windows + Linux versions (32 bit)
	// Windows
	{"6deb44e2902a29ab58d0fe4db8d3cc49e50cd68f", KnownUE1Games::UT99_469a},
	// Linux
	{"3e12742cd5338b9cd58ab97fc0d541b119364e33", KnownUE1Games::UT99_469a},

	// Unreal Tournament, v469b, Windows + Linux versions (32 bit)
	// Windows
	{"88e714db07b26717fc53fd88db3b9145e0f2c4c5", KnownUE1Games::UT99_469b},
	// Linux
	{"32e208c38d9a5b4a505f4d7a608f90c9b36954b8", KnownUE1Games::UT99_469b},

	// Unreal Tournament, v469c, Windows + Linux versions (32 + 64 bit)
	// Windows
	{"d32bd20c2b2160633e758882a36d113bf2280632", KnownUE1Games::UT99_469c},
	// Linux, 32 bit
	{"59a6382fbc32c4bad0df70c7b5b4aebde2c2776c", KnownUE1Games::UT99_469c},
	// Linux, 64 bit
	{"994da56d669b7956b289ea2f0221adf9814a894c", KnownUE1Games::UT99_469c},

	// Unreal Tournament, v469d, Windows + Linux versions (32 + 64 bit)
	// Windows
	{"78c65e9434b442b15820d863136bb5a44700ad26", KnownUE1Games::UT99_469d},
	// Linux, 32 bit
	{"412cb72ae6deac8073e49ccad78904a415b90cf8", KnownUE1Games::UT99_469d},
	// Linux, 64 bit
	{"e117d883e3d0c480e9c56b01932b161c6c6f1315", KnownUE1Games::UT99_469d},

	// Deus Ex, v1002f
	{"9f923d667a396e8243028c14dc3f5e0a6db13d84", KnownUE1Games::DEUS_EX_1002f},

	// Deus Ex, v1112fm
	{"2a933e26aa9cfb33b37f78afe21434caa031f14a", KnownUE1Games::DEUS_EX_1112fm},

	// Nerf, Version 300
	{"7026a49ba2bc71ff52653589661c6d8cda3a8748", KnownUE1Games::NERF_300},

	// Klingon Honor Guard, 219
	// One oddity with this game is that they seem to have changed the executable name in the 1.1 patch
	// (which I presume IS version 219)
	// SHA1Sums of both exe files will be here just in case
	// Klingons.exe (Version 1.0)
	{"4b1c00883d289edd2e7ed02fd4da13fb4be2256f", KnownUE1Games::KLINGON_219},
	// Khg.exe (Version 1.1)
	{"96a35fff68d3e1539b4d4da8d7363dfbbd115cc5", KnownUE1Games::KLINGON_219},
};

const std::vector<std::string> knownUE1ExecutableNames = {
	"Unreal.exe",
	"UnrealLinux.bin",
	"UnrealTournament.exe",
	"ut-bin",
	"ut-bin-x86",
	"ut-bin-x64",
	"DeusEx.exe",
	"Klingons.exe",
	"Khg.exe",
	"Nerf.exe",
	"Rune.exe",
	"Spore.exe"
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
