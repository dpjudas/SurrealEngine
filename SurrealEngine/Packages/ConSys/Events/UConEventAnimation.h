#pragma once

#include "UConEvent.h"

class UActor;

class UConEventAnimation : public UConEvent
{
public:
	using UConEvent::UConEvent;

	NameString& Sequence() { return Value<NameString>(PropOffsets_ConEventAnimation.Sequence); }
	BitfieldBool bFinishAnim() { return BoolValue(PropOffsets_ConEventAnimation.bFinishAnim); }
	BitfieldBool bLoopAnim() { return BoolValue(PropOffsets_ConEventAnimation.bLoopAnim); }
	UActor*& eventOwner() { return Value<UActor*>(PropOffsets_ConEventAnimation.eventOwner); }
	std::string& eventOwnerName() { return Value<std::string>(PropOffsets_ConEventAnimation.eventOwnerName); }
};
