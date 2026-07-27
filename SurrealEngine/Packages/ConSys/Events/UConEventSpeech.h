#pragma once

#include "UConEvent.h"

class UConSpeech;
class UActor;

class UConEventSpeech : public UConEvent
{
public:
	using UConEvent::UConEvent;

	UConSpeech*& ConSpeech() { return Value<UConSpeech*>(PropOffsets_ConEventSpeech.ConSpeech); }
	BitfieldBool bBold() { return BoolValue(PropOffsets_ConEventSpeech.bBold); }
	BitfieldBool bContinued() { return BoolValue(PropOffsets_ConEventSpeech.bContinued); }
	UActor*& speaker() { return Value<UActor*>(PropOffsets_ConEventSpeech.speaker); }
	std::string& speakerName() { return Value<std::string>(PropOffsets_ConEventSpeech.speakerName); }
	UActor*& speakingTo() { return Value<UActor*>(PropOffsets_ConEventSpeech.speakingTo); }
	std::string& speakingToName() { return Value<std::string>(PropOffsets_ConEventSpeech.speakingToName); }
	uint8_t& speechFont() { return Value<uint8_t>(PropOffsets_ConEventSpeech.speechFont); }
};
