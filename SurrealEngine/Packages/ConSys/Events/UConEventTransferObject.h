#pragma once

#include "UConEvent.h"

class UActor;

class UConEventTransferObject : public UConEvent
{
public:
	using UConEvent::UConEvent;

	std::string& ObjectName() { return Value<std::string>(PropOffsets_ConEventTransferObject.ObjectName); }
	std::string& failLabel() { return Value<std::string>(PropOffsets_ConEventTransferObject.failLabel); }
	UActor*& fromActor() { return Value<UActor*>(PropOffsets_ConEventTransferObject.fromActor); }
	std::string& fromName() { return Value<std::string>(PropOffsets_ConEventTransferObject.fromName); }
	UClass*& giveObject() { return Value<UClass*>(PropOffsets_ConEventTransferObject.giveObject); }
	UActor*& toActor() { return Value<UActor*>(PropOffsets_ConEventTransferObject.toActor); }
	std::string& toName() { return Value<std::string>(PropOffsets_ConEventTransferObject.toName); }
	int& transferCount() { return Value<int>(PropOffsets_ConEventTransferObject.transferCount); }
};
