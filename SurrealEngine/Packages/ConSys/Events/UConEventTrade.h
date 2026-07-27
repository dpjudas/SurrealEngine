#pragma once

#include "UConEvent.h"

class UActor;

class UConEventTrade : public UConEvent
{
public:
	using UConEvent::UConEvent;

	UActor*& eventOwner() { return Value<UActor*>(PropOffsets_ConEventTrade.eventOwner); }
	std::string& eventOwnerName() { return Value<std::string>(PropOffsets_ConEventTrade.eventOwnerName); }
};
