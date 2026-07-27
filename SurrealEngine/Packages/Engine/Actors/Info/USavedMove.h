#pragma once

#include "UInfo.h"

class USavedMove : public UInfo
{
public:
	using UInfo::UInfo;

	float& Delta() { return Value<float>(PropOffsets_SavedMove.Delta); }
	uint8_t& DodgeMove() { return Value<uint8_t>(PropOffsets_SavedMove.DodgeMove); }
	USavedMove*& NextMove() { return Value<USavedMove*>(PropOffsets_SavedMove.NextMove); }
	float& TimeStamp() { return Value<float>(PropOffsets_SavedMove.TimeStamp); }
	BitfieldBool bAltFire() { return BoolValue(PropOffsets_SavedMove.bAltFire); }
	BitfieldBool bDuck() { return BoolValue(PropOffsets_SavedMove.bDuck); }
	BitfieldBool bFire() { return BoolValue(PropOffsets_SavedMove.bFire); }
	BitfieldBool bForceAltFire() { return BoolValue(PropOffsets_SavedMove.bForceAltFire); }
	BitfieldBool bForceFire() { return BoolValue(PropOffsets_SavedMove.bForceFire); }
	BitfieldBool bPressedJump() { return BoolValue(PropOffsets_SavedMove.bPressedJump); }
	BitfieldBool bRun() { return BoolValue(PropOffsets_SavedMove.bRun); }
};
