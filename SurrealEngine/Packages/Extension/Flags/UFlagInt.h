#pragma once

#include "UFlag.h"

class UFlagInt : public UFlag
{
public:
	using UFlag::UFlag;

	int& intValue() { return Value<int>(PropOffsets_FlagInt.intValue); }
};
