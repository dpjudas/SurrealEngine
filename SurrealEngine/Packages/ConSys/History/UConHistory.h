#pragma once

#include "Packages/ConSys/UConObject.h"

class UConHistoryEvent;

class UConHistory : public UConObject
{
public:
	using UConObject::UConObject;

	UConHistory*& Next() { return Value<UConHistory*>(PropOffsets_ConHistory.Next); }
	BitfieldBool bInfoLink() { return BoolValue(PropOffsets_ConHistory.bInfoLink); }
	std::string& conOwnerName() { return Value<std::string>(PropOffsets_ConHistory.conOwnerName); }
	UConHistoryEvent*& firstEvent() { return Value<UConHistoryEvent*>(PropOffsets_ConHistory.firstEvent); }
	UConHistoryEvent*& lastEvent() { return Value<UConHistoryEvent*>(PropOffsets_ConHistory.lastEvent); }
	std::string& strDescription() { return Value<std::string>(PropOffsets_ConHistory.strDescription); }
	std::string& strLocation() { return Value<std::string>(PropOffsets_ConHistory.strLocation); }
};
