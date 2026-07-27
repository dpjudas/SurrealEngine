#pragma once

#include "UConEvent.h"

class UConEventCheckPersona : public UConEvent
{
public:
	using UConEvent::UConEvent;

	int& Value() { return UObject::Value<int>(PropOffsets_ConEventCheckPersona.Value); }
	uint8_t& condition() { return UObject::Value<uint8_t>(PropOffsets_ConEventCheckPersona.condition); }
	std::string& jumpLabel() { return UObject::Value<std::string>(PropOffsets_ConEventCheckPersona.jumpLabel); }
	uint8_t& personaType() { return UObject::Value<uint8_t>(PropOffsets_ConEventCheckPersona.personaType); }
};
