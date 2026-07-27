#pragma once

#include "UConEvent.h"

class UConEventTrigger : public UConEvent
{
public:
	using UConEvent::UConEvent;

	NameString& triggerTag() { return Value<NameString>(PropOffsets_ConEventTrigger.triggerTag); }
};
