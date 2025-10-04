#include "UE1GameDatabase.h"

#include "Utils/File.h"
#include "TinySHA1/TinySHA1.hpp"

std::pair<KnownUE1Games, std::string> FindUE1GameInPath(const std::string& ue1_game_root_folder_path)
{
	if (ue1_game_root_folder_path.empty())
		return std::make_pair(KnownUE1Games::UE1_GAME_NOT_FOUND, "");

	if (!fs::exists(ue1_game_root_folder_path) || !fs::is_directory(ue1_game_root_folder_path))
		return std::make_pair(KnownUE1Games::UE1_GAME_NOT_FOUND, "");

	const auto UE1GameSystemPath = fs::path(ue1_game_root_folder_path) / "System";

	for (auto& executable_name : knownUE1ExecutableNames)
	{
		auto executablePath = UE1GameSystemPath / executable_name;

		if (File::try_open_existing(executablePath))
		{
			// Such executable exists, let's try to take SHA1Sum of it
			auto bytes = File::read_all_bytes(executablePath);

			sha1::SHA1 s;
			s.processBytes(bytes.data(), bytes.size());
			uint32_t digest[5];
			s.getDigest(digest);

			char temp[41];
			snprintf(temp, 41, "%08x%08x%08x%08x%08x", digest[0], digest[1], digest[2], digest[3], digest[4]);

			std::string sha1sum(temp);

			// Now check whether there is a match within the database or not
			const auto it = SHA1Database.find(sha1sum);

			if (it == SHA1Database.end())
				return std::make_pair(KnownUE1Games::UE1_GAME_NOT_FOUND, "");

			// Hack: Tactical-Ops has a version that contains the exact same UTv469d executable. Handle that here
			if (it->second == KnownUE1Games::UT99_469d && executable_name == "TacticalOps.exe")
				return std::make_pair(KnownUE1Games::TACTICAL_OPS_469, executable_name);

			return std::make_pair(it->second, executable_name);
		}
	}

	// We got nothing here
	return std::make_pair(KnownUE1Games::UE1_GAME_NOT_FOUND, "");
}