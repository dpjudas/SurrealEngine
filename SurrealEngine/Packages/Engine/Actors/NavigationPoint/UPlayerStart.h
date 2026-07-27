#pragma once

#include "UNavigationPoint.h"

class UPlayerStart : public UNavigationPoint
{
public:
	using UNavigationPoint::UNavigationPoint;

	uint8_t& TeamNumber() { return Value<uint8_t>(PropOffsets_PlayerStart.TeamNumber); }
	BitfieldBool bCoopStart() { return BoolValue(PropOffsets_PlayerStart.bCoopStart); }
	BitfieldBool bEnabled() { return BoolValue(PropOffsets_PlayerStart.bEnabled); }
	BitfieldBool bSinglePlayerStart() { return BoolValue(PropOffsets_PlayerStart.bSinglePlayerStart); }
};
