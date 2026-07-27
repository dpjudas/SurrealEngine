#pragma once

#include "Packages/Core/UObject.h"

class ULevelSummary : public UObject
{
public:
	using UObject::UObject;

	std::string& Author() { return Value<std::string>(PropOffsets_LevelSummary.Author); }
	std::string& IdealPlayerCount() { return Value<std::string>(PropOffsets_LevelSummary.IdealPlayerCount); }
	std::string& LevelEnterText() { return Value<std::string>(PropOffsets_LevelSummary.LevelEnterText); }
	int& RecommendedEnemies() { return Value<int>(PropOffsets_LevelSummary.RecommendedEnemies); }
	int& RecommendedTeammates() { return Value<int>(PropOffsets_LevelSummary.RecommendedTeammates); }
	std::string& Title() { return Value<std::string>(PropOffsets_LevelSummary.Title); }
};
