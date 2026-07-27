#pragma once

#include "UConEvent.h"

class UConChoice;

class UConEventChoice : public UConEvent
{
public:
	using UConEvent::UConEvent;

	UConChoice*& ChoiceList() { return Value<UConChoice*>(PropOffsets_ConEventChoice.ChoiceList); }
	BitfieldBool bClearScreen() { return BoolValue(PropOffsets_ConEventChoice.bClearScreen); }
};
