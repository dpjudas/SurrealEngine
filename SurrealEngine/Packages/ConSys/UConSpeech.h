#pragma once

#include "UConObject.h"

class UConSpeech : public UConObject
{
public:
	using UConObject::UConObject;

	std::string& Speech() { return Value<std::string>(PropOffsets_ConSpeech.Speech); }
	int& soundID() { return Value<int>(PropOffsets_ConSpeech.soundID); }
};
