#pragma once

#include "UConEvent.h"

class UConEventAddNote : public UConEvent
{
public:
	using UConEvent::UConEvent;

	BitfieldBool bNoteAdded() { return BoolValue(PropOffsets_ConEventAddNote.bNoteAdded); }
	std::string& noteText() { return Value<std::string>(PropOffsets_ConEventAddNote.noteText); }
};
