#pragma once

#include "Packages/Engine/Actors/UActor.h"

class USound;

class UDecoration : public UActor
{
public:
	using UActor::UActor;

	UClass*& EffectWhenDestroyed() { return Value<UClass*>(PropOffsets_Decoration.EffectWhenDestroyed); }
	USound*& EndPushSound() { return Value<USound*>(PropOffsets_Decoration.EndPushSound); }
	USound*& PushSound() { return Value<USound*>(PropOffsets_Decoration.PushSound); }
	BitfieldBool bBobbing() { return BoolValue(PropOffsets_Decoration.bBobbing); }
	BitfieldBool bOnlyTriggerable() { return BoolValue(PropOffsets_Decoration.bOnlyTriggerable); }
	BitfieldBool bPushSoundPlaying() { return BoolValue(PropOffsets_Decoration.bPushSoundPlaying); }
	BitfieldBool bPushable() { return BoolValue(PropOffsets_Decoration.bPushable); }
	BitfieldBool bSplash() { return BoolValue(PropOffsets_Decoration.bSplash); }
	BitfieldBool bWasCarried() { return BoolValue(PropOffsets_Decoration.bWasCarried); }
	UClass*& content2() { return Value<UClass*>(PropOffsets_Decoration.content2); }
	UClass*& content3() { return Value<UClass*>(PropOffsets_Decoration.content3); }
	UClass*& contents() { return Value<UClass*>(PropOffsets_Decoration.contents); }
	int& numLandings() { return Value<int>(PropOffsets_Decoration.numLandings); }
};
