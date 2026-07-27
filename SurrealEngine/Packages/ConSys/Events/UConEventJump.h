#pragma once

#include "UConEvent.h"

class UConEventJump : public UConEvent
{
public:
	using UConEvent::UConEvent;

	int& conID() { return Value<int>(PropOffsets_ConEventJump.conID); }
	UConversation*& jumpCon() { return Value<UConversation*>(PropOffsets_ConEventJump.jumpCon); }
	std::string& jumpLabel() { return Value<std::string>(PropOffsets_ConEventJump.jumpLabel); }
};
