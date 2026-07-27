#pragma once

#include "UFlag.h"

class UFlagByte : public UFlag
{
public:
	using UFlag::UFlag;

	uint8_t& byteValue() { return Value<uint8_t>(PropOffsets_FlagByte.byteValue); }
};
