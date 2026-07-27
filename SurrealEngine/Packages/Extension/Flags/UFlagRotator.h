#pragma once

#include "UFlag.h"

class UFlagRotator : public UFlag
{
public:
	using UFlag::UFlag;

	Rotator& rotatorValue() { return Value<Rotator>(PropOffsets_FlagRotator.rotatorValue); }
};
