#pragma once

#include "UConEvent.h"

class UConEventAddSkillPoints : public UConEvent
{
public:
	using UConEvent::UConEvent;

	std::string& awardMessage() { return Value<std::string>(PropOffsets_ConEventAddSkillPoints.awardMessage); }
	int& pointsToAdd() { return Value<int>(PropOffsets_ConEventAddSkillPoints.pointsToAdd); }
};
