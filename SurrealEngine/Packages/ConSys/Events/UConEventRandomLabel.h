#pragma once

#include "UConEvent.h"

class UConEventRandomLabel : public UConEvent
{
public:
	using UConEvent::UConEvent;

	void Load(ObjectStream* stream) override;

	std::string GetLabel(int labelIndex);
	int GetLabelCount();
	std::string GetRandomLabel();

	BitfieldBool bCycleEvents() { return BoolValue(PropOffsets_ConEventRandomLabel.bCycleEvents); }
	BitfieldBool bCycleOnce() { return BoolValue(PropOffsets_ConEventRandomLabel.bCycleOnce); }
	BitfieldBool bCycleRandom() { return BoolValue(PropOffsets_ConEventRandomLabel.bCycleRandom); }
	BitfieldBool bLabelsCycled() { return BoolValue(PropOffsets_ConEventRandomLabel.bLabelsCycled); }
	int& cycleIndex() { return Value<int>(PropOffsets_ConEventRandomLabel.cycleIndex); }
	std::vector<std::string> labels;
};
