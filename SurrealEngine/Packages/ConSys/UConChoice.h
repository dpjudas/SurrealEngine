#pragma once

#include "UConObject.h"

class UConFlagRef;

class UConChoice : public UConObject
{
public:
	using UConObject::UConObject;

	BitfieldBool bDisplayAsSpeech() { return BoolValue(PropOffsets_ConChoice.bDisplayAsSpeech); }
	std::string& choiceLabel() { return Value<std::string>(PropOffsets_ConChoice.choiceLabel); }
	std::string& choiceText() { return Value<std::string>(PropOffsets_ConChoice.choiceText); }
	UConFlagRef*& flagRef() { return Value<UConFlagRef*>(PropOffsets_ConChoice.flagRef); }
	UConChoice*& nextChoice() { return Value<UConChoice*>(PropOffsets_ConChoice.nextChoice); }
	int& skillLevelNeeded() { return Value<int>(PropOffsets_ConChoice.skillLevelNeeded); }
	UClass*& skillNeeded() { return Value<UClass*>(PropOffsets_ConChoice.skillNeeded); }
	int& soundID() { return Value<int>(PropOffsets_ConChoice.soundID); }
};
