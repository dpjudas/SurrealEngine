#pragma once

#include "UConEvent.h"

class UConFlagRef;

class UConEventSetFlag : public UConEvent
{
public:
	using UConEvent::UConEvent;

	UConFlagRef*& flagRef() { return Value<UConFlagRef*>(PropOffsets_ConEventSetFlag.flagRef); }
};
