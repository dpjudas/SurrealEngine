#pragma once

#include "UConEvent.h"

class UConEventAddCredits : public UConEvent
{
public:
	using UConEvent::UConEvent;

	int& creditsToAdd() { return Value<int>(PropOffsets_ConEventAddCredits.creditsToAdd); }
};
