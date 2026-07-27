#pragma once

#include "UPathNodeIterator.h"

class UPakPawnPathNodeIterator : public UPakPathNodeIterator
{
public:
	using UPakPathNodeIterator::UPakPathNodeIterator;

	void SetPawn(UPawn* P);

	UPawn*& Pawn() { return Value<UPawn*>(PropOffsets_UPakPawnPathNodeIterator.Pawn); }
};
