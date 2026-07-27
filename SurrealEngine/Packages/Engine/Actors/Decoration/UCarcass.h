#pragma once

#include "UDecoration.h"

class UPawn;
class UPlayerReplicationInfo;

class UCarcass : public UDecoration
{
public:
	using UDecoration::UDecoration;

	UPawn*& Bugs() { return Value<UPawn*>(PropOffsets_Carcass.Bugs); }
	int& CumulativeDamage() { return Value<int>(PropOffsets_Carcass.CumulativeDamage); }
	UPlayerReplicationInfo*& PlayerOwner() { return Value<UPlayerReplicationInfo*>(PropOffsets_Carcass.PlayerOwner); }
	BitfieldBool bDecorative() { return BoolValue(PropOffsets_Carcass.bDecorative); }
	BitfieldBool bPlayerCarcass() { return BoolValue(PropOffsets_Carcass.bPlayerCarcass); }
	BitfieldBool bReducedHeight() { return BoolValue(PropOffsets_Carcass.bReducedHeight); }
	BitfieldBool bSlidingCarcass() { return BoolValue(PropOffsets_Carcass.bSlidingCarcass); }
	uint8_t& flies() { return Value<uint8_t>(PropOffsets_Carcass.flies); }
	uint8_t& rats() { return Value<uint8_t>(PropOffsets_Carcass.rats); }
};
