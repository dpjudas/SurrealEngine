#include "UE1GameDatabase.h"
#include "Utils/SHA1Sum.h"
#include <iostream>

std::pair<KnownUE1Games, std::string> FindUE1GameInPath(const std::string& ue1_game_root_folder_path)
{
	if (ue1_game_root_folder_path.empty())
		return std::make_pair(KnownUE1Games::UE1_GAME_NOT_FOUND, "");

	if (!fs::exists(ue1_game_root_folder_path) || !fs::is_directory(ue1_game_root_folder_path))
		return std::make_pair(KnownUE1Games::UE1_GAME_NOT_FOUND, "");

	const auto UE1GameSystemPath = fs::path(ue1_game_root_folder_path) / "System";
	const auto UE1GameSystem64Path = fs::path(ue1_game_root_folder_path) / "System64";

	for (auto& executable_name : knownUE1ExecutableNames)
	{
		auto executablePath = UE1GameSystemPath / executable_name;
		auto executablePath64 = UE1GameSystem64Path / executable_name;
		std::cout << "PATH64: " << executablePath64 << std::endl;

		std::string sha1sum = SHA1Sum::of_file(executablePath);
		std::string sha1sum64 = SHA1Sum::of_file(executablePath64);

		// If sha1sum is empty this means that the file doesn't exist
		if (sha1sum.empty() && sha1sum64.empty())
			continue;

		// Now check whether there is a match within the database or not
		const auto it = !sha1sum.empty() ? SHA1Database.find(sha1sum) : SHA1Database.find(sha1sum64);
		
		if (it == SHA1Database.end())
			return std::make_pair(KnownUE1Games::UE1_GAME_NOT_FOUND, "");

		// Hack: Tactical-Ops has a version that contains the exact same UTv469d executable. Handle that here
		if (it->second == KnownUE1Games::UT99_469d && executable_name == "TacticalOps.exe")
			return std::make_pair(KnownUE1Games::TACTICAL_OPS_469, executable_name);
		return std::make_pair(it->second, executable_name);
	}

	// We got nothing here
	return std::make_pair(KnownUE1Games::UE1_GAME_NOT_FOUND, "");
}