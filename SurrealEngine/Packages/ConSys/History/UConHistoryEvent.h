#pragma once

#include "Packages/ConSys/UConObject.h"

class UConHistoryEvent : public UConObject
{
public:
	using UConObject::UConObject;

	UConHistoryEvent*& Next() { return Value<UConHistoryEvent*>(PropOffsets_ConHistoryEvent.Next); }
	std::string& Speech() { return Value<std::string>(PropOffsets_ConHistoryEvent.Speech); }
	std::string& conSpeaker() { return Value<std::string>(PropOffsets_ConHistoryEvent.conSpeaker); }
	int& soundID() { return Value<int>(PropOffsets_ConHistoryEvent.soundID); }
};
