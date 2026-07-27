#pragma once

#include "UConEvent.h"

class UConEventCheckObject : public UConEvent
{
public:
	using UConEvent::UConEvent;

	std::string& ObjectName() { return Value<std::string>(PropOffsets_ConEventCheckObject.ObjectName); }
	UClass*& checkObject() { return Value<UClass*>(PropOffsets_ConEventCheckObject.checkObject); }
	std::string& failLabel() { return Value<std::string>(PropOffsets_ConEventCheckObject.failLabel); }
};
