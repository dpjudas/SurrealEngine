#pragma once

#include "UConEvent.h"

class UConFlagRef;

class UConEventCheckFlag : public UConEvent
{
public:
	using UConEvent::UConEvent;

	UConFlagRef*& flagRef() { return Value<UConFlagRef*>(PropOffsets_ConEventCheckFlag.flagRef); }
	std::string& setLabel() { return Value<std::string>(PropOffsets_ConEventCheckFlag.setLabel); }
};
