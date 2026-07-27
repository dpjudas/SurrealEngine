#pragma once

#include "UFlag.h"

class UFlagFloat : public UFlag
{
public:
	using UFlag::UFlag;

	float& floatValue() { return Value<float>(PropOffsets_FlagFloat.floatValue); }
};
