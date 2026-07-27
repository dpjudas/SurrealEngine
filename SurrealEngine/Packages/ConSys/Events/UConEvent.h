#pragma once

#include "Packages/ConSys/UConObject.h"

class USound;
class UConversation;

enum class EEventType
{
	Speech,
	Choice,
	SetFlag,
	CheckFlag,
	CheckObject,
	TransferObject,
	MoveCamera,
	Animation,
	Trade,
	Jump,
	Random,
	Trigger,
	AddGoal,
	AddNote,
	AddSkillPoints,
	AddCredits,
	CheckPersona,
	Comment,
	End
};

class UConEvent : public UConObject
{
public:
	using UConObject::UConObject;

	float GetSoundLength(USound* sound);

	UConversation*& Conversation() { return Value<UConversation*>(PropOffsets_ConEvent.Conversation); }
	std::string& Label() { return Value<std::string>(PropOffsets_ConEvent.Label); }
	uint8_t& eventType() { return Value<uint8_t>(PropOffsets_ConEvent.eventType); }
	UConEvent*& nextEvent() { return Value<UConEvent*>(PropOffsets_ConEvent.nextEvent); }
};
